/****************************/
/*     PICTURE ROUTINES     */
/* (c)1993 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/
#include "myglobals.h"
#include "window.h"
#include "picture.h"
#include "misc.h"
#include "externs.h"
#include "tga.h"
#include <string.h>

/****************************/
/*    CONSTANTS             */
/****************************/



/**********************/
/*     VARIABLES      */
/**********************/

Handle	gBackgroundHandle = nil;


/************************ LOAD BACKGROUND *****************/

void LoadBackground(const char* fileName, Boolean getPalFlag)
{
	EraseBackgroundBuffer();

	int width;
	int height;
	Handle imageHandle = LoadTGA(fileName, getPalFlag, &width, &height);
	GAME_ASSERT_MESSAGE(imageHandle, fileName);					// load & unpack image file

						/* DRAW THE IMAGE */

	Ptr dest = (*gBackgroundHandle)+WINDOW_OFFSET;				// init pointers
	Ptr source = *imageHandle;

	for (int y = 0; y < height; y++)
	{
		memcpy(dest, source, width);
		dest += OFFSCREEN_WIDTH;								// next row
		source += width;
	}

	DisposeHandle(imageHandle);									// zap image data
}


/************************ LOAD IMAGE *****************/
//
// showMode determines how the IMAGE is to be displayed:
//
// NOTE: image is loaded directly onto the screen.
//

void LoadIMAGE(const char* fileName, short showMode)
{
	int width;
	int height;
	Handle imageHandle = LoadTGA(fileName, true, &width, &height);
	GAME_ASSERT_MESSAGE(imageHandle, fileName);			// load & unpack image file

				/* DUMP PIXEL IMAGE INTO BUFFER */

	GAME_ASSERT((int)width * (int)height <= (int)sizeof(gIndexedFramebuffer));

	const uint8_t* srcPtr = *imageHandle;
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

	DisposeHandle(imageHandle);					// nuke image data

						/* LETS SEE IT */

	if (showMode & SHOW_IMAGE_FLAG_FADEIN)
		FadeInGameCLUT();
	else
		PresentIndexedFramebuffer();
}



/********************** LOAD BORDER IMAGE *****************************/

void LoadBorderImage(void)
{
					/* OPEN THE FILE */

	const char* path;
	short flags = 0;

	switch (gGamePrefs.pfSize)
	{
	case PFSIZE_SMALL:
		path = ":images:border.tga";
		break;
	case PFSIZE_MEDIUM:
		path = ":images:border2.tga";
		flags |= LOADIMAGE_ALIGNBOTTOM;
		break;
	case PFSIZE_WIDE:
		path = ":images:border832.tga";
		flags |= SHOW_IMAGE_FLAG_ALIGNBOTTOM;
		break;
	default:
		GAME_ASSERT_MESSAGE(false, "Unknown pfSize!");
		return;
	}

	LoadIMAGE(path, flags);
}

