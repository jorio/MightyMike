/****************************/
/*        WINDOW            */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/
#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "myglobals.h"
#include "window.h"
#include "playfield.h"
#include "object.h"
#include "misc.h"
#include "input.h"
#include "externs.h"
#include "renderdrivers.h"
#include "version.h"

/****************************/
/*    PROTOTYPES            */
/****************************/

static void DisposeScreenBuffers(void);
static void InitScreenBuffers(void);


/****************************/
/*    CONSTANTS             */
/****************************/

#define kHQStretchMinZoom 1.66f
#define kWiggleRoomCloseEnoughToIntScaling 16


/**********************/
/*     VARIABLES      */
/**********************/


int				VISIBLE_WIDTH = 640;			// dimensions of visible area (MULTIPLE OF 4!!!)
int				VISIBLE_HEIGHT = 480;

uint8_t*		gIndexedFramebuffer = nil;		// [VISIBLE_WIDTH * VISIBLE_HEIGHT]

int				gEffectiveScalingType = kScaling_Stretch;

uint8_t*		gRowDitherStrides = nil;		// for dithering filter

										// GAME STUFF
Handle			gBackgroundHandle = nil;
Handle			gOffScreenHandle = nil;
Handle			gPFBufferHandle = nil;
Handle			gPFBufferCopyHandle = nil;
Handle			gPFMaskBufferHandle = nil;

										// SCREEN ACCESS
long			gScreenXOffset = 0;				// to center 640x480 screens in widescreen mode
long			gScreenYOffset = 0;


uint8_t**		gScreenLookUpTable = nil;		//[VISIBLE_HEIGHT]
uint8_t**		gOffScreenLookUpTable = nil;	//[OFFSCREEN_HEIGHT]
uint8_t**		gBackgroundLookUpTable = nil;	//[OFFSCREEN_HEIGHT]

uint8_t**		gPFLookUpTable = nil;
uint8_t**		gPFCopyLookUpTable = nil;
uint8_t**		gPFMaskLookUpTable = nil;

static const uint32_t	kDebugTextUpdateInterval = 1000;
static uint32_t			gDebugTextFrameAccumulator = 0;
static uint32_t			gDebugTextLastUpdatedAt = 0;
static char				gDebugTextBuffer[1024];


/********************** ERASE BACKGROUND BUFFER ********************/

void EraseBackgroundBuffer(void)
{
	GAME_ASSERT(GetHandleSize(gBackgroundHandle) == OFFSCREEN_WIDTH*OFFSCREEN_HEIGHT);
	memset(*gBackgroundHandle, 0xFF, OFFSCREEN_WIDTH*OFFSCREEN_HEIGHT);	// clear to black
}


/********************** MAKE GAME WINDOW ********************/

void MakeGameWindow(void)
{
	SetScreenOffsetFor640x480();

	InitScreenBuffers();

	DumpGameWindow();

	OnChangeIntegerScaling();		// initial texture creation

	PresentIndexedFramebuffer();
}

/********************** DUMP GAME WINDOW ****************/
//
// Dumps offscreen buffer to framebuffer
//

void DumpGameWindow(void)
{
				/* GET SCREEN PIXMAP INFO */

	uint8_t* destPtr	= gIndexedFramebuffer;
	uint8_t* srcPtr		= (uint8_t *)(gOffScreenLookUpTable[0]+WINDOW_OFFSET);

						/* DO THE QUICK COPY */

	for (int y = 0; y < VISIBLE_HEIGHT; y++)
	{
		memcpy(destPtr, srcPtr, VISIBLE_WIDTH);

		destPtr += VISIBLE_WIDTH;				// Bump to start of next row
		srcPtr += OFFSCREEN_WIDTH;
	}
}



/********************** DUMP BACKGROUND ****************/
//
// Copy background image to playfield image buffer
//

void DumpBackground(void)
{
	memcpy(gOffScreenLookUpTable[0], gBackgroundLookUpTable[0], OFFSCREEN_WIDTH*OFFSCREEN_HEIGHT);
}



/******************** ERASE STORE **************************/
//
// Blanks alternate lines for interlace mode
//

void EraseStore(void)
{
uint8_t*	destPtr;
long		size;

				/* COPY PF BUFFER 2 TO BUFFER 1 TO ERASE STORE IMAGE */

	size = GetHandleSize(gPFBufferHandle);
	memcpy(*gPFBufferHandle, *gPFBufferCopyHandle, size);

				/* ERASE INTERLACING ZONE FROM MAIN SCREEN */

	if (gGamePrefs.interlaceMode)
	{
		destPtr = gScreenLookUpTable[PF_WINDOW_TOP+1] + PF_WINDOW_LEFT;

		for (short height = PF_WINDOW_HEIGHT>>1; height > 0; height--)
		{
			memset(destPtr, 0xFE, PF_WINDOW_WIDTH);		// dark grey
			destPtr += VISIBLE_WIDTH;
		}
	}
}


/******************* BLANK ENTIRE SCREEN AREA ********************/

void BlankEntireScreenArea(void)
{
Rect	r;

	r.left		= 0;
	r.top		= 0;
	r.right		= VISIBLE_WIDTH;
	r.bottom	= VISIBLE_HEIGHT;
	BlankScreenArea(r);
}

/********************* DISPOSE SCREEN BUFFERS ***********************/

static void DisposeScreenBuffers(void)
{
	CHECKED_DISPOSEPTR(gIndexedFramebuffer);

	CHECKED_DISPOSEHANDLE(gOffScreenHandle);
	CHECKED_DISPOSEHANDLE(gBackgroundHandle);
	CHECKED_DISPOSEPTR(gOffScreenLookUpTable);
	CHECKED_DISPOSEPTR(gBackgroundLookUpTable);

	CHECKED_DISPOSEPTR(gScreenLookUpTable);

	CHECKED_DISPOSEPTR(gPFLookUpTable);
	CHECKED_DISPOSEPTR(gPFCopyLookUpTable);
	CHECKED_DISPOSEPTR(gPFMaskLookUpTable);

	CHECKED_DISPOSEHANDLE(gPFBufferHandle);
	CHECKED_DISPOSEHANDLE(gPFBufferCopyHandle);
	CHECKED_DISPOSEHANDLE(gPFMaskBufferHandle);

	CHECKED_DISPOSEPTR(gRowDitherStrides);
}

/********************* INIT SCREEN BUFFERS ***********************/
//
// Create the Offscreen & Background buffers with xlate tables
//
//

static void InitScreenBuffers(void)
{
	DisposeScreenBuffers();

					/* MAKE INDEXED FRAMEBUFFER */

	gIndexedFramebuffer = (uint8_t*) NewPtrClear(VISIBLE_WIDTH * VISIBLE_HEIGHT);
	GAME_ASSERT(gIndexedFramebuffer);

	// Clear to black
	memset(gIndexedFramebuffer, 0xFF, VISIBLE_WIDTH * VISIBLE_HEIGHT);

					/* MAKE OFFSCREEN DRAW BUFFER */

	gOffScreenHandle = NewHandleClear(OFFSCREEN_WIDTH*OFFSCREEN_HEIGHT);
	GAME_ASSERT(gOffScreenHandle);

	// Clear to black
	memset(*gOffScreenHandle, 0xFF, OFFSCREEN_WIDTH*OFFSCREEN_HEIGHT);

					/* MAKE BACKPLANE BUFFER */

	gBackgroundHandle = NewHandleClear(OFFSCREEN_WIDTH*OFFSCREEN_HEIGHT);
	GAME_ASSERT(gBackgroundHandle);

					/* BUILD OFFSCREEN LOOKUP TABLE */

	gOffScreenLookUpTable = (uint8_t**) NewPtrClear(sizeof(uint8_t*) * OFFSCREEN_HEIGHT);
	for (int i = 0; i < OFFSCREEN_HEIGHT; i++)
	{
		gOffScreenLookUpTable[i] = ((uint8_t*) *gOffScreenHandle) + (i*OFFSCREEN_WIDTH);
	}

					/* BUILD BACKGROUND LOOKUP TABLE */

	gBackgroundLookUpTable = (uint8_t**) NewPtrClear(sizeof(uint8_t*) * OFFSCREEN_HEIGHT);
	for (int i = 0; i < OFFSCREEN_HEIGHT; i++)
	{
		gBackgroundLookUpTable[i] = ((uint8_t*) *gBackgroundHandle) + (i*OFFSCREEN_WIDTH);
	}

					/* ALLOC MEM FOR PF LOOKUP TABLES */

	gPFLookUpTable		= (uint8_t**) NewPtrClear(PF_BUFFER_HEIGHT * sizeof(uint8_t*));
	gPFCopyLookUpTable	= (uint8_t**) NewPtrClear(PF_BUFFER_HEIGHT * sizeof(uint8_t*));
	gPFMaskLookUpTable	= (uint8_t**) NewPtrClear(PF_BUFFER_HEIGHT * sizeof(uint8_t*));

					/* MAKE PLAYFIELD BUFFERS */

	gPFBufferHandle		= NewHandleClear(PF_BUFFER_HEIGHT * PF_BUFFER_WIDTH);
	gPFBufferCopyHandle	= NewHandleClear(PF_BUFFER_HEIGHT * PF_BUFFER_WIDTH);
	gPFMaskBufferHandle	= NewHandleClear(PF_BUFFER_HEIGHT * PF_BUFFER_WIDTH);

	GAME_ASSERT(gPFLookUpTable);
	GAME_ASSERT(gPFCopyLookUpTable);
	GAME_ASSERT(gPFMaskLookUpTable);
	GAME_ASSERT(gPFBufferHandle);
	GAME_ASSERT(gPFBufferCopyHandle);
	GAME_ASSERT(gPFMaskBufferHandle);

					/* BUILD SCREEN LOOKUP TABLE */

	gScreenLookUpTable = (uint8_t**) NewPtrClear(sizeof(uint8_t*) * VISIBLE_HEIGHT);
	GAME_ASSERT(gScreenLookUpTable);
	for (int i = 0; i < VISIBLE_HEIGHT; i++)
	{
		gScreenLookUpTable[i] = gIndexedFramebuffer + (VISIBLE_WIDTH * i);
	}

					/* BUILD PLAYFIELD LOOKUP TABLES */

	for (int i = 0; i < PF_BUFFER_HEIGHT; i++)
	{
		gPFLookUpTable[i]		= (uint8_t*)(*gPFBufferHandle)		+ (i * PF_BUFFER_WIDTH);
		gPFCopyLookUpTable[i]	= (uint8_t*)(*gPFBufferCopyHandle)	+ (i * PF_BUFFER_WIDTH);
		gPFMaskLookUpTable[i]	= (uint8_t*)(*gPFMaskBufferHandle)	+ (i * PF_BUFFER_WIDTH);
	}

					/* BUILD DITHERING FILTER BUFFER */

	gRowDitherStrides = (uint8_t*) NewPtrClear(gNumThreads * VISIBLE_WIDTH);
}


/************************ ERASE SCREEN AREA ********************/
//
// Copies an area from the Background to the Offscreen buffer.
// Then it creates an update region which will refresh the screen.
//

void EraseScreenArea(Rect theArea)
{
int32_t	*destPtr,*destStartPtr,*srcPtr,*srcStartPtr;
short	i;
short	width,height;
long	x,y;

	x = theArea.left;								// get x coord
	y = theArea.top;								// get y coord

	width = (theArea.right - x)>>2;
	height = (theArea.bottom - y);

	destStartPtr = (int32_t *)(gOffScreenLookUpTable[y]+x);	// calc read/write addrs
	srcStartPtr = (int32_t *)(gBackgroundLookUpTable[y]+x);

						/* DO THE ERASE */

	for (; height > 0; height--)
	{
		destPtr = destStartPtr;						// get line start ptrs
		srcPtr = srcStartPtr;

		for (i=0; i < width; i++)
			*destPtr++ = *srcPtr++;

		destStartPtr += (OFFSCREEN_WIDTH>>2);		// next row
		srcStartPtr += (OFFSCREEN_WIDTH>>2);
	}

	AddUpdateRegion(theArea,CLIP_REGION_SCREEN);			// create update region
}


/************************ BLANK SCREEN AREA ********************/
//
// Blanks part of the main screen to black
//

void BlankScreenArea(Rect theArea)
{
uint8_t	*destPtr;
short	width,height;
long	x,y;


	width = (theArea.right - (x = theArea.left));
	height = (theArea.bottom - (y = theArea.top));

	destPtr = gScreenLookUpTable[y] + x;			// calc write addr

						/* DO THE ERASE */

	for (; height > 0; height--)
	{
		memset(destPtr, 0xFF, width);

		destPtr += VISIBLE_WIDTH;					// next row
	}
}

#pragma mark -

/****************** SET GLOBAL SPRITE OFFSET - IN-GAME ***********************/

void SetScreenOffsetForArea(void)
{
	gScreenXOffset = 0;
	gScreenYOffset = 0;
}

/************* SET GLOBAL SPRITE OFFSET - NON-GAME SCREENS *******************/

void SetScreenOffsetFor640x480(void)
{
	gScreenXOffset = (VISIBLE_WIDTH - 640) / 2;
	gScreenYOffset = (VISIBLE_HEIGHT - 480) / 2;
}

#pragma mark -

/****************** CLEANUP DISPLAY *************************/

void CleanupDisplay(void)
{
#if GLRENDER
	GLRender_Shutdown();
#else
	SDLRender_Shutdown();
#endif

	DisposeScreenBuffers();
}

/****************** PRESENT FRAMEBUFFER *************************/

#if _DEBUG
static void SaveIndexedScreenshot(void)
{
	DumpIndexedTGA("/tmp/MikeIndexedScreenshot.tga", VISIBLE_WIDTH, VISIBLE_HEIGHT, (const char*) gIndexedFramebuffer);
}

void DumpIndexedTGA(const char* hostPath, int width, int height, const char* data)
{
	FILE* tga = fopen(hostPath, "wb");
	if (!tga)
	{
		DoAlert("Couldn't open screenshot file");
		return;
	}

	uint8_t tgaHeader[] = {
			0,
			1,
			1,		// image type: raw cmap
			0,0,	// pal origin
			0,1,	// pal size lo-hi (=256)
			24,		// pal bits per color
			0,0,0,0,	// origin
			width&0xFF,width>>8,
			height&0xFF,height>>8,
			8,		// bits per pixel
			1<<5,	// image descriptor (set flag for top-left origin)
	};

	// write header
	fwrite(tgaHeader, sizeof(tgaHeader), 1, tga);

	// write palette
	for (int i = 0; i < 256; i++)
	{
		fputc((gGamePalette.finalColors32[i]>>8)&0xFF, tga);
		fputc((gGamePalette.finalColors32[i]>>16)&0xFF, tga);
		fputc((gGamePalette.finalColors32[i]>>24)&0xFF, tga);
	}

	// write framebuffer
	fwrite(data, width, height, tga);

	// done
	fclose(tga);

	printf("wrote %s\n", hostPath);
}
#endif

void PresentIndexedFramebuffer(void)
{
	if (gScreenBlankedFlag)		// CLUT was blanked (in-between a fade-out and a fade-in), ignore
	{
		return;
	}

#if _DEBUG
	// Check screenshot key
	if (GetNewSDLKeyState(SDL_SCANCODE_F12))
	{
		SaveIndexedScreenshot();
	}
#endif

	//-------------------------------------------------------------------------
	// Present framebuffer

#if GLRENDER
	GLRender_PresentFramebuffer();
#else
	SDLRender_PresentFramebuffer();
#endif

	//-------------------------------------------------------------------------
	// Update debug info

	gDebugTextFrameAccumulator++;
	uint32_t ticksNow = SDL_GetTicks();
	uint32_t ticksElapsed = ticksNow - gDebugTextLastUpdatedAt;
	if (ticksElapsed >= kDebugTextUpdateInterval)
	{
		if (gGamePrefs.debugInfoInTitleBar && gGamePrefs.displayMode == kDisplayMode_Windowed)
		{
			float fps = 1000 * gDebugTextFrameAccumulator / (float)ticksElapsed;
			snprintf(
					gDebugTextBuffer, sizeof(gDebugTextBuffer),
					"Mike%s %s scl:%c thr:%d fps:%d obj:%ld x:%ld y:%ld",
					PROJECT_VERSION,
					gRendererName,
					'A' + gEffectiveScalingType,
					gNumThreads,
					(int)roundf(fps),
					NumObjects,
					gMyX,
					gMyY
			);
			SDL_SetWindowTitle(gSDLWindow, gDebugTextBuffer);
		}
		gDebugTextFrameAccumulator = 0;
		gDebugTextLastUpdatedAt = ticksNow;
	}
}

static void MoveToPreferredDisplay(void)
{
#if !(__APPLE__)
	int currentDisplay = SDL_GetWindowDisplayIndex(gSDLWindow);

	if (currentDisplay != gGamePrefs.preferredDisplay)
	{
		SDL_SetWindowPosition(
				gSDLWindow,
				SDL_WINDOWPOS_CENTERED_DISPLAY(gGamePrefs.preferredDisplay),
				SDL_WINDOWPOS_CENTERED_DISPLAY(gGamePrefs.preferredDisplay));
	}
#endif
}

void SetFullscreenMode(bool enforceDisplayPref)
{
	if (gGamePrefs.displayMode == kDisplayMode_Windowed)
	{
		SDL_SetWindowFullscreen(gSDLWindow, 0);
		if (enforceDisplayPref)
		{
			MoveToPreferredDisplay();
		}
	}
	else
	{
#if !(__APPLE__)
		if (enforceDisplayPref)
		{
			int currentDisplay = SDL_GetWindowDisplayIndex(gSDLWindow);

			if (currentDisplay != gGamePrefs.preferredDisplay)
			{
				// We must switch back to windowed mode for the preferred monitor to take effect
				SDL_SetWindowFullscreen(gSDLWindow, 0);
				MoveToPreferredDisplay();
			}
		}
#endif

		// Enter fullscreen mode
		SDL_SetWindowFullscreen(gSDLWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
	}

	SetOptimalWindowSize();
	OnChangeIntegerScaling();

	SDL_ShowCursor(gGamePrefs.displayMode == kDisplayMode_Windowed? 1: 0);
}

int GetMaxIntegerZoom(int displayWidth, int displayHeight)
{
	int maxZoomX = displayWidth / VISIBLE_WIDTH;
	int maxZoomY = displayHeight / VISIBLE_HEIGHT;
	int maxZoom = maxZoomX < maxZoomY ? maxZoomX : maxZoomY;

	return maxZoom <= 0? 1: maxZoom;
}

int GetMaxIntegerZoomForPreferredDisplay(void)
{
	int currentDisplay = SDL_GetWindowDisplayIndex(gSDLWindow);

#if !(__APPLE__)
	int numDisplays = SDL_GetNumVideoDisplays();
	if (gGamePrefs.preferredDisplay < numDisplays)
		currentDisplay = gGamePrefs.preferredDisplay;
#endif

	SDL_Rect displayBounds = {.x=0, .y=0, .w=VISIBLE_WIDTH, .h=VISIBLE_HEIGHT};
#if SDL_VERSION_ATLEAST(2,0,5)
	SDL_GetDisplayUsableBounds(currentDisplay, &displayBounds);
#else
	SDL_GetDisplayBounds(currentDisplay, &displayBounds);
#endif

	return GetMaxIntegerZoom(displayBounds.w, displayBounds.h);
}

void SetOptimalWindowSize(void)
{
	Uint32 windowFlags = SDL_GetWindowFlags(gSDLWindow);
	SDL_RestoreWindow(gSDLWindow);

	int maxZoom = GetMaxIntegerZoomForPreferredDisplay();
	int zoom = maxZoom;
	if (gGamePrefs.displayMode == kDisplayMode_Windowed)
	{
		int wantedZoom = gGamePrefs.windowedZoom;
		if (wantedZoom != 0)	// 0 is automatic
		{
			zoom = maxZoom < wantedZoom ? maxZoom : wantedZoom;
			windowFlags &= ~SDL_WINDOW_MAXIMIZED;  // don't restore maximized state if a hard zoom limit is set
		}
	}

	SDL_SetWindowSize(gSDLWindow, VISIBLE_WIDTH * zoom, VISIBLE_HEIGHT * zoom);
	SDL_SetWindowPosition(
			gSDLWindow,
			SDL_WINDOWPOS_CENTERED_DISPLAY(gGamePrefs.preferredDisplay),
			SDL_WINDOWPOS_CENTERED_DISPLAY(gGamePrefs.preferredDisplay));

	if (windowFlags & SDL_WINDOW_MAXIMIZED)
	{
		SDL_MaximizeWindow(gSDLWindow);
	}
}

static int GetEffectiveScalingType(void)
{
	int windowWidth = VISIBLE_WIDTH;
	int windowHeight = VISIBLE_HEIGHT;
	SDL_GetWindowSize(gSDLWindow, &windowWidth, &windowHeight);

	if (windowWidth < VISIBLE_WIDTH || windowHeight < VISIBLE_HEIGHT)
	{
		// If the window is smaller than the logical size,
		// SDL_RenderSetIntegerScale will cause the window to go black.
		return kScaling_Stretch;
	}
	else if (gGamePrefs.displayMode == kDisplayMode_FullscreenCrisp)
	{
		return kScaling_PixelPerfect;
	}
	else
	{
		float zoomX = windowWidth / (float)VISIBLE_WIDTH;
		float zoomY = windowHeight / (float)VISIBLE_HEIGHT;
		float uniformZoom = 0;
		float distanceToIntegerZoom = 0;

		if (zoomX < zoomY)
		{
			uniformZoom = zoomX;
			distanceToIntegerZoom = windowWidth - ((int)zoomX * VISIBLE_WIDTH);
		}
		else
		{
			uniformZoom = zoomY;
			distanceToIntegerZoom = windowHeight - ((int)zoomY * VISIBLE_HEIGHT);
		}

		if (distanceToIntegerZoom >= 0 && distanceToIntegerZoom <= kWiggleRoomCloseEnoughToIntScaling)
		{
			return kScaling_PixelPerfect;
		}
		else if (uniformZoom <= kHQStretchMinZoom)  // HQStretch doesn't look to good at 1x-1.5x zoom levels
		{
			return kScaling_Stretch;
		}
		else
		{
			return gCanDoHQStretch ? kScaling_HQStretch : kScaling_Stretch;
		}
	}
}

void OnChangeIntegerScaling(void)
{
	gEffectiveScalingType = GetEffectiveScalingType();

#if !(GLRENDER)
	SDLRender_InitTexture();
#endif
}

