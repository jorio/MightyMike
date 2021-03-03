/****************************/
/*     PICTURE ROUTINES     */
/* (c)1993 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/
#include "myglobals.h"
//#include <pictutils.h>
//#include <palettes.h>
#include "window.h"
#include "picture.h"
#include "misc.h"
#include <string.h>

extern	GamePalette		gGamePalette;
extern	Handle			gOffScreenHandle;
extern	uint8_t*		gScreenAddr;
extern	long			gScreenRowOffset;
extern	Boolean			gPPCFullScreenFlag;
extern	uint8_t			gIndexedFramebuffer[VISIBLE_WIDTH * VISIBLE_HEIGHT];

/****************************/
/*    CONSTANTS             */
/****************************/



/**********************/
/*     VARIABLES      */
/**********************/

Handle	gBackgroundHandle = nil;


/************************ LOAD BACKGROUND *****************/

void LoadBackground(Str255 fileName,Boolean getPalFlag)
{
	EraseBackgroundBuffer();

	Handle theImageHand = LoadPackedFile(fileName);				// load & unpack image file


					/* GET COLOR INFO FOR IMAGE */
	if (getPalFlag)
	{
		RGBColor* rgbPtr = (RGBColor *)(*theImageHand);			// get ptr to palette data
		ByteswapInts(2, 256*3, rgbPtr);							// byteswap colors (each component is 16-bit)
		for (int i = 0; i < 256; i++)
		{
			gGamePalette[i] = RGBColorToU32(&rgbPtr[i]);
		}
	}


						/* DRAW THE IMAGE */

	int16_t* ptr16 = (int16_t *)(*theImageHand + (256*2*3));	// get width & height
	int16_t width	= Byteswap16(ptr16++);
	int16_t height	= Byteswap16(ptr16++);

	Ptr dest = (*gBackgroundHandle)+WINDOW_OFFSET;				// init pointers
	Ptr source = (Ptr)ptr16;

	for (int y = 0; y < height; y++)
	{
		memcpy(dest, source, width);

		dest += OFFSCREEN_WIDTH;								// next row
		source += width;
	}

	DisposeHandle(theImageHand);								// zap image data
}


/************************ LOAD IMAGE *****************/
//
// showMode determines how the IMAGE is to be displayed:
//
// NOTE: image is loaded directly onto the screen.
//

void LoadIMAGE(Str255 fileName,short showMode)
{
	Handle imageHandle = LoadPackedFile(fileName);			// load & unpack image file

					/* GET COLOR INFO FOR PICTURE */

	RGBColor* rgbPtr = (RGBColor *)(*imageHandle);				// get ptr to palette data
	ByteswapInts(2, 256*3, rgbPtr);						// byteswap colors (each component is 16-bit)
	for (int i = 0; i < 256; i++)
	{
		gGamePalette[i] = RGBColorToU32(&rgbPtr[i]);
	}

				/* DUMP PIXEL IMAGE INTO BUFFER */

	int16_t* ptr16	= (int16_t *)(*imageHandle + (256*2*3));	// get width & height
	int16_t width	= Byteswap16(ptr16++);
	int16_t height	= Byteswap16(ptr16++);

	GAME_ASSERT(width * height <= sizeof(gIndexedFramebuffer));

	const uint8_t* srcPtr = (const uint8_t*) ptr16;
	uint8_t* destPtr = gIndexedFramebuffer;

	// offset X
	destPtr += (VISIBLE_WIDTH - width) / 2;

	// offset Y
	if (showMode & SHOW_IMAGE_FLAG_ALIGNBOTTOM)
		destPtr += gScreenRowOffset * (VISIBLE_HEIGHT - height);
	else
		destPtr += gScreenRowOffset * (VISIBLE_HEIGHT - height) / 2;

	for (int y = 0; y < height; y++)
	{
		memcpy(destPtr, srcPtr, width);
		destPtr += gScreenRowOffset;
		srcPtr += width;
	}

	PresentIndexedFramebuffer();

	DisposeHandle(imageHandle);					// nuke image data

						/* LETS SEE IT */

//	gColorListSize = 255;							// force the CLUT size

	if (showMode & SHOW_IMAGE_FLAG_FADEIN)
		FadeInGameCLUT();
	else
		ActivateCLUT();
}



/********************** LOAD BORDER IMAGE *****************************/
//
// The border image is saved as a non-compressed .image file.  It is not packed
// so that it can easily be drawn to the screen without a loading buffer or decompression buffer.
// Since it is the last thing loaded before beginning a level, memory
// is at a premium.
//

void LoadBorderImage(void)
{
					/* OPEN THE FILE */

	const char* path;
	short flags = 0;

	if (gPPCFullScreenFlag)
		path = ":data:images:border2.image";
	else
		path = ":data:images:border.image";

#if WIDESCREEN
	flags |= SHOW_IMAGE_FLAG_ALIGNBOTTOM;
#endif

	LoadIMAGE(path, flags);
}

