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

#define KEY_A				0x00
#define KEY_B				0x0b
#define	KEY_C				0x08
#define KEY_D				0x02
#define KEY_E				0x0e
#define KEY_F				0x03
#define KEY_G				0x05
#define KEY_H				0x04
#define KEY_I				0x22
#define KEY_J				0x26
#define KEY_K				0x28
#define KEY_L				0x25
#define KEY_M				0x2e
#define KEY_N				0x2d
#define KEY_O				0x1f
#define	KEY_P				0x23
#define KEY_Q				0x0c
#define KEY_R				0x0f
#define KEY_S				0x01
#define KEY_T				0x11
#define KEY_U				0x20
#define KEY_V				0x09
#define KEY_W				0x0d
#define KEY_X				0x07
#define KEY_Y				0x10
#define KEY_Z				0x06

#define	KEY_1				0x12
#define KEY_2				0x13
#define KEY_3				0x14
#define KEY_4				0x15
#define KEY_5				0x17
#define KEY_6				0x16
#define KEY_7				0x1a
#define KEY_8				0x1c
#define KEY_9				0x19
#define KEY_0				0x1d

#define	KEY_K0				0x52
#define	KEY_K1				0x53
#define	KEY_K2				0x54
#define	KEY_K3				0x55
#define	KEY_K4				0x56
#define	KEY_K5				0x57
#define	KEY_K6				0x58
#define	KEY_K7				0x59
#define	KEY_K8				0x5b
#define	KEY_K9				0x5c

#define KEY_PERIOD			0x2f
#define	KEY_QMARK			0x2c
#define	KEY_COMMA			0X2b

#define KEY_TAB				0x30
#define KEY_ESC				0x35
#define	KEY_CAPSLOCK		0x39
#define KEY_APPLE			0x37
#define KEY_SPACE			0x31
#define KEY_OPTION			0x3a
#define	KEY_CTRL			0x3b
#define	KEY_UP				0x7e
#define	KEY_DOWN			0x7d
#define	KEY_LEFT			0x7b
#define	KEY_RIGHT			0x7c
#define	KEY_SHIFT			0x38
#define	KEY_DELETE			0x33
#define	KEY_RETURN			0x24
#define	KEY_MINUS			0x1b
#define	KEY_PLUS			0x18

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