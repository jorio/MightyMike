/****************************/
/*       EQUATES            */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/

#define		MAX_SHAPE_GROUPS	10
#define		MAX_OBJECTS			200
#define 	MAX_INVENTORY		100		// max # items I can have (not including dulicates)

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


							// BINARY EQUATES
							//===================================

#define	b1					0x1L
#define	b11					0x3L
#define	b111				0x7L
#define	b1111				0xfL
#define	b11111				0x1fL
#define	b111111				0x3fL
#define	b1111111			0x7fL
#define	b11111111			0xffL
#define	b111111111			0x1ffL
#define	b1111111111			0x3ffL
#define	b11111111111		0x7ffL
#define	b111111111111		0xfffL
#define	b1111111111111		0x1fffL
#define	b1111111111100000	0xffe0L

#define	b1100				0xcL
#define	b1110				0xeL
#define	b110				0x6L

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

							// KEYBOARD DEFINES
							//=================================

#define	CHAR_RETURN			0x0d				// ASCII codes
#define CHAR_UP				0x1e
#define CHAR_DOWN			0x1f
#define	CHAR_LEFT			0x1c
#define	CHAR_RIGHT			0x1d
#define	CHAR_DELETE			0x08


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
	DIFFICULTY_EASY,
	DIFFICULTY_NORMAL,
	DIFFICULTY_HARD
};