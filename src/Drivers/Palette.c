/****************************/
/*    PALETTE MANAGER       */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/
#include "myglobals.h"
#include "misc.h"
#include "window.h"
#include <string.h>

/****************************/
/*    CONSTANTS             */
/****************************/

#if _DEBUG
static const int kFadeFrameDelayTicks = 0;		// faster fade in/out in debug mode
#else
static const int kFadeFrameDelayTicks = 2;
#endif

/**********************/
/*     VARIABLES      */
/**********************/

GamePalette				gGamePalette;
static	GamePalette		gBackUpPalette;

Boolean					gScreenBlankedFlag = false;


/********************** INIT PALETTE STUFF ****************/

void InitPaletteStuff(void)
{
	for (int i = 0; i < 256; i++)
	{
		gGamePalette[i]		= 0xFF000000 | i;
		gBackUpPalette[i]	= 0xFF000000 | i;
	}
}

/********************* MAKE BACKUP PALETTE *******************/

static void MakeBackUpPalette(void)
{
	memcpy(gBackUpPalette, gGamePalette, sizeof(GamePalette));
}

/********************* RESTORE BACKUP PALETTE *******************/

static void RestoreBackUpPalette(void)
{
	memcpy(gGamePalette, gBackUpPalette, sizeof(GamePalette));
}


/************************ FADE IN GAME CLUT ********************/

void FadeInGameCLUT(void)
{
						/* BACK UP TARGET COLORS */

	MakeBackUpPalette();
	
						/* UNLOCK PresentIndexedFramebuffer */

	gScreenBlankedFlag = false;

						/* FADE IN THE CLUT */

	for (int brightness = 4; brightness <= 100; brightness += 8)
	{
		for (int i = 0; i < 255; i++)
		{
			RGBColor rgbColor = U32ToRGBColor(gBackUpPalette[i]);	// get color from palette
			rgbColor.red	= (short)((int32_t)rgbColor.red		* brightness /100);
			rgbColor.green	= (short)((int32_t)rgbColor.green	* brightness /100);
			rgbColor.blue	= (short)((int32_t)rgbColor.blue	* brightness /100);
			gGamePalette[i] = RGBColorToU32(&rgbColor);				// set it
		}

		PresentIndexedFramebuffer();
		Wait(kFadeFrameDelayTicks);
	}

						/* SET TO ORIGINAL PALETTE TO BE SURE */

	RestoreBackUpPalette();
}

/*********************** ERASE CLUT **********************/

void EraseCLUT(void)
{
	RGBColor rgbColor = {0, 0, 0};
	uint32_t color = RGBColorToU32(&rgbColor);

			/* ZAP 0..254 */

	for (int i = 0; i < 255; i++)
	{
		gGamePalette[0] = color;				// assign color
	}

	gScreenBlankedFlag = true;
}



/************************ FADE OUT GAME CLUT ********************/
//
// BASES IT ON THE BACKUP PALETTE!!!!!!!!
//

void FadeOutGameCLUT(void)
{
	if (gScreenBlankedFlag)									// see if already out
		return;


	MakeBackUpPalette();									// (needs backup pal to do fade)

						/* FADE OUT THE CLUT */

	for (int brightness = 96; brightness >= 0; brightness -= 8)
	{
		for (int i = 0; i < 255; i++)
		{
			RGBColor rgbColor = U32ToRGBColor(gBackUpPalette[i]);	// get color from palette
			rgbColor.red	= (short)((int)rgbColor.red		* brightness /100);
			rgbColor.green	= (short)((int)rgbColor.green	* brightness /100);
			rgbColor.blue	= (short)((int)rgbColor.blue	* brightness /100);
			gGamePalette[i] = RGBColorToU32(&rgbColor);				// set it
		}

		PresentIndexedFramebuffer();
		Wait(kFadeFrameDelayTicks);
	}


			/* LOCK PresentIndexedFramebuffer UNTIL NEXT FADEIN */

	gScreenBlankedFlag = true;

	RestoreBackUpPalette();
}
