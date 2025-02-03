// FRAMEBUFFER FILTER
// (C) 2025 Iliyas Jorio
// This file is part of Mighty Mike. https://github.com/jorio/mightymike

#include <SDL3/SDL_mutex.h>
#include <SDL3/SDL_thread.h>

#include "externs.h"
#include "misc.h"
#include "window.h"
#include "framebufferfilter.h"

int gNumThreads = 0;

static SDL_Thread* gRenderThreadPool[MAX_RENDER_THREADS];
static SDL_Mutex* gMutex;
static SDL_Condition* gCondition_GetToWork;			// Main thread tells worker threads to start processing the image
static SDL_Condition* gCondition_AllThreadsReady;	// Last thread tells main thread the image is complete
static bool gQuitRenderThreads = false;
static int gNumThreadsReady = 0;

static color_t gScratch[1024*512];  // todo: actual size
static color_t* gFinalColor = NULL;

typedef struct ThreadParams { int threadNum, firstRow, numRows; } ThreadParams;

// ----------------------------------------------------------------------------

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

static int ConverterThread(void* data)
{
	ThreadParams params = *(ThreadParams*) data;

	while (true)
	{
		SDL_LockMutex(gMutex);
		{
			// We're ready to receive more work.
			gNumThreadsReady++;

			// When all threads are ready, tell the main thread to present the image.
			if (gNumThreadsReady == gNumThreads)
			{
				gNumThreadsReady = 0;
				SDL_SignalCondition(gCondition_AllThreadsReady);
			}

			// Let main thread wake us up when it's time to process the next image.
			SDL_WaitCondition(gCondition_GetToWork, gMutex);
			GAME_ASSERT(gNumThreadsReady < gNumThreads);
		}
		SDL_UnlockMutex(gMutex);

		if (gQuitRenderThreads)
			break;

		// Do the work
		Convert(params.threadNum, params.firstRow, params.numRows);
	}

	return 0;
}

void InitRenderThreads(void)
{
	static ThreadParams threadParams[MAX_RENDER_THREADS];
	char name[32];

	GAME_ASSERT(gNumThreads == 0);
	GAME_ASSERT(gNumThreadsReady == 0);
	GAME_ASSERT(!gMutex);
	GAME_ASSERT(!gCondition_GetToWork);
	GAME_ASSERT(!gCondition_AllThreadsReady);

	gNumThreads = SDL_GetNumLogicalCPUCores();
	gNumThreads = SDL_clamp(gNumThreads, 1, MAX_RENDER_THREADS);
	SDL_Log("Render thread pool: %d", gNumThreads);

	if (gNumThreads <= 1)	// single-threaded rendering: don't create extra threads
	{
		return;
	}

	gMutex = SDL_CreateMutex();
	gCondition_AllThreadsReady = SDL_CreateCondition();
	gCondition_GetToWork = SDL_CreateCondition();
	gQuitRenderThreads = false;

	SDL_LockMutex(gMutex);

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

		SDL_snprintf(name, sizeof(name), "Filter%c-%03d-%03d", 'A' + i, y, y+rowsThisThread-1);

		threadParams[i] = (ThreadParams) { .threadNum = i, .firstRow = y, .numRows = rowsThisThread };

		// Create thread
		gRenderThreadPool[i] = SDL_CreateThread(ConverterThread, name, &threadParams[i]);

		y += rowsThisThread;
	}

	// Wait for all threads to be ready so they don't miss the condition when we start rendering
	SDL_WaitCondition(gCondition_AllThreadsReady, gMutex);
	SDL_UnlockMutex(gMutex);
}

void ConvertFramebufferMT(color_t* colorBuffer)
{
	GAME_ASSERT(gNumThreads != 0);

	gFinalColor = colorBuffer;

	if (gNumThreads == 1)	// single-threaded: do rendering on main thread
	{
		Convert(0, 0, VISIBLE_HEIGHT);
		return;
	}

	SDL_LockMutex(gMutex);
	SDL_BroadcastCondition(gCondition_GetToWork);
	SDL_WaitCondition(gCondition_AllThreadsReady, gMutex);
	SDL_UnlockMutex(gMutex);
}

void ShutdownRenderThreads(void)
{
	if (gNumThreads <= 1)
	{
		return;
	}

	// Tell all threads to quit
	SDL_LockMutex(gMutex);
	gQuitRenderThreads = true;
	SDL_BroadcastCondition(gCondition_GetToWork);
	SDL_UnlockMutex(gMutex);

	// Join all threads
	for (int i = 0; i < gNumThreads; i++)
	{
		SDL_WaitThread(gRenderThreadPool[i], NULL);
		gRenderThreadPool[i] = NULL;
	}

	gNumThreads = 0;
	gNumThreadsReady = 0;

	SDL_DestroyMutex(gMutex);
	SDL_DestroyCondition(gCondition_GetToWork);
	SDL_DestroyCondition(gCondition_AllThreadsReady);
	gMutex = NULL;
	gCondition_GetToWork = NULL;
	gCondition_AllThreadsReady = NULL;
}
