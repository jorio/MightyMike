// FRAMEBUFFER FILTER
// (C) 2021 Iliyas Jorio
// This file is part of Mighty Mike. https://github.com/jorio/mightymike

#include <Pomme.h>
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
	#include "framebufferfilter.h"
}

static std::vector<std::thread> gRenderThreadPool;
static std::mutex gMutex;
static std::condition_variable gMainToRenderers;
static std::condition_variable gRenderersToMain;
static bool gQuitRenderThreads = false;
static uint32_t gLatchMask;

static color_t gScratch[1024*512];  // todo: actual size
static color_t* gFinalColor = NULL;

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

static void Convert(int threadNum, int firstRow, int numRows)
{
	bool doX2 = gEffectiveScalingType == kScaling_HQStretch;

	color_t* scratch = doX2 ? gScratch: gFinalColor;

	if (gGamePrefs.filterDithering)
		IndexedFramebufferToColor_FilterDithering(scratch, threadNum, firstRow, numRows);
	else
		IndexedFramebufferToColor_NoFilter(scratch, firstRow, numRows);

	if (gEffectiveScalingType == kScaling_HQStretch)
		DoublePixels(scratch, gFinalColor, firstRow, numRows);
}

static void ConverterThread(int threadNum, int firstRow, int numRows)
{
#if !_WIN32 && _GNU_SOURCE
	char name[32];
	snprintf(name, sizeof(name), "Rows %03d-%03d", firstRow, firstRow+numRows-1);
	pthread_setname_np(pthread_self(), name);
#endif

	// Initial condition: tell main thread we're ready
	{
		std::scoped_lock lock(gMutex);
		TellMainThreadImReady(threadNum);
	}

	while (true)
	{
		// Wait on main to raise latch
		{
			std::unique_lock lock(gMutex);

			gMainToRenderers.wait(lock, [=] { return IsLatchRaised(threadNum); });

			if (gQuitRenderThreads)
				break;
		}

		// Do the work
		Convert(threadNum, firstRow, numRows);

		// Tell main thread we're ready (lower latch)
		{
			std::scoped_lock lock(gMutex);
			TellMainThreadImReady(threadNum);
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
	if (gNumThreads <= 1)	// single-threaded rendering: don't create extra threads
	{
		return;
	}

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

void ConvertFramebufferMT(color_t* colorBuffer)
{
	gFinalColor = colorBuffer;

	if (gNumThreads <= 1)	// single-threaded: do rendering on main thread
	{
		Convert(0, 0, VISIBLE_HEIGHT);
		return;
	}

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
