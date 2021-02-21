/****************************/
/*     PICTURE ROUTINES     */
/* (c)1993 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/
#include "myglobals.h"
#include <pictutils.h>
#include <palettes.h>
#include "windows.h"
#include "picture.h"
#include "misc.h"

extern	PaletteHandle	gGamePalette;
extern short				gColorListSize;
extern	Handle			gOffScreenHandle;
extern	Ptr				gScreenAddr;
extern	char  			gMMUMode;
extern	long			gScreenRowOffsetLW,gScreenRowOffset;
extern	Boolean			gPPCFullScreenFlag;

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
register	long	*source,*dest;
long		*destStart;
Handle		theImageHand;
register 	short		i,x,y,width,height,*intPtr;
RGBColor	*rgbPtr,rgb;

	EraseBackgroundBuffer();

	theImageHand = LoadPackedFile(fileName);				// load & unpack image file


					/* GET COLOR INFO FOR IMAGE */
	if (getPalFlag)
	{
		gColorListSize = 256;								// assume its a full palette
		rgbPtr = (RGBColor *)(*theImageHand);				// get ptr to palette data
		for (i=0; i<256; i++)
		{
			rgb = *rgbPtr++;								// get a color
			SetEntryColor(gGamePalette,i,&rgb);				// set
		}
	}


						/* DRAW THE IMAGE */

	intPtr = (short *)(*theImageHand + (sizeof(RGBColor)*256));	// get width & height
	width = (*intPtr++)>>2;
	height = *intPtr++;

	destStart = (long *)((*gBackgroundHandle)+WINDOW_OFFSET);			// init pointers
	source = (long *)intPtr;

	for (y=0; y<height; y++)
	{
		dest = destStart;
		for (x=0; x<width; x++)
		{
			*dest++ = *source++;							// get a pixel
		}
		destStart += (OFFSCREEN_WIDTH>>2);					// next row
	}

	DisposeHandle(theImageHand);							// zap image data
}

/************************ LOAD BACKGROUND: DIRECT *****************/
//
// Loads directly to main screen.  Does not store to any offscreen buffers.
//

void LoadBackground_Direct(Str255 fileName,Boolean getPalFlag)
{
register	long	*source,*dest;
long		*destStart;
Handle		theImageHand;
register 	short		i,x,y,width,height,*intPtr;
RGBColor		*rgbPtr,rgb;

	theImageHand = LoadPackedFile(fileName);				// load & unpack image file


					/* GET COLOR INFO FOR IMAGE */
	if (getPalFlag)
	{
		gColorListSize = 256;								// assume its a full palette
		rgbPtr = (RGBColor *)(*theImageHand);				// get ptr to palette data
		for (i=0; i<256; i++)
		{
			rgb = *rgbPtr++;								// get a color
			SetEntryColor(gGamePalette,i,&rgb);				// set
		}
	}


						/* DRAW THE IMAGE */

	intPtr = (short *)(*theImageHand + (sizeof(RGBColor)*256));	// get width & height
	width = (*intPtr++)>>2;
	height = *intPtr++;

	destStart = (long *)gScreenAddr;				// init pointers
	source = (long *)StripAddress((Ptr)intPtr);

//	gMMUMode = true32b;								// we must do this in 32bit addressing mode
//	SwapMMUMode(&gMMUMode);

	for (y=0; y<height; y++)
	{
		dest = destStart;
		for (x=0; x<width; x++)
		{
			*dest++ = *source++;					// get a pixel
		}
		destStart += gScreenRowOffsetLW;			// next row
	}

	DisposeHandle(theImageHand);					// zap image data

//	SwapMMUMode(&gMMUMode);							// Restore addressing mode
}


/************************ LOAD IMAGE *****************/
//
// showMode determines how the IMAGE is to be displayed:
//
// NOTE: image is loaded directly onto the screen.
//

void LoadIMAGE(Str255 fileName,short showMode)
{

Handle		theImageHand;
register	short		i,x,y,width,height,*intPtr;
RGBColor	*rgbPtr,rgb;
long		*source,*dest,*destStart;

	EraseCLUT();

	theImageHand = LoadPackedFile(fileName);			// load & unpack image file


					/* GET COLOR INFO FOR PICTURE */

	rgbPtr = (RGBColor *)(*theImageHand);				// get ptr to palette data
	for (i=0; i<256; i++)
	{
		rgb = *rgbPtr++;								// get a color
		SetEntryColor(gGamePalette,i,&rgb);				// set
	}

				/* DUMP PIXEL IMAGE INTO BUFFER */

	intPtr = (short *)rgbPtr;								// get width & height
	width = (*intPtr++)>>2;
	height = *intPtr++;

	destStart = (long *)gScreenAddr;					// init pointers
	source = (long *)StripAddress((Ptr)intPtr);

//	gMMUMode = true32b;									// we must do this in 32bit addressing mode
//	SwapMMUMode(&gMMUMode);


	for (y=0; y<height; y++)
	{
		dest = destStart;
		for (x=0; x<width; x++)							// copy it
			*dest++ = *source++;
		destStart += gScreenRowOffsetLW;				// next row
	}

//	SwapMMUMode(&gMMUMode);								// Restore addressing mode

	DisposeHandle(theImageHand);					// nuke image data

						/* LETS SEE IT */

	gColorListSize = 255;							// force the CLUT size

	switch(showMode)
	{
		case SHOW_IMAGE_MODE_FADEIN:
			FadeInGameCLUT();
			break;
		case SHOW_IMAGE_MODE_QUICK:
			ActivateCLUT();
			break;
		case SHOW_IMAGE_MODE_NOSHOW:
			break;
	}
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
short			width,height;
register short	i,k;
long			numToRead;
short			fRefNum,vRefNum;
register Ptr	linePtr,destPtr,srcPtr;
RGBColor		*rgbPtr,rgb;


					/* OPEN THE FILE */

	GetVol(nil,&vRefNum);									// get default volume

	if (gPPCFullScreenFlag)
		OpenMikeFile(":data:images:border2.image",&fRefNum,"Cant open Border Image!");
	else
		OpenMikeFile(":data:images:border.image",&fRefNum,"Cant open Border Image!");

					/* READ & SET THE PALETTE */

	SetFPos(fRefNum,fsFromStart,8);							// skip pack header
	rgbPtr = (RGBColor *)AllocPtr(256*sizeof(RGBColor));	// alloc memory to hold colors
	numToRead = 256*sizeof(RGBColor);
	FSRead(fRefNum,&numToRead,(Ptr)rgbPtr);					// read color data
	gColorListSize = 256;
	for (i=0; i<256; i++)
	{
		rgb = *rgbPtr++;									// get a color
		SetEntryColor(gGamePalette,i,&rgb);					// set
	}
	DisposePtr((Ptr)rgbPtr);

	SetFPos(fRefNum,fsFromStart,256*sizeof(RGBColor)+8);	// skip palette & pack header

					/* READ & DRAW IMAGE */

	numToRead = 2;
	FSRead(fRefNum,&numToRead,&width);						// read width
	numToRead = 2;
	FSRead(fRefNum,&numToRead,&height);						// read height

	linePtr = AllocPtr(width*4);							// alloc memory to hold 4 lines of data

//	gMMUMode = true32b;										// we must do this in 32bit addressing mode
//	SwapMMUMode(&gMMUMode);

	destPtr = gScreenAddr;
	height = height/4;
	for (i = 0; i < height; i++)
	{
		numToRead = width*4;								// read 4 lines of data
		FSRead(fRefNum,&numToRead,linePtr);

		srcPtr = StripAddress(linePtr);
		for (k=0; k < 4; k++)
		{
			BlockMove(srcPtr,destPtr,width);				// plot line
			destPtr += gScreenRowOffset;					// next row
			srcPtr += width;
		}
	}

//	SwapMMUMode(&gMMUMode);									// Restore addressing mode

	FSClose(fRefNum);
	DisposePtr((Ptr)linePtr);
}



