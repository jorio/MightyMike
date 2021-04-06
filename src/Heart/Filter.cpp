// FRAMEBUFFER FILTER
// (C) 2021 Iliyas Jorio
// This file is part of Mighty Mike. https://github.com/jorio/mightymike

#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>

#if !_WIN32
	#include <pthread.h>
#endif

extern "C"
{
	#include "externs.h"
	#include "window.h"
}

static std::vector<std::thread> gRenderThreadPool;
static std::mutex gMutex;
static std::condition_variable gMainToRenderers;
static std::condition_variable gRenderersToMain;
static bool gQuitRenderThreads = false;
static uint32_t gLatchMask;

static inline void FilterDithering_Row(const uint8_t* indexedRow, uint8_t* rowSmearFlags);

// ----------------------------------------------------------------------------

static void ConvertIndexedFramebufferToRGBA_NoFilter(int firstRow, int numRows)
{
	uint32_t* rgba				= ((uint32_t*)gRGBAFramebuffer) + firstRow * VISIBLE_WIDTH;
	const uint8_t* indexed		= gIndexedFramebuffer + firstRow * VISIBLE_WIDTH;

	for (int y = 0; y < numRows; y++)
	{
		for (int x = 0; x < VISIBLE_WIDTH; x++)
		{
			*(rgba++) = gGamePalette[*(indexed++)];
		}
	}
}

static void ConvertIndexedFramebufferToRGBA_FilterDithering(int threadNum, int firstRow, int numRows)
{
	uint32_t* rgba				= ((uint32_t*)gRGBAFramebuffer) + firstRow * VISIBLE_WIDTH;
	const uint8_t* indexed		= gIndexedFramebuffer + firstRow * VISIBLE_WIDTH;
	uint8_t* smearFlags			= gRowDitherStrides + threadNum * VISIBLE_WIDTH;

	for (int y = 0; y < numRows; y++)
	{
		FilterDithering_Row(indexed, smearFlags);

		for (int x = 0; x < VISIBLE_WIDTH-1; x++)
		{
			if (smearFlags[x])
			{
				uint8_t* me		= (uint8_t*) &gGamePalette[*indexed];
				uint8_t* next	= (uint8_t*) &gGamePalette[*(indexed+1)];
				uint8_t* out	= (uint8_t*) rgba;
				out[1] = (me[1] + next[1]) >> 1;
				out[2] = (me[2] + next[2]) >> 1;
				out[3] = (me[3] + next[3]) >> 1;
				smearFlags[x] = 0;			// clear for next row
			}
			else
				*rgba = gGamePalette[*indexed];

			rgba++;
			indexed++;
		}

		*rgba = gGamePalette[*indexed];		// last
		rgba++;
		indexed++;
	}
}

static inline void FilterDithering_Row(const uint8_t* indexedRow, uint8_t* rowSmearFlags)
{
	static const int THRESH = 2;
	static const int BLEED = 1;

	int prev	= -1;
	int me		= indexedRow[0];
	int next	= indexedRow[1];

	int ditherStart		= 0;
	int ditherEnd		= -1;


#define COMMIT_STRIDE do { \
	int ditherLength = ditherEnd - ditherStart;								\
	if (ditherLength > THRESH)												\
		memset(rowSmearFlags+ditherStart, 1, ditherLength+BLEED);			\
	} while(0)

	for (int x = 0; x < VISIBLE_WIDTH-1; x++)
	{
		next = indexedRow[x+1];

		if (me==next || me==prev)	// contiguous solid color
		{
			COMMIT_STRIDE;			// 			commit current dither stride if any
			ditherEnd = -1;			// 			break dither stride
		}
		else if (prev==next)		// middle of dithered stride
		{
			if (ditherEnd < 0)		// 			no current dither stride yet
				ditherStart = x-1;	// 			start stride on left dither pixel
			ditherEnd = x+1;		// 			extend stride to right dither pixel
		}
		else if (x == ditherEnd)	// pixel was used to dither previous column
		{
			;						// 			let it be -- perhaps next pixel will detect we're still in dither stride
		}
		else						// lone non-dithered pixel
		{
			COMMIT_STRIDE;			// 			commit current dither stride if any
			ditherEnd = -1;			// 			break dither stride
		}

		prev = me;
		me = next;
	}

	// commit last
	COMMIT_STRIDE;

#undef COMMIT_STRIDE
}

// ----------------------------------------------------------------------------

static void RaiseLatches()
{
	gLatchMask = (1ul << gRenderThreadPool.size()) - 1ul;
}

static bool IsLatchRaised(int threadNum)
{
	return (gLatchMask >> threadNum) & 1;
}

static void LowerLatch(int threadNum)
{
	gLatchMask &= ~(1 << threadNum);
}

static void TellMainThreadImReady(int threadNum)
{
	LowerLatch(threadNum);

	if (gLatchMask == 0)
	{
		gRenderersToMain.notify_one();
	}
}

static void ConverterThread(int threadNo, int firstRow, int numRows)
{
#if !_WIN32 && _GNU_SOURCE
	char name[32];
	snprintf(name, sizeof(name), "Rows %03d-%03d", firstRow, firstRow+numRows-1);
	pthread_setname_np(pthread_self(), name);
#endif

	// Initial condition: tell main thread we're ready
	{
		std::scoped_lock lock(gMutex);
		TellMainThreadImReady(threadNo);
	}

	while (true)
	{
		// Wait on main to raise latch
		{
			std::unique_lock lock(gMutex);

			gMainToRenderers.wait(lock, [=] { return IsLatchRaised(threadNo); });

			if (gQuitRenderThreads)
				break;
		}

		// Do the work
		if (gGamePrefs.filterDithering)
			ConvertIndexedFramebufferToRGBA_FilterDithering(threadNo, firstRow, numRows);
		else
			ConvertIndexedFramebufferToRGBA_NoFilter(firstRow, numRows);

		// Tell main thread we're ready (lower latch)
		{
			std::scoped_lock lock(gMutex);
			TellMainThreadImReady(threadNo);
		}
	}
}


static void WaitForAllRenderThreadsReady()
{
	std::unique_lock lock(gMutex);
	gRenderersToMain.wait(lock, [] { return gLatchMask == 0; });
	lock.unlock();
}

static void InitRenderThreadPool()
{
	gQuitRenderThreads = false;
	RaiseLatches();

	int rowsPerThread = VISIBLE_HEIGHT / gNumThreads;
	int remainder = VISIBLE_HEIGHT - rowsPerThread * gNumThreads;
	int y = 0;

	for (int i = 0; i < gNumThreads; i++)
	{
		// Distribute number of rows as evenly as possible
		int rowsThisThread = rowsPerThread;
		if (remainder > 0)
		{
			rowsThisThread++;
			remainder--;
		}

		// Create thread
		gRenderThreadPool.emplace_back(ConverterThread, i, y, rowsThisThread);

		y += rowsThisThread;
	}

	// Wait for all threads to be ready so they don't miss the condition when we start rendering
	WaitForAllRenderThreadsReady();
}

void ConvertFramebufferToRGBA(void)
{
	if (gRenderThreadPool.empty())
	{
		InitRenderThreadPool();
	}

	{
		std::scoped_lock lock(gMutex);
		RaiseLatches();
		gMainToRenderers.notify_all();
	}

	WaitForAllRenderThreadsReady();
}

void ShutdownRenderThreads(void)
{
	if (gRenderThreadPool.empty())
	{
		return;
	}

	// Tell all threads they need to quit
	{
		std::scoped_lock lock(gMutex);
		gQuitRenderThreads = true;
		RaiseLatches();
		gMainToRenderers.notify_all();
	}

	// Wait on all threads
	for (auto& t : gRenderThreadPool)
	{
		t.join();
	}

	gRenderThreadPool.clear();
}