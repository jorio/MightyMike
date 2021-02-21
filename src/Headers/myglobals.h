/****************************/
/*         GLOBALS          */
/* (c)1994 Pangea Software  */
/*   By Brian Greenstone    */
/****************************/

#include "Equates.h"
#include "Structures.h"

#define		BETA	0			//-------------- 0 for final

#ifdef __powerc
#define		GAME_FPS		32L
#else
#define		GAME_FPS		31L
#endif

#define		GAME_SPEED		(1000L*1000L/GAME_FPS)

#define 	REMOVE_ALL_EVENTS	0
#define		MOVE_TO_FRONT		(WindowPtr)-1L
#define		NIL_STRING			""

#define	GET_PALETTE			true
#define	DONT_GET_PALETTE	false

#define	ONE_PLAYER		0
#define	TWO_PLAYER		1

/**************** PROTOTYPES *****************/




			/* PALETTE */

extern void	InitPaletteStuff(void);
extern void	BuildShapePalette(Byte);
extern void	RestoreDefaultCLUT(void);
extern void	ActivateCLUT(void);
extern void	FadeInGameCLUT(void);
extern void	EraseCLUT(void);
extern Boolean	ColorInGamePalette(RGBColor);
extern void	FadeOutGameCLUT(void);
extern void	MakeBackUpPalette(void);
extern void	DrawColors(void);

			/* ANIMATION */

extern void	AnimateASprite(ObjNode *);
extern void	SwitchAnim(ObjNode *, short);


		/* LZSS */

extern long	LZSS_Decode(short, Ptr, long);
