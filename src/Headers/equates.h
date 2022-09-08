/****************************/
/*       EQUATES            */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/

#pragma once

#define		MAX_COLLISIONS		40

#define		MAX_GLOBAL_FLAGS	10

#define		MAX_SHAPE_GROUPS	10
#define		MAX_OBJECTS			200
#define		MAX_REGIONS			(MAX_OBJECTS*2)
#define		MAX_CLIP_REGIONS	5					// see reserved clip regions

#define		MAX_SCENES	5							// 5 scenes in game: jurassic, candy, etc...


enum
{
	SCENE_JURASSIC,
	SCENE_CANDY,
	SCENE_FAIRY,
	SCENE_CLOWN,
	SCENE_BARGAIN
};


//================================================================================


							// COLLISION SIDE INFO
							//=================================

#define	SIDE_BITS_TOP		(1)							// %0001	(r/l/b/t)
#define	SIDE_BITS_BOTTOM	(1<<1)						// %0010
#define	SIDE_BITS_LEFT		(1<<2)						// %0100
#define	SIDE_BITS_RIGHT		(1<<3)						// %1000
#define	ALL_SOLID_SIDES		(SIDE_BITS_TOP|SIDE_BITS_BOTTOM|SIDE_BITS_LEFT|SIDE_BITS_RIGHT)

#define	COLLISION_TYPE_TILE		0
#define	COLLISION_TYPE_OBJ		1


							// CBITS (32 BIT VALUES)
							//==================================

enum
{
	CBITS_TOP 			= SIDE_BITS_TOP,
	CBITS_BOTTOM 		= SIDE_BITS_BOTTOM,
	CBITS_LEFT 			= SIDE_BITS_LEFT,
	CBITS_RIGHT 		= SIDE_BITS_RIGHT,
	CBITS_ALLSOLID		= ALL_SOLID_SIDES,
	CBITS_TOUCHABLE		= (1L<<4)			// %10000
};


							// CTYPES (32 BIT VALUES)
							//==================================

enum
{
	CTYPE_MYGUY		=	1,				// %0000000000000001	Me
	CTYPE_ENEMYA	=	(1L<<1),		// %0000000000000010	Enemy
	CTYPE_ENEMYB	=	(1L<<2),		// %0000000000000100	Enemy projectile (stops when hits me)
	CTYPE_BONUS		=	(1L<<3),		// %0000000000001000    Bonus item
	CTYPE_MYBULLET	=	(1L<<4),		// %0000000000010000    My Bullet
	CTYPE_TRIGGER	=	(1L<<5),		// %0000000000100000    Trigger
	CTYPE_BGROUND	=	(1L<<6),		// %0000000001000000    BGround
	CTYPE_MISC		=	(1L<<7),		// %0000000010000000    Misc
	CTYPE_ENEMYC	=	(1L<<8),		// %0000000100000000	Enemy projectile (will go thru me) / generic harmful things
	CTYPE_HEALTH	=	(1L<<9),		// %0000001000000000	MUST be combined with BONUS to form a health bonus
	CTYPE_MPLATFORM	=	(1L<<10),		// %0000010000000000    MPlatform
	CTYPE_KEY 		=	(1L<<11),		// %0000100000000000    Key Type
	CTYPE_WEAPONPOW	=	(1L<<12),		// %0001000000000000    Weapon Powerup
	CTYPE_MISCPOW	=	(1L<<13),		// %0010000000000000    Misc Powerup
	CTYPE_HURTENEMY = 	(1L<<14)		// &0100000000000000	Misc hurt Enemy item
};

#define INVALID_NODE_FLAG 0xffffffffL	// put into CType when node is deleted


			/* AIMING VALUES */

enum
{
	AIM_UP = 0,
	AIM_UP_RIGHT,
	AIM_RIGHT,
	AIM_DOWN_RIGHT,
	AIM_DOWN,
	AIM_DOWN_LEFT,
	AIM_LEFT,
	AIM_UP_LEFT
};


		/* DIFFICULTY VALUES */

enum
{
	DIFFICULTY_EASY		= 0,
	DIFFICULTY_NORMAL	= 1,
	DIFFICULTY_HARD		= 2,
};


enum
{
	PFSIZE_SMALL		= 0,
	PFSIZE_MEDIUM		= 1,
	PFSIZE_WIDE			= 2,
};

enum
{
	kDisplayMode_Windowed				= 0,
	kDisplayMode_FullscreenStretched	= 1,
	kDisplayMode_FullscreenCrisp		= 2,
	kDisplayMode_COUNT
};

enum
{
	kScaling_Unspecified	= -1,
	kScaling_PixelPerfect	= 0,
	kScaling_Stretch		= 1,
	kScaling_HQStretch		= 2,
};
