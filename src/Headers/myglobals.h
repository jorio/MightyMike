/****************************/
/*         GLOBALS          */
/* (c)1994 Pangea Software  */
/*   By Brian Greenstone    */
/****************************/

#include "equates.h"
#include "structures.h"

// Simulation frames per second.
// This is based on the PowerPC version. The game ran at 31 FPS on 68K.
#define		GAME_FPS		32L

// Duration of a simulation frame in microseconds.
#define		GAME_SPEED_MICROSECONDS		(1000L*1000L/(GAME_FPS))

// Duration of a simulation frame in milliseconds (SDL clock ticks).
#define		GAME_SPEED_SDL				(1000/(GAME_FPS))

#define	ONE_PLAYER		0
#define	TWO_PLAYER		1

/**************** PROTOTYPES *****************/


static inline uint32_t RGBColorToU32(const RGBColor* color)
{
	return	0x000000FF
			| ((color->red   >> 8) << 24)
			| ((color->green >> 8) << 16)
			| ((color->blue  >> 8) << 8);
}

static inline RGBColor U32ToRGBColor(const uint32_t color)
{
	return (RGBColor)
	{
			((color >> 24) & 0xFF) * 0x101,
			((color >> 16) & 0xFF) * 0x101,
			((color >> 8) & 0xFF) * 0x101,
	};
}


			/* PALETTE */

void	InitPaletteStuff(void);
void	FadeInGameCLUT(void);
void	EraseCLUT(void);
void	FadeOutGameCLUT(void);
void	SetPaletteColorCorrection(Boolean enabled);
void	SetPaletteColor(struct GamePalette_s *palette, int index, const RGBColor *color);

			/* ANIMATION */

void	AnimateASprite(ObjNode *);
void	SwitchAnim(ObjNode *, short);

