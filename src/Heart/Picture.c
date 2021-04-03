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

/************************ LOAD BACKGROUND *****************/

void LoadBackground(const char* fileName)
{
	LoadImage(fileName, LOADIMAGE_BACKGROUND);
}

/************************ LOAD IMAGE *****************/
//
// showMode determines how the IMAGE is to be displayed:
//

void LoadImage(const char* fileName, short showMode)
{
	const bool		getPalette = true;
	int				width;
	int				height;
	uint8_t*		destPtr;
	int				destRowBytes;
	const uint8_t*	srcPtr;

				/* LOAD TGA FILE */

	Handle imageHandle = LoadTGA(fileName, getPalette, &width, &height);
	GAME_ASSERT_MESSAGE(imageHandle, fileName);			// load & unpack image file

	GAME_ASSERT(width <= VISIBLE_WIDTH);				// image must fit on screen
	GAME_ASSERT(height <= VISIBLE_HEIGHT);

				/* GET DESTINATION POINTER */

	if (showMode & LOADIMAGE_BACKGROUND)
	{
		destPtr = (uint8_t*) *gBackgroundHandle;
		destPtr += OFFSCREEN_WINDOW_TOP*OFFSCREEN_WIDTH;
		destPtr += OFFSCREEN_WINDOW_LEFT;
		destRowBytes = OFFSCREEN_WIDTH;
		EraseBackgroundBuffer();
	}
	else
	{
		destPtr = gIndexedFramebuffer;
		destRowBytes = gScreenRowOffset;
	}

				/* OFFSET DESTINATION POINTER */

	// offset X
	destPtr += (VISIBLE_WIDTH - width) / 2;

	// offset Y
	if (showMode & LOADIMAGE_ALIGNBOTTOM)
		destPtr += destRowBytes * (VISIBLE_HEIGHT - height);
	else
		destPtr += destRowBytes * (VISIBLE_HEIGHT - height) / 2;

				/* DUMP PIXEL IMAGE INTO BUFFER */

	srcPtr = (const uint8_t*) *imageHandle;

	for (int y = 0; y < height; y++)
	{
		memcpy(destPtr, srcPtr, width);
		destPtr += destRowBytes;
		srcPtr += width;
	}

	DisposeHandle(imageHandle);					// nuke image data

						/* LETS SEE IT */

	if (showMode & LOADIMAGE_FADEIN)
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
		flags |= LOADIMAGE_ALIGNBOTTOM;
		break;
	default:
		GAME_ASSERT_MESSAGE(false, "Unknown pfSize!");
		return;
	}

	LoadImage(path, flags);
}

