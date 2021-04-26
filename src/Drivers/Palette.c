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

#include "io.h"

/****************************/
/*    CONSTANTS             */
/****************************/

static const int kFadeDurationMacTicks = 15;

/**********************/
/*     VARIABLES      */
/**********************/

GamePalette				gGamePalette;
static	GamePalette		gBackUpPalette;

Boolean					gPaletteColorCorrectionFlag = true;
Boolean					gScreenBlankedFlag = false;
uint16_t				gAppleRGBToLinear[1 << 16];
uint8_t					gLinearToSRGB[1 << 16];

/********************** INIT PALETTE STUFF ****************/

void InitPaletteStuff(void)
{
	for (int i = 0; i < 256; i++)
	{
		for (int ch = 0; ch < 3; ch++)
		{
			gGamePalette.baseColors[i][ch]		= 0;
			gBackUpPalette.baseColors[i][ch]	= 0;
		}
		gGamePalette.finalColors[i]		= 0x000000FF;	// black
		gBackUpPalette.finalColors[i]	= 0x000000FF;
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
			const uint16_t *baseColor = gBackUpPalette.baseColors[i];
			RGBColor rgbColor = { baseColor[0], baseColor[1], baseColor[2] };	// get color from palette
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
	uint32_t color = RGBColorToU32(&rgbColor);

			/* ZAP 0..254 */

	for (int i = 0; i < 255; i++)
	{
		gGamePalette.finalColors[i] = color;				// assign color
		for (int ch = 0; ch < 3; ch++)
			gGamePalette.baseColors[i][ch] = 0;
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
			const uint16_t *baseColor = gBackUpPalette.baseColors[i];
			RGBColor rgbColor = { baseColor[0], baseColor[1], baseColor[2] };	// get color from palette
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
		const uint16_t *baseColor = palette->baseColors[i];
		RGBColor rgb = { baseColor[0], baseColor[1], baseColor[2] };
		SetPaletteColor(palette, i, &rgb);
	}
}

void SetPaletteColorCorrection(Boolean enabled)
{
	gPaletteColorCorrectionFlag = enabled;
	ResetSinglePaletteColorCorrection(&gGamePalette);
	ResetSinglePaletteColorCorrection(&gBackUpPalette);
}

void SetPaletteColor(struct GamePalette_s *palette, int index, const RGBColor *color)
{
	uint32_t rgba = 0;

	if (gPaletteColorCorrectionFlag)
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

		rgba = 0x000000FF
				| (red << 24)
				| (green << 16)
				| (blue << 8);
	}
	else
		rgba = RGBColorToU32(color);

	palette->baseColors[index][0] = color->red;
	palette->baseColors[index][1] = color->green;
	palette->baseColors[index][2] = color->blue;
	palette->finalColors[index] = rgba;
}
