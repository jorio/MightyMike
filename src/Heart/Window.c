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

#include <SDL.h>
#include <string.h>

/****************************/
/*    PROTOTYPES            */
/****************************/

static void PrepDrawSprockets(void);
static void FilterDithering_Row(const uint8_t* indexedRow);


/****************************/
/*    CONSTANTS             */
/****************************/


/**********************/
/*     VARIABLES      */
/**********************/


uint8_t			gIndexedFramebuffer[VISIBLE_WIDTH * VISIBLE_HEIGHT];
uint8_t			gRGBAFramebuffer[VISIBLE_WIDTH * VISIBLE_HEIGHT * 4];

uint8_t			gRowDitherStrides[VISIBLE_WIDTH];		// for dithering filter

										// GAME STUFF
Handle			gOffScreenHandle = nil;
Handle			gPFBufferHandle = nil;
Handle			gPFBufferCopyHandle = nil;
Handle			gPFMaskBufferHandle = nil;

uint8_t*		gScreenAddr	= gIndexedFramebuffer;	// SCREEN ACCESS
long			gScreenRowOffset = VISIBLE_WIDTH;		// offset for bytes
long			gScreenRowOffsetLW = VISIBLE_WIDTH/4;	// offset for Long Words
long			gScreenXOffset,gScreenYOffset;


uint8_t*		gScreenLookUpTable[VISIBLE_HEIGHT];
uint8_t*		gOffScreenLookUpTable[OFFSCREEN_HEIGHT];
uint8_t*		gBackgroundLookUpTable[OFFSCREEN_HEIGHT];

Ptr				*gPFLookUpTable = nil;
Ptr				*gPFCopyLookUpTable = nil;
Ptr				*gPFMaskLookUpTable = nil;

Boolean			gLoadedDrawSprocket = false;
//DSpContextReference 	gDisplayContext = nil;



/********************** ERASE OFFSCREEN BUFFER ********************/

void EraseOffscreenBuffer(void)
{
	memset(*gOffScreenHandle, 0xFF, OFFSCREEN_WIDTH*OFFSCREEN_HEIGHT);	// clear to black
}

/********************** ERASE BACKGROUND BUFFER ********************/

void EraseBackgroundBuffer(void)
{
	memset(*gBackgroundHandle, 0xFF, OFFSCREEN_WIDTH*OFFSCREEN_HEIGHT);	// clear to black
}


/********************** MAKE GAME WINDOW ********************/

void MakeGameWindow(void)
{
	gScreenXOffset = 0;
	gScreenYOffset = 0;


	WindowToBlack();
	InitScreenBuffers();


#define CHECKED_DISPOSE(disposefunc, p) do { if (p) { disposefunc(p); (p) = nil; } } while(0)
	CHECKED_DISPOSE(DisposePtr, gPFLookUpTable);
	CHECKED_DISPOSE(DisposePtr, gPFCopyLookUpTable);
	CHECKED_DISPOSE(DisposePtr, gPFMaskLookUpTable);
	CHECKED_DISPOSE(DisposeHandle, gPFBufferHandle);
	CHECKED_DISPOSE(DisposeHandle, gPFBufferCopyHandle);
	CHECKED_DISPOSE(DisposeHandle, gPFMaskBufferHandle);
#undef CHECKED_DISPOSE
	

				/* ALLOC MEM FOR PF LOOKUP TABLES */

	gPFLookUpTable = NewPtr(PF_BUFFER_HEIGHT*sizeof(Ptr));
	gPFCopyLookUpTable = NewPtr(PF_BUFFER_HEIGHT*sizeof(Ptr));
	gPFMaskLookUpTable = NewPtr(PF_BUFFER_HEIGHT*sizeof(Ptr));


					/* MAKE PLAYFIELD BUFFERS */

	gPFBufferHandle = AllocHandle(PF_BUFFER_HEIGHT * PF_BUFFER_WIDTH);
	gPFBufferCopyHandle = AllocHandle(PF_BUFFER_HEIGHT * PF_BUFFER_WIDTH);
	gPFMaskBufferHandle = AllocHandle(PF_BUFFER_HEIGHT * PF_BUFFER_WIDTH);

	GAME_ASSERT(gPFLookUpTable);
	GAME_ASSERT(gPFCopyLookUpTable);
	GAME_ASSERT(gPFMaskLookUpTable);
	GAME_ASSERT(gPFBufferHandle);
	GAME_ASSERT(gPFBufferCopyHandle);
	GAME_ASSERT(gPFMaskBufferHandle);




	/******************** BUILD LOOKUP TABLES *******************/
	//
	// NOTE: Does NOT build the Offscreen & Background lookup tables (SEE INITSCREENBUFFERS)
	//

					/* BUILD SCREEN LOOKUP TABLE */

	for (int i = 0; i < VISIBLE_HEIGHT; i++)
		gScreenLookUpTable[i] = gScreenAddr + (gScreenRowOffset * i);

		
					/* BUILD PLAYFIELD LOOKUP TABLES */

	for (int i = 0; i < PF_BUFFER_HEIGHT; i++)
	{
		gPFLookUpTable[i]		= (*gPFBufferHandle)		+ (i * PF_BUFFER_WIDTH);
		gPFCopyLookUpTable[i]	= (*gPFBufferCopyHandle)	+ (i * PF_BUFFER_WIDTH);
		gPFMaskLookUpTable[i]	= (*gPFMaskBufferHandle)	+ (i * PF_BUFFER_WIDTH);
	}


	
	EraseGameWindow();												// erase buffer & screen
}


/********************** ERASE GAME WINDOW ******************/

void EraseGameWindow (void)
{
	EraseOffscreenBuffer();
	DumpGameWindow();
}

/********************** WINDOW TO BLACK *****************/

void WindowToBlack(void)
{
	memset(gIndexedFramebuffer, 0xFF, VISIBLE_WIDTH * VISIBLE_HEIGHT);
	PresentIndexedFramebuffer();
}

/********************** DUMP GAME WINDOW ****************/
//
// Dumps the full 640*480 screen (from larger buffer) to screen
//

void DumpGameWindow(void)
{
				/* GET SCREEN PIXMAP INFO */

	uint8_t* destPtr	= (uint8_t *)gScreenAddr;
	uint8_t* srcPtr		= (uint8_t *)(gOffScreenLookUpTable[0]+WINDOW_OFFSET);

						/* DO THE QUICK COPY */

	for (int y = 0; y < VISIBLE_HEIGHT; y++)
	{
		memcpy(destPtr, srcPtr, VISIBLE_WIDTH);

		destPtr += gScreenRowOffset;				// Bump to start of next row
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




/********************** WIPE SCREEN BUFFERS *************************/
//
// Deletes the Offscreen & Background buffers and wipes their xlate tables
//

void WipeScreenBuffers(void)
{
	if (gOffScreenHandle != nil)
	{
		DisposeHandle(gOffScreenHandle);
		gOffScreenHandle = nil;
	}

	if (gBackgroundHandle != nil)
	{
		DisposeHandle(gBackgroundHandle);
		gBackgroundHandle = nil;
	}

	for (int i = 0; i < OFFSCREEN_HEIGHT; i++)
	{
		gOffScreenLookUpTable[i] = nil;
		gBackgroundLookUpTable[i] = nil;
	}
}



/******************** ERASE STORE **************************/
//
// Blanks alternate lines for interlace mode
//

void EraseStore(void)
{
int32_t	*destPtr;
register	short		height,width;
register	long	destAdd,size;

				/* COPY PF BUFFER 2 TO BUFFER 1 TO ERASE STORE IMAGE */

	size = GetHandleSize(gPFBufferHandle);
	memcpy(*gPFBufferHandle, *gPFBufferCopyHandle, size);

				/* ERASE INTERLACING ZONE FROM MAIN SCREEN */

	if (gGamePrefs.interlaceMode)
	{
		destPtr = (int32_t *)(gScreenLookUpTable[PF_WINDOW_TOP+1]+PF_WINDOW_LEFT);
		destAdd = gScreenRowOffsetLW*2-(PF_WINDOW_WIDTH>>2);

		for ( height = PF_WINDOW_HEIGHT>>1; height > 0; height--)
		{
			for (width = PF_WINDOW_WIDTH>>2; width > 0; width--)
			{
				*destPtr++ = 0xfefefefe;				// dark grey
			}
			destPtr += destAdd;
		}
	}
}


/******************* DISPLAY STORE BUFFER *********************/
//
// The store image has been drawn into the primary PF buffer, so copy the PF buffer
// to the screen.
//

void DisplayStoreBuffer(void)
{
int32_t	*srcPtr,*destPtr,*destStart;
long	x,y,width,height;

	srcPtr = (int32_t *)(gPFLookUpTable[0]);
	if (gGamePrefs.pfSize != PFSIZE_SMALL)						// special for playfield display size
	{
		width = 13*32/4;
		height = 12*32;
		destStart = (int32_t *)(gScreenLookUpTable[PF_WINDOW_TOP+22]+PF_WINDOW_LEFT+110);
	}
	else
	{
		width = (PF_WINDOW_WIDTH/4);
		height = PF_WINDOW_HEIGHT;
		destStart = (int32_t *)(gScreenLookUpTable[PF_WINDOW_TOP]+PF_WINDOW_LEFT);
	}


	for (y = 0; y < height; y++)
	{
		destPtr = destStart;

		for (x = 0; x < width; x++)
			*destPtr++ = *srcPtr++;
		destStart += gScreenRowOffsetLW;
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
	WipeScreenBuffers();										// clear from any previous time



					/* MAKE OFFSCREEN DRAW BUFFER */

	gOffScreenHandle = AllocHandle(OFFSCREEN_WIDTH*OFFSCREEN_HEIGHT);
	GAME_ASSERT(gOffScreenHandle);


					/* MAKE BACKPLANE BUFFER */

	gBackgroundHandle = AllocHandle(OFFSCREEN_WIDTH*OFFSCREEN_HEIGHT);	// get mem for background
	GAME_ASSERT(gBackgroundHandle);


					/* BUILD OFFSCREEN LOOKUP TABLE */

	for (int i = 0; i < OFFSCREEN_HEIGHT; i++)
	{
		gOffScreenLookUpTable[i] = (*gOffScreenHandle)+(i*OFFSCREEN_WIDTH);
	}
					/* BUILD BACKGROUND LOOKUP TABLE */

	for (int i = 0; i < OFFSCREEN_HEIGHT; i++)
	{
		gBackgroundLookUpTable[i] = (*gBackgroundHandle)+(i*OFFSCREEN_WIDTH);
	}
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

		destPtr += gScreenRowOffset;				// next row
	}
}



#pragma mark -

/****************** PREP DRAW SPROCKETS *********************/

static void PrepDrawSprockets(void)
{
	TODO_REWRITE_THIS();
#if 0
DSpContextAttributes 	displayConfig;
OSStatus 				theError;
Boolean					confirmIt = false;

        /* SEE IF DSP EXISTS */

    if ((void *)DSpStartup == (void *)kUnresolvedCFragSymbolAddress)
		DoFatalAlert("You do not seem to have Draw Sprocket installed.  This game requires Draw Sprocket to function.  To install Apple's Game Sprockets, go to www.pangeasoft.net/downloads.html");


		/* startup DrawSprocket */

	theError = DSpStartup();
	if( theError )
	{
		DoFatalAlert("DSpStartup failed!");
	}
	gLoadedDrawSprocket = true;


				/*************************/
				/* SETUP A REQUEST BLOCK */
				/*************************/

	displayConfig.frequency					= 00;
	displayConfig.displayWidth				= VISIBLE_WIDTH;
	displayConfig.displayHeight				= VISIBLE_HEIGHT;
	displayConfig.reserved1					= 0;
	displayConfig.reserved2					= 0;
	displayConfig.colorNeeds				= kDSpColorNeeds_Request;
	displayConfig.colorTable				= nil;
	displayConfig.contextOptions			= 0;
	displayConfig.backBufferDepthMask		= kDSpDepthMask_8;
	displayConfig.displayDepthMask			= kDSpDepthMask_8;
	displayConfig.backBufferBestDepth		= 8;
	displayConfig.displayBestDepth			= 8;
	displayConfig.pageCount					= 1;
	displayConfig.filler[0]                 = 0;
	displayConfig.filler[1]                 = 0;
	displayConfig.filler[2]                 = 0;
	displayConfig.gameMustConfirmSwitch		= false;
	displayConfig.reserved3[0]				= 0;
	displayConfig.reserved3[1]				= 0;
	displayConfig.reserved3[2]				= 0;
	displayConfig.reserved3[3]				= 0;

				/* AUTOMATICALLY FIND BEST CONTEXT */

	theError = DSpFindBestContext( &displayConfig, &gDisplayContext );
	if (theError)
	{
		DoFatalAlert("PrepDrawSprockets: DSpFindBestContext failed");
	}

				/* RESERVE IT */

	theError = DSpContext_Reserve( gDisplayContext, &displayConfig );
	if( theError )
		DoFatalAlert("PrepDrawSprockets: DSpContext_Reserve failed");


			/* MAKE STATE ACTIVE */

	theError = DSpContext_SetState( gDisplayContext, kDSpContextState_Active );
	if (theError == kDSpConfirmSwitchWarning)
	{
		confirmIt = true;
	}
	else
	if (theError)
	{
		DSpContext_Release( gDisplayContext );
		gDisplayContext = nil;
		DoFatalAlert("PrepDrawSprockets: DSpContext_SetState failed");
	}
#endif
}


/****************** CLEANUP DISPLAY *************************/

void CleanupDisplay(void)
{
	TODO_REWRITE_THIS_MINOR();
#if 0
OSStatus 		theError;

	if (gDisplayContext != nil)
	{
		DSpContext_SetState( gDisplayContext, kDSpContextState_Inactive );	// deactivate
		DSpContext_Release( gDisplayContext );								// release

		gDisplayContext = nil;
	}


	/* shutdown draw sprocket */

	if (gLoadedDrawSprocket)
	{
		theError = DSpShutdown();
		gLoadedDrawSprocket = false;
	}


//	{
//        GDHandle 	gdh;
//    	gdh = GetMainDevice();
//	    SetDepth(gdh,16,1,1);				//------ set to 16-bit
//    }
#endif
}


/****************** PRESENT FRAMEBUFFER *************************/

static void ConvertIndexedFramebufferToRGBA_NoFilter(void)
{
	uint32_t* rgba = (uint32_t*) gRGBAFramebuffer;
	const uint8_t* indexed = &gIndexedFramebuffer[0];

	for (int y = 0; y < VISIBLE_HEIGHT; y++)
	{
		for (int x = 0; x < VISIBLE_WIDTH; x++)
		{
			*(rgba++) = gGamePalette[*(indexed++)];
		}
	}
}

static void ConvertIndexedFramebufferToRGBA_FilterDithering(void)
{
	uint32_t* rgba = (uint32_t*) gRGBAFramebuffer;
	const uint8_t* indexed = &gIndexedFramebuffer[0];

	// initialize row bleed: no dithering by default
	memset(gRowDitherStrides, 0, VISIBLE_WIDTH);

	for (int y = 0; y < VISIBLE_HEIGHT; y++)
	{
		uint8_t* smearFlag = gRowDitherStrides;
		FilterDithering_Row(indexed);

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

static inline void FilterDithering_Row(const uint8_t* indexedRow)
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
		memset(gRowDitherStrides+ditherStart, 1, ditherLength+BLEED);		\
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
	
	// Check dithering key
//	if (GetNewSDLKeyState(SDL_SCANCODE_F10))
//	{
//		filterDithering = !filterDithering;
//	}


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

