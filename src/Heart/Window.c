/****************************/
/*        WINDOW            */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/
#include "myglobals.h"
//#include <qdoffscreen.h>
//#include <pictutils.h>
#include "window.h"
#include "playfield.h"
#include "object.h"
#include "misc.h"
//#include 	<DrawSprocket.h>

#include <SDL.h>

extern	Boolean		gPPCFullScreenFlag;
extern	Handle		gBackgroundHandle;
extern	Ptr			gSHAPE_HEADER_Ptrs[];
extern	ObjNode		*gMyNodePtr;
extern	short		gLevelNum;
extern	unsigned char	gInterlaceMode;
#if __USE_PF_VARS
extern long	PF_TILE_HEIGHT;
extern long	PF_TILE_WIDTH;
extern long	PF_WINDOW_TOP;
extern long	PF_WINDOW_LEFT;
#endif

extern	GamePalette			gGamePalette;
extern SDL_Window*			gSDLWindow;
extern SDL_Renderer*		gSDLRenderer;
extern SDL_Texture*			gSDLTexture;


/****************************/
/*    PROTOTYPES            */
/****************************/

static void PrepDrawSprockets(void);


/****************************/
/*    CONSTANTS             */
/****************************/


/**********************/
/*     VARIABLES      */
/**********************/


uint8_t			gIndexedFramebuffer[VISIBLE_WIDTH * VISIBLE_HEIGHT];
uint8_t			gRGBAFramebuffer[VISIBLE_WIDTH * VISIBLE_HEIGHT * 4];

										// GAME STUFF
Handle			gOffScreenHandle = nil;
Handle			gPFBufferHandle = nil;
Handle			gPFBufferCopyHandle = nil;
Handle			gPFMaskBufferHandle = nil;

uint8_t*		gScreenAddr	= gIndexedFramebuffer;	// SCREEN ACCESS
char  			gMMUMode;
long			gScreenRowOffset = 640;		// offset for bytes
long			gScreenRowOffsetLW;		// offset for Long Words
long			gScreenXOffset,gScreenYOffset;


uint8_t*		gScreenLookUpTable[VISIBLE_HEIGHT];
long			gOffScreenLookUpTable[OFFSCREEN_HEIGHT];
long			gBackgroundLookUpTable[OFFSCREEN_HEIGHT];

long			*gPFLookUpTable = nil;
long			*gPFCopyLookUpTable = nil;
long			*gPFMaskLookUpTable = nil;

Boolean			gLoadedDrawSprocket = false;
//DSpContextReference 	gDisplayContext = nil;



/********************** ERASE OFFSCREEN BUFFER ********************/

void EraseOffscreenBuffer(void)
{
	long		i;
	Ptr 	tempPtr;

	tempPtr = *gOffScreenHandle;

	for (i=0; i<(OFFSCREEN_WIDTH*OFFSCREEN_HEIGHT); i++)
			*tempPtr++ = 0xff;											// clear to black

}

/********************** ERASE BACKGROUND BUFFER ********************/

void EraseBackgroundBuffer(void)
{
long		i;
Ptr 	tempPtr;

	if (gBackgroundHandle != nil)
	{
		tempPtr = *gBackgroundHandle;

		for (i=0; i<(OFFSCREEN_WIDTH*OFFSCREEN_HEIGHT); i++)
				*tempPtr++ = 0xff;											// clear to black
	}
}


/********************** MAKE GAME WINDOW ********************/

void MakeGameWindow(void)
{
WindowPtr		gGameWindow = nil;
GDHandle		gMainScreen;			// SCREEN ACCESS
PixMapHandle	gMainScreenPixMap;
Rect		screenRect;
short		width,height;

#if 0
	PrepDrawSprockets();			// use DSp just to resize screen


				/* GET SCREEN DRAWING VARIABLES */

//#ifdef __MWERKS__
//	screenRect = qd.screenBits.bounds;
//#else
//	screenRect = screenBits.bounds;
//#endif

	gMainScreen = GetMainDevice();
	screenRect = (**gMainScreen).gdRect;
	gMainScreenPixMap = (**gMainScreen).gdPMap;
	gScreenAddr = StripAddress(GetPixBaseAddr(gMainScreenPixMap));
	gScreenRowOffset = (long)(0x3fff&(**gMainScreenPixMap).rowBytes);	// High bit of pixMap rowBytes must be cleared.
	gScreenRowOffsetLW = gScreenRowOffset>>2;


	width = screenRect.right-screenRect.left;				// center for large monitors
	height = screenRect.bottom-screenRect.top;
	gScreenXOffset = ((width-640)/2)&0xfffffff8;			// 8pix align
	gScreenYOffset = (height-480)/2;
	if ((gScreenXOffset<0)||(gScreenYOffset<0))
		DoFatalAlert("640*480 Minimum Screen Required!");

	gScreenAddr += (gScreenYOffset*gScreenRowOffset)+gScreenXOffset;	// calc new addr


				/* CLEAR SCREEN & MAKE WINDOW */

									/* cover up the part of the screen we are writing on */
									/* with a window, so other apps will not mess with our */
									/* animation via update events in the background */

	gGameWindow = NewCWindow(nil, &screenRect, "", true, plainDBox,	// make new window to cover screen
								 MOVE_TO_FRONT, false, nil);
#endif
	gScreenXOffset = 0;
	gScreenYOffset = 0;


	WindowToBlack();
	InitScreenBuffers();


				/* ALLOC MEM FOR PF LOOKUP TABLES */

	gPFLookUpTable = (long *)NewPtr(PF_BUFFER_HEIGHT*sizeof(long));
	gPFCopyLookUpTable = (long *)NewPtr(PF_BUFFER_HEIGHT*sizeof(long));
	gPFMaskLookUpTable = (long *)NewPtr(PF_BUFFER_HEIGHT*sizeof(long));


					/* MAKE PLAYFIELD BUFFERS */

	if ((gPFBufferHandle = AllocHandle(PF_BUFFER_HEIGHT*PF_BUFFER_WIDTH)) == nil)
		DoFatalAlert ("No Memory for gPFBufferHandle!");
	HLockHi(gPFBufferHandle);

	if ((gPFBufferCopyHandle = AllocHandle(PF_BUFFER_HEIGHT*PF_BUFFER_WIDTH)) == nil)
		DoFatalAlert ("No Memory for gPFBufferCopyHandle!");
	HLockHi(gPFBufferCopyHandle);

	if ((gPFMaskBufferHandle = AllocHandle(PF_BUFFER_HEIGHT*PF_BUFFER_WIDTH)) == nil)
		DoFatalAlert ("No Memory for gPFMaskBufferHandle!");
	HLockHi(gPFMaskBufferHandle);


//	SetPort(gGameWindow);
	BuildLookUpTables();											// build all graphic lookup tbls
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
	TODO_REWRITE_THIS_MINOR();
#if 0
GrafPort	killMenuBar;
									/* This covers up the menu bar */
									/* and fills the screen with black */

	OpenPort((GrafPtr)&killMenuBar);
	BackColor(blackColor);
	EraseRect(&killMenuBar.portRect);
#endif
}

/********************** DUMP GAME WINDOW ****************/
//
// Dumps the full 640*480 screen (from larger buffer) to screen
//

void DumpGameWindow(void)
{
	TODO_REWRITE_THIS_MINOR();
#if 0
register	long			*destPtr,*srcPtr;
register	short				row;
register	long			*srcStartPtr,*destStartPtr;


				/* GET SCREEN PIXMAP INFO */

	destStartPtr = (long *)gScreenAddr;
	srcStartPtr	= 	(long *)(gOffScreenLookUpTable[0]+WINDOW_OFFSET);

//	gMMUMode = true32b;									// we must do this in 32bit addressing mode
//	SwapMMUMode(&gMMUMode);


						/* DO THE QUICK COPY */

		for (row=0; row<VISIBLE_HEIGHT; row++)
		{
			destPtr = destStartPtr;						// get line start ptrs
			srcPtr = srcStartPtr;

			*destPtr++ = *srcPtr++;						// in-line 640 byte copy code
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;

			*destPtr++ = *srcPtr++;						//10..19
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;

			*destPtr++ = *srcPtr++;						//20..29
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;

			*destPtr++ = *srcPtr++;						//30..39
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;

			*destPtr++ = *srcPtr++;						//40..49
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;

			*destPtr++ = *srcPtr++;						//50..59
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;

			*destPtr++ = *srcPtr++;						//60..69
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;

			*destPtr++ = *srcPtr++;						//70..79
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;

			*destPtr++ = *srcPtr++;						//80..89
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;

			*destPtr++ = *srcPtr++;						//90..99
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;

			*destPtr++ = *srcPtr++;						//100..109
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;

			*destPtr++ = *srcPtr++;						//110..119
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;

			*destPtr++ = *srcPtr++;						//120..129
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;

			*destPtr++ = *srcPtr++;						//130..139
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;

			*destPtr++ = *srcPtr++;						//140..149
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;

			*destPtr++ = *srcPtr++;						//150..159
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;
			*destPtr++ = *srcPtr++;


			destStartPtr += gScreenRowOffsetLW;				// Bump to start of next row.
			srcStartPtr += (OFFSCREEN_WIDTH/4);
		}

//		SwapMMUMode(&gMMUMode);						/* Restore addressing mode */
#endif
}



/********************** DUMP BACKGROUND ****************/
//
// Copy background image to playfield image buffer
//

void DumpBackground(void)
{
register	long	*destPtr,*srcPtr,i;

	destPtr = 	(long *)gOffScreenLookUpTable[0];
	srcPtr	= 	(long *)gBackgroundLookUpTable[0];

	for (i=0; i<(OFFSCREEN_WIDTH*OFFSCREEN_HEIGHT/4); i++)
			*destPtr++ = *srcPtr++;

}


/******************** BUILD LOOKUP TABLES *******************/
//
// NOTE: Does NOT build the Offscreen & Background lookup tables (SEE INITSCREENBUFFERS)
//

void BuildLookUpTables(void)
{
long	i;
					/* BUILD SCREEN LOOKUP TABLE */

	for (i=0; i<VISIBLE_HEIGHT; i++)
		gScreenLookUpTable[i] = gScreenAddr + (gScreenRowOffset*i);


					/* BUILD PLAYFIELD LOOKUP TABLES */

	for (i=0; i<PF_BUFFER_HEIGHT; i++)
	{
		gPFLookUpTable[i] = (long)(*gPFBufferHandle)+(i*PF_BUFFER_WIDTH);
		gPFLookUpTable[i] = (long)StripAddress((Ptr)gPFLookUpTable[i]);

		gPFCopyLookUpTable[i] = (long)(*gPFBufferCopyHandle)+(i*PF_BUFFER_WIDTH);
		gPFCopyLookUpTable[i] = (long)StripAddress((Ptr)gPFCopyLookUpTable[i]);

		gPFMaskLookUpTable[i] = (long)(*gPFMaskBufferHandle)+(i*PF_BUFFER_WIDTH);
		gPFMaskLookUpTable[i] = (long)StripAddress((Ptr)gPFMaskLookUpTable[i]);
	}
}




/********************** WIPE SCREEN BUFFERS *************************/
//
// Deletes the Offscreen & Background buffers and wipes their xlate tables
//

void WipeScreenBuffers(void)
{
short		i;

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

	for (i = 0; i < OFFSCREEN_HEIGHT; i++)
	{
		gOffScreenLookUpTable[i] = 0xffffffffL;
		gBackgroundLookUpTable[i] = 0xffffffffL;
	}
}



/******************** ERASE STORE **************************/
//
// Blanks alternate lines for interlace mode
//

void EraseStore(void)
{
register	long	*destPtr;
register	short		height,width;
register	long	destAdd,size,i;
Ptr		a,b;

				/* COPY PF BUFFER 2 TO BUFFER 1 TO ERASE STORE IMAGE */

	size = GetHandleSize(gPFBufferHandle);
	a = *gPFBufferHandle;
	b = *gPFBufferCopyHandle;
	for (i = 0; i < size; i++)
	{
		*a++ = *b++;
	}

				/* ERASE INTERLACING ZONE FROM MAIN SCREEN */

	if (gInterlaceMode)
	{
//		gMMUMode = true32b;										// we must do this in 32bit addressing mode
//		SwapMMUMode(&gMMUMode);


		destPtr = (long *)(gScreenLookUpTable[PF_WINDOW_TOP+1]+PF_WINDOW_LEFT);
		destAdd = gScreenRowOffsetLW*2-(PF_WINDOW_WIDTH>>2);

		for ( height = PF_WINDOW_HEIGHT>>1; height > 0; height--)
		{
			for (width = PF_WINDOW_WIDTH>>2; width > 0; width--)
			{
				*destPtr++ = 0xfefefefe;				// dark grey
			}
			destPtr += destAdd;
		}

//		SwapMMUMode(&gMMUMode);						// Restore addressing mode
	}
}


/******************* DISPLAY STORE BUFFER *********************/
//
// The store image has been drawn into the primary PF buffer, so copy the PF buffer
// to the screen.
//

void DisplayStoreBuffer(void)
{
long 	*srcPtr,*destPtr,*destStart;
long	x,y,width,height;

	srcPtr = (long *)(gPFLookUpTable[0]);
	if (gPPCFullScreenFlag)								// special for playfield display size
	{
		width = 13*32/4;
		height = 12*32;
		destStart = (long *)(gScreenLookUpTable[PF_WINDOW_TOP+22]+PF_WINDOW_LEFT+110);
	}
	else
	{
		width = (PF_WINDOW_WIDTH/4);
		height = PF_WINDOW_HEIGHT;
		destStart = (long *)(gScreenLookUpTable[PF_WINDOW_TOP]+PF_WINDOW_LEFT);
	}


//	gMMUMode = true32b;										// we must do this in 32bit addressing mode
//	SwapMMUMode(&gMMUMode);

	for (y = 0; y < height; y++)
	{
		destPtr = destStart;

		for (x = 0; x < width; x++)
			*destPtr++ = *srcPtr++;
		destStart += gScreenRowOffsetLW;
	}

//	SwapMMUMode(&gMMUMode);						// Restore addressing mode
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
short		i;

	WipeScreenBuffers();										// clear from any previous time



					/* MAKE OFFSCREEN DRAW BUFFER */

	gOffScreenHandle = AllocHandle(OFFSCREEN_WIDTH*OFFSCREEN_HEIGHT);
	if	(gOffScreenHandle == nil)
		DoFatalAlert("Not Enough Memory for OffScreen Buffer!");
	HLock(gOffScreenHandle);


					/* MAKE BACKPLANE BUFFER */

	if ((gBackgroundHandle = AllocHandle(OFFSCREEN_WIDTH*OFFSCREEN_HEIGHT))	// get mem for background
			 == nil)
		DoFatalAlert ("No Memory for Background buffer!");
	HLock(gBackgroundHandle);


					/* BUILD OFFSCREEN LOOKUP TABLE */

	for (i=0; i<OFFSCREEN_HEIGHT; i++)
	{
		gOffScreenLookUpTable[i] = (long)(*gOffScreenHandle)+(i*OFFSCREEN_WIDTH);
		gOffScreenLookUpTable[i] = (long)StripAddress((Ptr)gOffScreenLookUpTable[i]);

	}
					/* BUILD BACKGROUND LOOKUP TABLE */

	for (i=0; i<OFFSCREEN_HEIGHT; i++)
	{
		gBackgroundLookUpTable[i] = (long)(*gBackgroundHandle)+(i*OFFSCREEN_WIDTH);
		gBackgroundLookUpTable[i] = (long)StripAddress((Ptr)gBackgroundLookUpTable[i]);
	}


}



#ifdef __powerc
//=======================================================================================
//                  POWERPC CODE
//=======================================================================================

/************************ ERASE SCREEN AREA ********************/
//
// Copies an area from the Background to the Offscreen buffer.
// Then it creates an update region which will refresh the screen.
//

void EraseScreenArea(Rect theArea)
{
long	*destPtr,*destStartPtr,*srcPtr,*srcStartPtr;
short	i;
short	width,height;
long	x,y;

	x = theArea.left;								// get x coord
	y = theArea.top;								// get y coord

	width = (theArea.right - x)>>2;
	height = (theArea.bottom - y);

	destStartPtr = (long *)(gOffScreenLookUpTable[y]+x);	// calc read/write addrs
	srcStartPtr = (long *)(gBackgroundLookUpTable[y]+x);

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
long	*destPtr,*destStartPtr;
short	i;
short	width,height;
long	x,y;


	width = (theArea.right - (x = theArea.left))>>2;
	height = (theArea.bottom - (y = theArea.top));

	destStartPtr = (long *)(gScreenLookUpTable[y]+x);	// calc write addr

						/* DO THE ERASE */

//	gMMUMode = true32b;									// we must do this in 32bit addressing mode
//	SwapMMUMode(&gMMUMode);

	for (; height > 0; height--)
	{
		destPtr = destStartPtr;						// get line start ptrs

		for (i=0; i < width; i++)
			*destPtr++ = 0xffffffffL;

		destStartPtr += gScreenRowOffsetLW;				// next row
	}
//	SwapMMUMode(&gMMUMode);								// Restore addressing mode
}


//=======================================================================================
//                  68000 CODE
//=======================================================================================
#else

/************************ ERASE SCREEN AREA ********************/
//
// Copies an area from the Background to the Offscreen buffer.
// Then it creates an update region which will refresh the screen.
//

void EraseScreenArea(Rect theArea)
{
register	long	*destPtr,*destStartPtr,*srcPtr,*srcStartPtr;
register	short		col;
register	short		width,height;
static		long	x,y;

	x = theArea.left;								// get x coord
	y = theArea.top;								// get y coord

	width = (theArea.right - x)>>2;
	height = (theArea.bottom - y);

	destStartPtr = (long *)(gOffScreenLookUpTable[y]+x);	// calc read/write addrs
	srcStartPtr = (long *)(gBackgroundLookUpTable[y]+x);

						/* DO THE ERASE */

	for (; height > 0; height--)
	{
		destPtr = destStartPtr;						// get line start ptrs
		srcPtr = srcStartPtr;

		col = (112-width)<<1;

		TODO_REWRITE_ASM();
#if 0	// TODO REWRITE ASM!
		asm
		{
				jmp		@inline(col)

			@inline
				move.l	(srcPtr)+,(destPtr)+	//0..15 longs
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+

				move.l	(srcPtr)+,(destPtr)+	//16..31
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+

				move.l	(srcPtr)+,(destPtr)+	//32..47
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+

				move.l	(srcPtr)+,(destPtr)+	//48..63
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+

				move.l	(srcPtr)+,(destPtr)+	//64..79
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+

				move.l	(srcPtr)+,(destPtr)+	//80..95
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+

				move.l	(srcPtr)+,(destPtr)+	//96..111
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+

			}
#endif

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
register	long	*destPtr,*destStartPtr;
register	short		col;
register	short		width,height;
static		long	x,y;


	width = (theArea.right - (x = theArea.left))>>2;
	height = (theArea.bottom - (y = theArea.top));

	destStartPtr = (long *)(gScreenLookUpTable[y]+x);	// calc write addr

						/* DO THE ERASE */

//	gMMUMode = true32b;									// we must do this in 32bit addressing mode
//	SwapMMUMode(&gMMUMode);

	for (; height > 0; height--)
	{
		printf("TODO! %s\n", __func__ );

#if 0	// TODO REWRITE ASM!
		destPtr = destStartPtr;						// get line start ptrs

		col = (160-width)*6;

		asm
		{
				jmp		@inline(col)

			@inline
				move.l	#0xffffffff,(destPtr)+	//0..15 longs
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+

				move.l	#0xffffffff,(destPtr)+	//16..31
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+

				move.l	#0xffffffff,(destPtr)+	//32..47
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+

				move.l	#0xffffffff,(destPtr)+	//48..63
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+

				move.l	#0xffffffff,(destPtr)+	//64..79
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+

				move.l	#0xffffffff,(destPtr)+	//80..95
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+

				move.l	#0xffffffff,(destPtr)+	//96..111
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+

				move.l	#0xffffffff,(destPtr)+	//112..127
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+

				move.l	#0xffffffff,(destPtr)+	//128..144
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+

				move.l	#0xffffffff,(destPtr)+	//145..159
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+
				move.l	#0xffffffff,(destPtr)+

			}
#endif

		destStartPtr += gScreenRowOffsetLW;				// next row
	}

//	SwapMMUMode(&gMMUMode);								// Restore addressing mode
}

#endif


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

void PresentIndexedFramebuffer(void)
{
	uint32_t* rgba = (uint32_t*) gRGBAFramebuffer;
	const uint8_t* indexed = &gIndexedFramebuffer[0];

	for (int y = 0; y < VISIBLE_HEIGHT; y++)
	{
		for (int x = 0; x < VISIBLE_WIDTH; x++)
		{
			*rgba = gGamePalette[*indexed];
			rgba++;
			indexed++;
		}
	}

	SDL_UpdateTexture(gSDLTexture, NULL, gRGBAFramebuffer, VISIBLE_WIDTH*4);
	SDL_RenderClear(gSDLRenderer);
	SDL_RenderCopy(gSDLRenderer, gSDLTexture, NULL, NULL);
	SDL_RenderPresent(gSDLRenderer);
}
