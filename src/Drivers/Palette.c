/****************************/
/*    PALETTE MANAGER       */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/
#include "externs.h"
#include "io.h"
#include "myglobals.h"
#include "misc.h"
#include "window.h"

#include <math.h>
#include <string.h>

/****************************/
/*    CONSTANTS             */
/****************************/

static const int kFadeDurationMacTicks = 15;

/**********************/
/*     VARIABLES      */
/**********************/

GamePalette				gGamePalette;
static	GamePalette		gBackUpPalette;

Boolean					gScreenBlankedFlag = false;
uint16_t				gAppleRGBToLinear[1 << 16];
uint8_t					gLinearToSRGB[1 << 16];

/********************** INIT PALETTE STUFF ****************/

void InitPaletteStuff(void)
{
	// Init all colors to black
	for (int i = 0; i < 256; i++)
	{
		gGamePalette.baseColors[i]		= (RGBColor) {0,0,0};
		gGamePalette.finalColors32[i]	= 0x000000FF;
		gGamePalette.finalColors16[i]	= 0x0000;
		gBackUpPalette.baseColors[i]	= (RGBColor) {0,0,0};
		gBackUpPalette.finalColors32[i]	= 0x000000FF;
		gBackUpPalette.finalColors16[i]	= 0x0000;
	}

	for (int i = 0; i < (1 << 16); i++)
	{
		double linearScale = ((double)i) / (double)0xffff;
		double appleRGBToLinear = pow(linearScale, 1.8);
		gAppleRGBToLinear[i] = (uint16_t)(floor(appleRGBToLinear * 65535.0 + 0.5));

		double srgbIntensity = 0.0;
		if (linearScale < 0.0031308)
			srgbIntensity = linearScale / 12.92;
		else
			srgbIntensity = 1.055 * pow(linearScale, 1.0 / 2.4) - 0.055;

		gLinearToSRGB[i] = (uint8_t)floor(srgbIntensity * 255.0 + 0.5);
	}
}

/********************* MAKE BACKUP PALETTE *******************/

static void MakeBackUpPalette(void)
{
	memcpy(&gBackUpPalette, &gGamePalette, sizeof(GamePalette));
}

/********************* RESTORE BACKUP PALETTE *******************/

static void RestoreBackUpPalette(void)
{
	memcpy(&gGamePalette, &gBackUpPalette, sizeof(GamePalette));
}


/************************ FADE IN GAME CLUT ********************/

void FadeInGameCLUT(void)
{
						/* BACK UP TARGET COLORS */

	MakeBackUpPalette();
	
						/* UNLOCK PresentIndexedFramebuffer */

	gScreenBlankedFlag = false;

						/* FADE IN THE CLUT */

	const UInt32 start = TickCount();

	while (1)
	{
		int brightness = 100 * (int)(TickCount() - start) / kFadeDurationMacTicks;

		if (brightness > 100)
			break;
		
		for (int i = 0; i < 255; i++)
		{
			RGBColor rgbColor = gBackUpPalette.baseColors[i]; // get color from palette
			rgbColor.red	= (short)((int32_t)rgbColor.red		* brightness /100);
			rgbColor.green	= (short)((int32_t)rgbColor.green	* brightness /100);
			rgbColor.blue	= (short)((int32_t)rgbColor.blue	* brightness /100);
			SetPaletteColor(&gGamePalette, i, &rgbColor);		// set it
		}

		PresentIndexedFramebuffer();
		ReadKeyboard();		// flush keypresses
		RegulateSpeed2(1);
	}

						/* SET TO ORIGINAL PALETTE TO BE SURE */

	RestoreBackUpPalette();
	PresentIndexedFramebuffer();
}

/*********************** ERASE CLUT **********************/

void EraseCLUT(void)
{
	RGBColor rgbColor = {0, 0, 0};
	uint32_t color32 = RGBColorToU32(&rgbColor);
	uint16_t color16 = RGBColorToU16_565(&rgbColor);

			/* ZAP 0..254 */

	for (int i = 0; i < 255; i++)
	{
		gGamePalette.baseColors[i] = rgbColor;
		gGamePalette.finalColors32[i] = color32;
		gGamePalette.finalColors16[i] = color16;
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

	const UInt32 start = TickCount();

	while (1)
	{
		int brightness = 100 - (int)(100 * (TickCount() - start) / kFadeDurationMacTicks);

		if (brightness < 0)
			break;
		
		for (int i = 0; i < 255; i++)
		{
			RGBColor rgbColor = gBackUpPalette.baseColors[i]; // get color from palette
			rgbColor.red	= (short)((int32_t)rgbColor.red		* brightness /100);
			rgbColor.green	= (short)((int32_t)rgbColor.green	* brightness /100);
			rgbColor.blue	= (short)((int32_t)rgbColor.blue	* brightness /100);
			SetPaletteColor(&gGamePalette, i, &rgbColor);		// set it
		}

		PresentIndexedFramebuffer();
		ReadKeyboard();		// flush keypresses
		RegulateSpeed2(1);
	}

			/* LOCK PresentIndexedFramebuffer UNTIL NEXT FADEIN */

	gScreenBlankedFlag = true;

	RestoreBackUpPalette();
}

static void ResetSinglePaletteColorCorrection(struct GamePalette_s *palette)
{
	for (int i = 0; i < 256; i++)
	{
		SetPaletteColor(palette, i, &palette->baseColors[i]);
	}
}

void SetPaletteColorCorrection(void)
{
	ResetSinglePaletteColorCorrection(&gGamePalette);
	ResetSinglePaletteColorCorrection(&gBackUpPalette);
}

void SetPaletteColor(struct GamePalette_s *palette, int index, const RGBColor *color)
{
	uint32_t color32;
	uint16_t color16;

	if (gGamePrefs.colorCorrection)
	{
		int32_t argbRed = gAppleRGBToLinear[color->red];
		int32_t argbGreen = gAppleRGBToLinear[color->green];
		int32_t argbBlue = gAppleRGBToLinear[color->blue];
		
		int32_t srgbRed = argbRed * 17510 - argbGreen * 1288 + argbBlue * 162 + 8192;
		int32_t srgbGreen = argbRed * 395 + argbGreen * 15730 + argbBlue * 259 + 8192;
		int32_t srgbBlue = argbRed * 29 + argbGreen * 487 + argbBlue * 15868 + 8192;

		if (srgbRed < 0)
			srgbRed = 0;

		srgbRed >>= 14;
		srgbGreen >>= 14;
		srgbBlue >>= 14;

		if (srgbRed > 0xffff)
			srgbRed = 0xffff;

		uint8_t red = gLinearToSRGB[srgbRed];
		uint8_t green = gLinearToSRGB[srgbGreen];
		uint8_t blue = gLinearToSRGB[srgbBlue];

		color32 = 0x000000FF
				| (red << 24)
				| (green << 16)
				| (blue << 8);
		
		color16 = ((red >> 3) << 11)
				| ((green >> 2) << 5)
				| (blue >> 3);
	}
	else
	{
		color32 = RGBColorToU32(color);
		color16 = RGBColorToU16_565(color);
	}

	palette->baseColors[index] = *color;
	palette->finalColors32[index] = color32;
	palette->finalColors16[index] = color16;
}
