/****************************/
/*        WINDOW            */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/
#include "myglobals.h"
#include "window.h"
#include "playfield.h"
#include "object.h"
#include "misc.h"
#include "input.h"
#include "externs.h"
#include <version.h>

#include <SDL.h>
#include <string.h>

#ifdef _OPENMP
	#include <omp.h>
#else
	#define omp_get_thread_num() 0
	#define omp_get_max_threads() 1
#endif

/****************************/
/*    PROTOTYPES            */
/****************************/

static void InitScreenBuffers(void);
static void FilterDithering_Row(const uint8_t* indexedRow, uint8_t* rowSmearFlags);


/****************************/
/*    CONSTANTS             */
/****************************/


/**********************/
/*     VARIABLES      */
/**********************/


int				VISIBLE_WIDTH = 640;			// dimensions of visible area (MULTIPLE OF 4!!!)
int				VISIBLE_HEIGHT = 480;

uint8_t*		gIndexedFramebuffer = nil;		// [VISIBLE_WIDTH * VISIBLE_HEIGHT]
uint8_t*		gRGBAFramebuffer = nil;			// [VISIBLE_WIDTH * VISIBLE_HEIGHT * 4]

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

Ptr				*gPFLookUpTable = nil;
Ptr				*gPFCopyLookUpTable = nil;
Ptr				*gPFMaskLookUpTable = nil;

static const uint32_t	kDebugTextUpdateInterval = 200;
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

	SetRect(&r,0,0,VISIBLE_WIDTH,VISIBLE_HEIGHT);
	BlankScreenArea(r);
}

/********************* INIT SCREEN BUFFERS ***********************/
//
// Create the Offscreen & Background buffers with xlate tables
//

void InitScreenBuffers(void)
{
	CHECKED_DISPOSEPTR(gIndexedFramebuffer);
	CHECKED_DISPOSEPTR(gRGBAFramebuffer);

	CHECKED_DISPOSEHANDLE(gOffScreenHandle);
	CHECKED_DISPOSEHANDLE(gBackgroundHandle);
	CHECKED_DISPOSEPTR(gOffScreenLookUpTable);
	CHECKED_DISPOSEPTR(gBackgroundLookUpTable);

	CHECKED_DISPOSEPTR(gPFLookUpTable);
	CHECKED_DISPOSEPTR(gPFCopyLookUpTable);
	CHECKED_DISPOSEPTR(gPFMaskLookUpTable);

	CHECKED_DISPOSEHANDLE(gPFBufferHandle);
	CHECKED_DISPOSEHANDLE(gPFBufferCopyHandle);
	CHECKED_DISPOSEHANDLE(gPFMaskBufferHandle);

	CHECKED_DISPOSEPTR(gRowDitherStrides);

					/* MAKE INDEXED FRAMEBUFFER */

	gIndexedFramebuffer = (uint8_t*) NewPtrClear(VISIBLE_WIDTH * VISIBLE_HEIGHT);
	GAME_ASSERT(gIndexedFramebuffer);

	// Clear to black
	memset(gIndexedFramebuffer, 0xFF, VISIBLE_WIDTH * VISIBLE_HEIGHT);

					/* MAKE RGBA FRAMEBUFFER */

	gRGBAFramebuffer = (uint8_t*) NewPtrClear(VISIBLE_WIDTH * VISIBLE_HEIGHT * 4);
	GAME_ASSERT(gRGBAFramebuffer);

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

	gPFLookUpTable		= (Ptr*) NewPtrClear(PF_BUFFER_HEIGHT*sizeof(Ptr));
	gPFCopyLookUpTable	= (Ptr*) NewPtrClear(PF_BUFFER_HEIGHT*sizeof(Ptr));
	gPFMaskLookUpTable	= (Ptr*) NewPtrClear(PF_BUFFER_HEIGHT*sizeof(Ptr));

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
	for (int i = 0; i < VISIBLE_HEIGHT; i++)
	{
		gScreenLookUpTable[i] = gIndexedFramebuffer + (VISIBLE_WIDTH * i);
	}

					/* BUILD PLAYFIELD LOOKUP TABLES */

	for (int i = 0; i < PF_BUFFER_HEIGHT; i++)
	{
		gPFLookUpTable[i]		= (*gPFBufferHandle)		+ (i * PF_BUFFER_WIDTH);
		gPFCopyLookUpTable[i]	= (*gPFBufferCopyHandle)	+ (i * PF_BUFFER_WIDTH);
		gPFMaskLookUpTable[i]	= (*gPFMaskBufferHandle)	+ (i * PF_BUFFER_WIDTH);
	}

					/* BUILD DITHERING FILTER BUFFER */

	gRowDitherStrides = (uint8_t*) NewPtrClear(omp_get_max_threads() * VISIBLE_WIDTH);
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
	if (gRowDitherStrides != nil)
	{
		DisposePtr((Ptr) gRowDitherStrides);
		gRowDitherStrides = nil;
	}
}


/****************** PRESENT FRAMEBUFFER *************************/

static void ConvertIndexedFramebufferToRGBA_NoFilter(void)
{
#pragma omp parallel for schedule(static) default(none) \
		shared(gGamePalette, gRGBAFramebuffer, gIndexedFramebuffer, VISIBLE_WIDTH, VISIBLE_HEIGHT)
	for (int y = 0; y < VISIBLE_HEIGHT; y++)
	{
		uint32_t* rgba			= ((uint32_t*) gRGBAFramebuffer) + y * VISIBLE_WIDTH;
		const uint8_t* indexed	= gIndexedFramebuffer + y * VISIBLE_WIDTH;

		for (int x = 0; x < VISIBLE_WIDTH; x++)
		{
			*(rgba++) = gGamePalette[*(indexed++)];
		}
	}
}

static void ConvertIndexedFramebufferToRGBA_FilterDithering(void)
{
	// initialize row bleed: no dithering by default
	if (gRowDitherStrides == nil)
	{
		gRowDitherStrides = (uint8_t*) NewPtrClear(omp_get_max_threads() * VISIBLE_WIDTH);
#if _DEBUG
		printf("Rendering threads: %d\n", omp_get_max_threads());
#endif
	}

#pragma omp parallel for schedule(static) default(none) \
		shared(gGamePalette, gRGBAFramebuffer, gIndexedFramebuffer, gRowDitherStrides, VISIBLE_WIDTH, VISIBLE_HEIGHT)
	for (int y = 0; y < VISIBLE_HEIGHT; y++)
	{
		uint32_t* rgba			= ((uint32_t*) gRGBAFramebuffer) + y * VISIBLE_WIDTH;
		const uint8_t* indexed	= gIndexedFramebuffer + y * VISIBLE_WIDTH;

		uint8_t* smearFlag		= &gRowDitherStrides[omp_get_thread_num() * VISIBLE_WIDTH];
		FilterDithering_Row(indexed, smearFlag);

		for (int x = 0; x < VISIBLE_WIDTH-1; x++)
		{
			if (*smearFlag)
			{
				uint8_t* me		= (uint8_t*) &gGamePalette[*indexed];
				uint8_t* next	= (uint8_t*) &gGamePalette[*(indexed+1)];
				uint8_t* out	= (uint8_t*) rgba;
				out[1] = (me[1] + next[1]) >> 1;
				out[2] = (me[2] + next[2]) >> 1;
				out[3] = (me[3] + next[3]) >> 1;
			}
			else
				*rgba = gGamePalette[*indexed];

			rgba++;
			indexed++;

			*smearFlag = 0;	// clear for next row
			smearFlag++;
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
		fputc((gGamePalette[i]>>8)&0xFF, tga);
		fputc((gGamePalette[i]>>16)&0xFF, tga);
		fputc((gGamePalette[i]>>24)&0xFF, tga);
	}

	// write framebuffer
	fwrite(data, width, height, tga);

	// done
	fclose(tga);

	printf("wrote %s\n", hostPath);
}

void PresentIndexedFramebuffer(void)
{
	if (gScreenBlankedFlag)		// CLUT was blanked (in-between a fade-out and a fade-in), ignore
	{
		return;
	}

	// Check screenshot key
//	if (CheckNewKeyDown2(kVK_F12, &kdScreenshot))
//	{
//		SaveIndexedScreenshot();
//	}

	//-------------------------------------------------------------------------
	// Convert indexed to RGBA, with optional post-processing

	if (!gGamePrefs.filterDithering)
	{
		ConvertIndexedFramebufferToRGBA_NoFilter();
	}
	else
	{
		ConvertIndexedFramebufferToRGBA_FilterDithering();
	}

	//-------------------------------------------------------------------------
	// Update SDL texture and swap buffers

	SDL_UpdateTexture(gSDLTexture, NULL, gRGBAFramebuffer, VISIBLE_WIDTH*4);
	SDL_RenderClear(gSDLRenderer);
	SDL_RenderCopy(gSDLRenderer, gSDLTexture, NULL, NULL);
	SDL_RenderPresent(gSDLRenderer);

	//-------------------------------------------------------------------------
	// Update debug info

	gDebugTextFrameAccumulator++;
	uint32_t ticksNow = SDL_GetTicks();
	uint32_t ticksElapsed = ticksNow - gDebugTextLastUpdatedAt;
	if (ticksElapsed >= kDebugTextUpdateInterval)
	{
		float fps = 1000 * gDebugTextFrameAccumulator / (float)ticksElapsed;
		snprintf(
				gDebugTextBuffer, sizeof(gDebugTextBuffer),
				"Mighty Mike %s - fps:%d - x:%ld y:%ld",
				PROJECT_VERSION,
				(int)roundf(fps),
				gMyX,
				gMyY
		);
		SDL_SetWindowTitle(gSDLWindow, gDebugTextBuffer);
		gDebugTextFrameAccumulator = 0;
		gDebugTextLastUpdatedAt = ticksNow;
	}
}

void SetFullscreenMode(void)
{
	SDL_SetWindowFullscreen(gSDLWindow, gGamePrefs.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

void OnChangeIntegerScaling(void)
{
	bool crisp;

	if (!gGamePrefs.integerScaling)
	{
		// Stretch: don't use nearest-neighbor
		crisp = false;
	}
	else
	{
		int windowWidth = VISIBLE_WIDTH;
		int windowHeight = VISIBLE_HEIGHT;
		SDL_GetWindowSize(gSDLWindow, &windowWidth, &windowHeight);

		if (windowWidth < VISIBLE_WIDTH || windowHeight < VISIBLE_HEIGHT)
		{
			// If the window is smaller than the logical size,
			// SDL_RenderSetIntegerScale will cause the window to go black.
			crisp = false;
		}
		else
		{
			crisp = true;
		}
	}

	// Nuke old texture
	if (gSDLTexture)
	{
		SDL_DestroyTexture(gSDLTexture);
		gSDLTexture = NULL;
	}

	// Set scaling quality before creating texture
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, crisp ? "nearest" : "best");

	// Recreate texture
	gSDLTexture = SDL_CreateTexture(gSDLRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, VISIBLE_WIDTH, VISIBLE_HEIGHT);
	GAME_ASSERT(gSDLTexture);

	// Set integer scaling setting
	SDL_RenderSetIntegerScale(gSDLRenderer, crisp);
}

