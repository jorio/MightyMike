/****************************/
/*         GLOBALS          */
/* (c)1994 Pangea Software  */
/*   By Brian Greenstone    */
/****************************/

#include "equates.h"
#include "structures.h"

#define		BETA	0			//-------------- 0 for final

#ifdef __powerc
#define		GAME_FPS		32L
#else
#define		GAME_FPS		31L
#endif

#define		GAME_SPEED_MICROSECONDS		(1000L*1000L/(GAME_FPS))
#define		GAME_SPEED_SDL				(1000/(GAME_FPS))

#define 	REMOVE_ALL_EVENTS	0

#define	GET_PALETTE			true
#define	DONT_GET_PALETTE	false

#define	ONE_PLAYER		0
#define	TWO_PLAYER		1

/**************** PROTOTYPES *****************/


typedef uint32_t GamePalette[256];

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
			((color >> 24) & 0xFF) << 8,
			((color >> 16) & 0xFF) << 8,
			((color >> 8) & 0xFF) << 8,
	};
}


			/* PALETTE */

extern void	InitPaletteStuff(void);
extern void	ActivateCLUT(void);
extern void	FadeInGameCLUT(void);
extern void	EraseCLUT(void);
extern void	FadeOutGameCLUT(void);

			/* ANIMATION */

extern void	AnimateASprite(ObjNode *);
extern void	SwitchAnim(ObjNode *, short);


		/* LZSS */

extern long	LZSS_Decode(short, Ptr, long);
