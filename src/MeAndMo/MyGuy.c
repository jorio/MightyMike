/****************************/
/*    	MYGUY               */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/****************************/
/*    EXTERNALS             */
/****************************/
#include "myglobals.h"
#include "objecttypes.h"
#include "windows.h"
#include "myguy.h"
#include "myguy2.h"
#include "playfield.h"
#include "object.h"
#include "bonus.h"
#include "infobar.h"
#include "triggers.h"
#include "miscanims.h"
#include "store.h"
#include "misc.h"
#include "sound2.h"
#include "weapon.h"
#include "shape.h"
#include "io.h"
#include "collision.h"
#include "input.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr,*gShieldNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY;
extern	union_gX;
extern	union_gY;
extern	CollisionRec	gCollisionList[];
extern	short			gNumCollisions;
extern	long				gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	Boolean			gShootButtonDownFlag,gTeleportingFlag,gFrogFlag,gSpaceShipFlag;
extern	unsigned char	gInterlaceMode;
extern	Byte			gCurrentWeaponType;
extern	Boolean			gGlobalFlagList[MAX_GLOBAL_FLAGS];
extern	Byte			gDepartment,gSceneNum;
extern	Handle			gPlayfieldHandle;
extern	short			gMyHealth,gMyMaxHealth;
extern	short			gEnemyFreezeTimer;
extern	short			gShieldTimer;
extern	Boolean			gFinishedArea;

/****************************/
/*    CONSTANTS             */
/****************************/

#define	BLINKIE_DURATION	(GAME_FPS*2-1)			// amount of time to blink after I've been hit


#define	MY_FULL_COLLISION	CTYPE_BGROUND|CTYPE_ENEMYA|CTYPE_ENEMYB|	\
							CTYPE_BONUS|CTYPE_TRIGGER|CTYPE_MISC|		\
							CTYPE_ENEMYC

#define	NO_ENEMYA_COLLISION	CTYPE_BGROUND|CTYPE_ENEMYB|	\
							CTYPE_BONUS|CTYPE_TRIGGER|CTYPE_MISC|		\
							CTYPE_ENEMYC

#define	MY_SHIP_COLLISION	CTYPE_BGROUND|CTYPE_MISC|CTYPE_BONUS


#define	GUN_DRAW_DURATION	(GAME_FPS*2)			// amt of time gun remains drawn after shot

#define	WIND_BASE			0x8000L

#define	MY_INIT_ITEM_NUM	29						// map item # to init me

#define	SPEEDO_ICE_SPEED	0xe0000L;


/**********************/
/*     VARIABLES      */
/**********************/

#define		MeOnMPlatformFlag	Flag0

short		gMyInitX,gMyInitY;						// my init coords in map

long		gMyX,gMyY;
long		gMySumDX,gMySumDY,gMyDX,gMyDY;			// NOTE: these are only calculated during update (used by window scroll)
ObjNode		*gMyNodePtr;

short		gMyDirection;
Byte		gMyMode;

long		gMyNormalMaxSpeed;						// normal speed for me
long		gMyMaxSpeedX,gMyMaxSpeedY;				// current speed after friction

long		gMyAcceleration;						// current acceleration value
Boolean		gMeOnIceFlag;							// set if Im on slippery

long		gMyAccX,gMyAccY;						// current acceleration value
long		gMyWindDX,gMyWindDY;					// force of wind
short		gMyGunDrawnTimer;
Boolean		gMyKeys[6] = {false,false,false,false,false,false};

static	Byte		gMyWalkAnims[8] = {MY_ANIMBASE_WALK,
							MY_ANIMBASE_WALK+1,
							MY_ANIMBASE_WALK+1,
							MY_ANIMBASE_WALK+1,
							MY_ANIMBASE_WALK+2,
							MY_ANIMBASE_WALK+3,
							MY_ANIMBASE_WALK+3,
							MY_ANIMBASE_WALK+3};

static	Byte		gMyShootWalkAnims[8] = {MY_ANIMBASE_SHOOTWALK,
							MY_ANIMBASE_SHOOTWALK+1,
							MY_ANIMBASE_SHOOTWALK+2,
							MY_ANIMBASE_SHOOTWALK+3,
							MY_ANIMBASE_SHOOTWALK+4,
							MY_ANIMBASE_SHOOTWALK+5,
							MY_ANIMBASE_SHOOTWALK+6,
							MY_ANIMBASE_SHOOTWALK+7};

static	Byte		gMyThrowWalkAnims[8] = {MY_ANIMBASE_THROWWALK,
							MY_ANIMBASE_THROWWALK+1,
							MY_ANIMBASE_THROWWALK+2,
							MY_ANIMBASE_THROWWALK+3,
							MY_ANIMBASE_THROWWALK+4,
							MY_ANIMBASE_THROWWALK+5,
							MY_ANIMBASE_THROWWALK+6,
							MY_ANIMBASE_THROWWALK+7};


static	Byte		gMyStandAnims[8] = {MY_ANIMBASE_STAND,
							MY_ANIMBASE_STAND+1,
							MY_ANIMBASE_STAND+1,
							MY_ANIMBASE_STAND+1,
							MY_ANIMBASE_STAND+2,
							MY_ANIMBASE_STAND+3,
							MY_ANIMBASE_STAND+3,
							MY_ANIMBASE_STAND+3};

static	Byte		gMyShootStandAnims[8] = {MY_ANIMBASE_SHOOTSTAND,
							MY_ANIMBASE_SHOOTSTAND+1,
							MY_ANIMBASE_SHOOTSTAND+2,
							MY_ANIMBASE_SHOOTSTAND+3,
							MY_ANIMBASE_SHOOTSTAND+4,
							MY_ANIMBASE_SHOOTSTAND+5,
							MY_ANIMBASE_SHOOTSTAND+6,
							MY_ANIMBASE_SHOOTSTAND+7};

static	Byte		gMyThrowStandAnims[8] = {MY_ANIMBASE_THROWSTAND,
							MY_ANIMBASE_THROWSTAND+1,
							MY_ANIMBASE_THROWSTAND+2,
							MY_ANIMBASE_THROWSTAND+3,
							MY_ANIMBASE_THROWSTAND+4,
							MY_ANIMBASE_THROWSTAND+5,
							MY_ANIMBASE_THROWSTAND+6,
							MY_ANIMBASE_THROWSTAND+7};


static	Byte		gMyHurtAnims[8] = {MY_ANIMBASE_HURT,
							MY_ANIMBASE_HURT+1,
							MY_ANIMBASE_HURT+1,
							MY_ANIMBASE_HURT+1,
							MY_ANIMBASE_HURT+2,
							MY_ANIMBASE_HURT+3,
							MY_ANIMBASE_HURT+3,
							MY_ANIMBASE_HURT+3};


static	Byte		gMyWaterHurtAnims[8] = {MY_ANIMBASE_WATERHURT,
							MY_ANIMBASE_WATERHURT+1,
							MY_ANIMBASE_WATERHURT+1,
							MY_ANIMBASE_WATERHURT+1,
							MY_ANIMBASE_WATERHURT+2,
							MY_ANIMBASE_WATERHURT+3,
							MY_ANIMBASE_WATERHURT+3,
							MY_ANIMBASE_WATERHURT+3};

static	Byte		gMySwimAnims[8] = {MY_ANIMBASE_SWIM,
							MY_ANIMBASE_SWIM+1,
							MY_ANIMBASE_SWIM+1,
							MY_ANIMBASE_SWIM+1,
							MY_ANIMBASE_SWIM+2,
							MY_ANIMBASE_SWIM+3,
							MY_ANIMBASE_SWIM+3,
							MY_ANIMBASE_SWIM+3};

short			gMyBlinkieTimer;


static	LongPoint	gWindVectorTable[8] = {-WIND_BASE,0,				// Y,X	(NOT X,Y!!!!!!)
									-WIND_BASE,WIND_BASE,
									0,WIND_BASE,
									WIND_BASE,WIND_BASE,
									WIND_BASE,0,
									WIND_BASE,-WIND_BASE,
									0,-WIND_BASE,
									-WIND_BASE,-WIND_BASE};

Boolean		gMeOnWaterFlag;
short		gLastNonDeathX,gLastNonDeathY;

short		gSpeedyTimer;					// time remaining on speedy shoes

Boolean		gButtonWasDownFlag = false;


/************************ INIT ME ********************/

void InitMe(void)
{
	FindMyInitCoords();								// see where to put me

	gMyDirection = 4;
	gMyBlinkieTimer = 0;
	gTeleportingFlag = false;
	gMeOnIceFlag = false;
	gMeOnWaterFlag = false;
	gFrogFlag = false;
	gShieldTimer = 0;
	gSpeedyTimer = 0;

				/* GET NEW OBJECT FOR ME */

	gMyX = gMyInitX;
	gMyY = gMyInitY;

	gMyNodePtr = MakeNewShape(GroupNum_MyGuy,ObjType_MyGuy,gMyStandAnims[gMyDirection],
					gMyX,gMyY,50,MoveMe,PLAYFIELD_RELATIVE);

	if (gMyNodePtr == nil)
		DoFatalAlert("Couldnt init Me!");

	gMyNodePtr->CType = CTYPE_MYGUY;
	gMyNodePtr->CBits = CBITS_TOUCHABLE;

	gMyNodePtr->TopOff = -17;					// set box
	gMyNodePtr->BottomOff = 2;
	gMyNodePtr->LeftOff = -14;
	gMyNodePtr->RightOff = 15;

	gMyNodePtr->MeOnMPlatformFlag = false;		// not on mplatform

	gMyNormalMaxSpeed = MY_WALK_SPEED;
	gMyAcceleration = MY_NORMAL_ACCELERATION;

	gMyMode = MY_MODE_BASICSTAND;				// set my MODE

}


/********************* MOVE ME *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//


void MoveMe(void)
{
static	void(*myMoveTable[])(void) =
				{
					MoveMe_Walking,				// walk up
					MoveMe_Walking,				// walk right
					MoveMe_Walking,				// walk down
					MoveMe_Walking,				// walk left
					MoveMe_Standing,			// standing up
					MoveMe_Standing,			// standing down
					MoveMe_Standing,			// standing left
					MoveMe_Standing,			// standing right
					MoveMe_Hurt,				// hurt up
					MoveMe_Hurt,				// hurt down
					MoveMe_Hurt,				// hurt left
					MoveMe_Hurt,				// hurt right
					MoveMe_Die,					// dying
					MoveMe_Liftoff,				// liftoff
					nil,						// (rocket flames)
					MoveMe_ShootWalking,		// shoot walk up
					MoveMe_ShootWalking,		// shoot walk up/rt
					MoveMe_ShootWalking,		// shoot walk rt
					MoveMe_ShootWalking,		// shoot walk rt/d
					MoveMe_ShootWalking,		// shoot walk d
					MoveMe_ShootWalking,		// shoot walk d/l
					MoveMe_ShootWalking,		// shoot walk l
					MoveMe_ShootWalking,		// shoot walk u/l
					MoveMe_ShootStanding,		// shoot standing up
					MoveMe_ShootStanding,		// shoot standing up/rt
					MoveMe_ShootStanding,		// shoot standing rt
					MoveMe_ShootStanding,		// shoot standing rt/d
					MoveMe_ShootStanding,		// shoot standing d
					MoveMe_ShootStanding,		// shoot standing d/l
					MoveMe_ShootStanding,		// shoot standing l
					MoveMe_ShootStanding,		// shoot standing u/l
					MoveMe_Walking,				// throw walk up
					MoveMe_Walking,				// throw walk up/rt
					MoveMe_Walking,				// throw walk rt
					MoveMe_Walking,				// throw walk rt/d
					MoveMe_Walking,				// throw walk d
					MoveMe_Walking,				// throw walk d/l
					MoveMe_Walking,				// throw walk l
					MoveMe_Walking,				// throw walk u/l
					MoveMe_Standing,			// throw standing up
					MoveMe_Standing,			// throw standing up/rt
					MoveMe_Standing,			// throw standing rt
					MoveMe_Standing,			// throw standing rt/d
					MoveMe_Standing,			// throw standing d
					MoveMe_Standing,			// throw standing d/l
					MoveMe_Standing,			// throw standing l
					MoveMe_Standing,			// throw standing u/l
					MoveMe_Swimming,			// swimming  u
					MoveMe_Swimming,			// swimming  r
					MoveMe_Swimming,			// swimming  d
					MoveMe_Swimming,			// swimming  l
					MoveMe_Die,					// water die
					MoveMe_Hurt,				// water hurt up
					MoveMe_Hurt,				// water hurt down
					MoveMe_Hurt,				// water hurt left
					MoveMe_Hurt					// water hurt right
				};

	if (gSpeedyTimer)									// see if speedy shoes
	{
		if (--gSpeedyTimer == 0)
		{
			gMyNormalMaxSpeed = MY_WALK_SPEED;			// reset normal speed
		}
		else
		{
			MakeFeetSmoke();
		}
	}

	if (gEnemyFreezeTimer)								// update enemy freeze timer
		gEnemyFreezeTimer--;

	if (gTeleportingFlag)								// lock me while teleporting
	{
		CalcObjectBox2(gThisNodePtr);
		DoMyScreenScroll();
		return;
	}

	GetObjectInfo();
	gMyMaxSpeedX = gMyMaxSpeedY = gMyNormalMaxSpeed;	// assume normal speed
	gMyAcceleration = MY_NORMAL_ACCELERATION;			// assume normal acceleration
	gMyWindDX = gMyWindDY = 0;							// assume no wind

	myMoveTable[gThisNodePtr->SubType]();

//	PrintNum(Absolute(gThisNodePtr->X.Int - gThisNodePtr->OldX),	//-------
//			 3, 100,440);
//	if (Absolute(gThisNodePtr->X.Int - gThisNodePtr->OldX) > 15)	//------------
//	{
//		SysBeep(0);
//	}
}


/*************** MOVE ME:  WALKING ****************/

void MoveMe_Walking(void)
{
	gMyMode = MY_MODE_BASICWALK;

	HandleMyCenter();									// check any center spot stuff
	if (CalcMyAim())									// calc my aiming vector
		SetMyWalkAnim();

	DoMyMove();											// do the move

	if (DoMyCollisionDetect())							// handle collision
		goto update;

	if (!(gDX|gDY))										// see if stopped
		SetMyStandAnim();

	CheckFireWeapon();									// handle shooting

update:
	UpdateMe();
}


/*************** MOVE ME:  SWIMMING ****************/

void MoveMe_Swimming(void)
{
	gMyMode = MY_MODE_SWIMMING;

	HandleMyCenter();									// check any center spot stuff
	if (CalcMyAim())									// calc my aiming vector
		SetMySwimAnim();

	DoMyMove();											// do the move

	if (DoMyCollisionDetect())							// handle collision
		goto update;

	if (!gMeOnWaterFlag)								// see if exited water
		SetMyWalkAnim();								// go back to walking

update:
	UpdateMe();
}


/****************** MOVE ME: SHOOT WALKING *************/

void MoveMe_ShootWalking(void)
{
	gMyMode = MY_MODE_SHOOTWALK;

	HandleMyCenter();									// check any center spot stuff
	if (CalcMyAim())									// calc my aiming vector
		SetMyWalkAnim();

	DoMyMove();											// do the move

	if (DoMyCollisionDetect())							// handle collision
		goto update;

	if (!(gDX|gDY))										// see if stopped
		SetMyStandAnim();

	if (--gMyGunDrawnTimer < 0)							// see if put gun away
	{
		gMyMode = MY_MODE_BASICWALK;					// normal walk
		SetMyWalkAnim();								// reset anim
	}

	CheckFireWeapon();									// handle shooting

update:
	UpdateMe();
}



/*************** MOVE ME:  STANDING ****************/

void MoveMe_Standing(void)
{
	gMyMode = MY_MODE_BASICSTAND;						// assume this

	HandleMyCenter();									// check any center spot stuff
	if (CalcMyAim())									// calc my aiming vector
		SetMyStandAnim();

	DoMyMove();

	if (DoMyCollisionDetect())							// handle collision
		goto update;

	if (gDX | gDY)										// see if unwalk
		SetMyWalkAnim();

	CheckFireWeapon();									// handle shooting

update:
	UpdateMe();
}

/*************** MOVE ME:  SHOOT STANDING ****************/

void MoveMe_ShootStanding(void)
{
	gMyMode = MY_MODE_SHOOTSTAND;						// assume this

	HandleMyCenter();									// check any center spot stuff
	if (CalcMyAim())									// calc my aiming vector
		SetMyStandAnim();

	DoMyMove();

	if (DoMyCollisionDetect())							// handle collision
		goto update;

	if (gDX | gDY)										// see if unwalk
		SetMyWalkAnim();

	if (--gMyGunDrawnTimer < 0)							// see if put gun away
	{
		gMyMode = MY_MODE_BASICSTAND;					// normal walk
		SetMyStandAnim();								// reset anim
	}

	CheckFireWeapon();									// handle shooting

update:
	UpdateMe();
}


/*************** MOVE ME:  HURT ****************/

void MoveMe_Hurt(void)
{
	gMyMode = MY_MODE_HURT;						// assume this

	gDX = gDY = 0;
	DoMyMove();
	UpdateMe_NoBlinkie();
}


/*************** MOVE ME:  DIE ****************/

void MoveMe_Die(void)
{
	gDX = gDY = 0;
	DoMyMove();
	UpdateMe_NoBlinkie();
}


/*************** MOVE ME:  LIFTOFF ****************/

void MoveMe_Liftoff(void)
{
ObjNode	*newObj;

	gMyNodePtr->DrawFlag = true;

	if (gThisNodePtr->Flag0)
	{
		if (gThisNodePtr->Flag0 == true)
		{
			gThisNodePtr->Flag0++;							// set flag to 2

						/* ATTACK FLAME */

			newObj = MakeNewShape(GroupNum_MyGuy,ObjType_MyGuy,MY_ANIMBASE_FLAME,
								gMyNodePtr->X.Int,gMyNodePtr->Y.Int,
								gMyNodePtr->Z-1,MoveMyFlame,PLAYFIELD_RELATIVE);

						/* PUT SHADOW UNDER ME */

			MakeShadow(gMyNodePtr,SHADOWSIZE_MEDIUM);
		}

					/* MOVE MIKE */

		gThisNodePtr->YOffset.L -= 0x30000L;				// move it up

		if (gThisNodePtr->YOffset.Int < -340)				// see if off screen
			gFinishedArea = true;							// end level
	}
}


/******************* DO MY MOVE ********************/

void DoMyMove(void)
{
	gSumDX = gDX+gMyWindDX;								// calc sum deltas
	gSumDY = gDY+gMyWindDY;

	CheckIfMeOnMPlatform();								// add any moving platform deltas

	gX.L += gSumDX;										// do the move
	gY.L += gSumDY;
}


/************* MOVE MY CONTROLLER *****************/
//
// Read controller (keyboard) and set my deltas accordingly
//

void MoveMyController(void)
{
Boolean	horizFlag,vertFlag;

	horizFlag = vertFlag = false;						// assume no control

				/* CALL CORRECT CONTROLLER ROUTINE */

	ControlMeByKeyboard(&horizFlag,&vertFlag);


				/* DO FRICTION IN DIRECTION OF NON MOVEMENT */

	if (gMeOnWaterFlag)								// WATER FRICTION
	{
		if (!vertFlag)
		{
			gDY = gDY*WATER_FRICTION;
			if (Absolute(gDY) < 0x10000L)
				gDY = 0;
		}
		if (!horizFlag)
		{
			gDX = gDX*WATER_FRICTION;
			if (Absolute(gDX) < 0x10000L)
				gDX = 0;
		}
	}
	else
	if (gMeOnIceFlag)									// ICE FRICTION
	{
		if (!vertFlag)
		{
			gDY = gDY*ICE_FRICTION;
			if (Absolute(gDY) < 0x10000L)
				gDY = 0;
		}
		if (!horizFlag)
		{
			gDX = gDX*ICE_FRICTION;
			if (Absolute(gDX) < 0x10000L)
				gDX = 0;
		}
	}
	else												// GROUND FRICTION
	{
		if (!vertFlag)
		{
			gDY = gDY*GROUND_FRICTION;
			if (Absolute(gDY) < 0x10000L)
				gDY = 0;
		}
		if (!horizFlag)
		{
			gDX = gDX*GROUND_FRICTION;
			if (Absolute(gDX) < 0x10000L)
				gDX = 0;
		}
	}
					/* CHECK DELTAS */

	PinMyDeltas();
}

/******************* CONTROL ME BY KEYBOARD *************************/

void ControlMeByKeyboard(Boolean *horizFlag, Boolean *vertFlag)
{
	if (GetKeyState(kKey_Forward))
	{
		gDY -= gMyAcceleration;
		*vertFlag = true;
	}
	else
	if (GetKeyState(kKey_Backward))
	{
		gDY += gMyAcceleration;
		*vertFlag = true;
	}

	if (GetKeyState(kKey_Left))
	{
		gDX -= gMyAcceleration;
		*horizFlag = true;
	}
	else
	if (GetKeyState(kKey_Right))
	{
		gDX += gMyAcceleration;
		*horizFlag = true;
	}

#if 0
	else
	if (GetKeyState(KEY_K9) || GetKeyState(KEY_O))
	{
		gDX += gMyAcceleration;
		gDY -= gMyAcceleration;
		*horizFlag = true;
		*vertFlag = true;
	}
	else
	if (GetKeyState(KEY_K3) || GetKeyState(KEY_PERIOD))
	{
		gDX += gMyAcceleration;
		gDY += gMyAcceleration;
		*horizFlag = true;
		*vertFlag = true;
	}
	else
	if (GetKeyState(KEY_K1) || GetKeyState(KEY_M))
	{
		gDX -= gMyAcceleration;
		gDY += gMyAcceleration;
		*horizFlag = true;
		*vertFlag = true;
	}
	else
	if (GetKeyState(KEY_K7) || GetKeyState(KEY_U))
	{
		gDX -= gMyAcceleration;
		gDY -= gMyAcceleration;
		*horizFlag = true;
		*vertFlag = true;
	}
#endif
}



/********************* PIN MY DELTAS ************************/
//
// Pin velocity to max allowed speed
//

void PinMyDeltas(void)
{
	if (gMeOnWaterFlag)					// WATER PIN
	{
		if (gDX > (MY_WALK_SPEED/2))
			gDX = MY_WALK_SPEED/2;
		else
		if (gDX < (-(MY_WALK_SPEED/2)))
			gDX = -(MY_WALK_SPEED/2);

		if (gDY > (MY_WALK_SPEED/2))
			gDY = MY_WALK_SPEED/2;
		else
		if (gDY < (-(MY_WALK_SPEED/2)))
			gDY = -(MY_WALK_SPEED/2);
	}
	else								// NORMAL PIN
	{
		if (gDX > gMyMaxSpeedX)
			gDX = gMyMaxSpeedX;
		else
		if (gDX < -gMyMaxSpeedX)
			gDX = -gMyMaxSpeedX;

		if (gDY > gMyMaxSpeedY)
			gDY = gMyMaxSpeedY;
		else
		if (gDY < -gMyMaxSpeedY)
			gDY = -gMyMaxSpeedY;
	}
}


/***************** DO MY COLLISION DETECT ********************/
//
// returns true if I got hurt or killed during collision
//

Boolean DoMyCollisionDetect(void)
{
register	short	i;
register	short	originalX,originalY,offset;
register	Boolean		hurtFlag;
Boolean		killFlag;
unsigned short	attrib;
short		maxYOffset,maxXOffset;

	hurtFlag = killFlag = false;							// assume not hurt

	CalcObjectBox();										// calc newest box


			/* SET COLLISION ATTRIBUTES & CHECK COLLIDE */

	if (gMyMode == MY_MODE_SPACESHIP)						// special if spaceship
	{
		CollisionDetect(gMyNodePtr,MY_SHIP_COLLISION);
	}
	else
	{
		if (!gEnemyFreezeTimer)
			CollisionDetect(gMyNodePtr,MY_FULL_COLLISION);		// do full collision
		else
			CollisionDetect(gMyNodePtr,NO_ENEMYA_COLLISION);	// do no enemy A
	}

	originalX = gX.Int;										// remember starting coords
	originalY = gY.Int;
	maxYOffset = maxXOffset = 0;

	for (i=0; i < gNumCollisions; i++)						// handle all collisions
	{
		if (gTeleportingFlag)								// if previous collision caused teleport, then cancel all other collisions
			break;

		if (gCollisionList[i].type == COLLISION_TYPE_TILE)
		{
					/**************************/
					/* HANDLE TILE COLLISIONS */
					/**************************/

			if (gCollisionList[i].sides & SIDE_BITS_TOP)	// SEE IF HIT TOP
			{
				offset = TILE_SIZE-(gTopSide&0x1f);			// see how far over it went
				if (offset > maxYOffset)					// see if worst one
					gY.Int = originalY+offset;				// adjust y coord
				maxYOffset = offset;
				gDY = 0;									// stop my dy
			}
			else
			if (gCollisionList[i].sides & SIDE_BITS_BOTTOM)	// SEE IF HIT BOTTOM
			{
				offset = (gBottomSide&0x1f)+1;				// see how far over it went
				if (offset > maxYOffset)					// see if worst one
					gY.Int = originalY-offset;				// adjust y coord
				maxYOffset = offset;
				gDY = 0;								// stop my dy
			}


			if (gCollisionList[i].sides & SIDE_BITS_LEFT)	// SEE IF HIT LEFT
			{
				offset = TILE_SIZE-(gLeftSide&0x1f);		// see how far over it went
				if (offset > maxXOffset)					// see if worst one
				{
					gX.Int = originalX+offset;				// adjust x coord
				}
				maxXOffset = offset;
				gDX = 0;									// stop my dx
			}
			else
			if (gCollisionList[i].sides & SIDE_BITS_RIGHT)	// SEE IF HIT RIGHT
			{
				offset = (gRightSide&0x1f)+1;				// see how far over it went
				if (offset > maxXOffset)					// see if worst one
				{
					gX.Int = originalX-offset;				// adjust x coord
				}
				maxXOffset = offset;
				gDX = 0;									// stop my dx
			}
		}
		else
		if (gCollisionList[i].type == COLLISION_TYPE_OBJ)
		{
					/****************************/
					/* HANDLE OBJECT COLLISIONS */
					/****************************/


							/* SEE IF HIT TRIGGER */

			if (gCollisionList[i].objectPtr->CType & CTYPE_TRIGGER)
			{
				if (!HandleTrigger(gCollisionList[i].objectPtr,gCollisionList[i].sides))	// if returns false, then ignore solids
					goto ignore_solid;
			}


						/* HANDLE COORDINATE ADJUSTMENT */

			if (gCollisionList[i].sides & SIDE_BITS_TOP)	// SEE IF HIT TOP
			{
				offset = (gCollisionList[i].objectPtr->BottomSide-gTopSide)+1;	// see how far over it went
				if (offset > maxYOffset)					// see if worst one
					gY.Int = originalY+offset;				// adjust y coord
				maxYOffset = offset;
				gDY = 0;									// stop my dy
			}
			else
			if (gCollisionList[i].sides & SIDE_BITS_BOTTOM)	// SEE IF HIT BOTTOM
			{
				offset = (gBottomSide-gCollisionList[i].objectPtr->TopSide)+1;	// see how far over it went
				if (offset > maxYOffset)					// see if worst one
					gY.Int = originalY-offset;				// adjust y coord
				maxYOffset = offset;
				gDY = 0;									// stop my dy
			}


			if (gCollisionList[i].sides & SIDE_BITS_LEFT)	// SEE IF HIT LEFT
			{
				offset = (gCollisionList[i].objectPtr->RightSide-gLeftSide)+1;	// see how far over it went
				if (offset > maxXOffset)					// see if worst one
					gX.Int = originalX+offset;				// adjust x coord
				maxXOffset = offset;
				gDX = 0;									// stop my dx
			}
			else
			if (gCollisionList[i].sides & SIDE_BITS_RIGHT)	// SEE IF HIT RIGHT
			{
				offset = (gRightSide-gCollisionList[i].objectPtr->LeftSide)+1;	// see how far over it went
				if (offset > maxXOffset)					// see if worst one
					gX.Int = originalX-offset;				// adjust x coord
				maxXOffset = offset;
				gDX = 0;									// stop my dx
			}
ignore_solid:

							/* SEE IF HIT BONUS OBJECT */

			if (gCollisionList[i].objectPtr->CType & CTYPE_BONUS)
				MeHitBonusObject(gCollisionList[i].objectPtr);

							/* SEE IF HIT ENEMY OBJECT */
			else
			if (gCollisionList[i].objectPtr->CType & (CTYPE_ENEMYA|CTYPE_ENEMYB|CTYPE_ENEMYC))
			{
				if ((gMyBlinkieTimer+gShieldTimer) <= 0)		// check if I'm in blinkie invincible mode
				{												// 	or shield mode
					MeHitEnemyObject(gCollisionList[i].objectPtr);
					hurtFlag = true;
				}
			}
		}
	}

				/* CHECK SPECIAL TILE ATTRIBUTES FOR HURT/DEATH */

	if (!gMyNodePtr->MeOnMPlatformFlag)						// dont check if on mplatform
	{
		attrib = GetMapTileAttribs(gX.Int,gY.Int);			// get tile attrib

		if (attrib & TILE_ATTRIB_DEATH)						// see if on death & Im vulnerable
		{
			if (!gMyBlinkieTimer)
			{
				if (gMeOnWaterFlag)
					SwitchAnim(gMyNodePtr,MY_ANIMBASE_WATERDIE); // do water death animation
				else
					SwitchAnim(gMyNodePtr,MY_ANIMBASE_DIE);		// do death animation
				PlaySound(SOUND_DEATHSCREAM);
				killFlag = true;
			}
		}
		else
		{
			gLastNonDeathX = gX.Int;						// remember non-death spot
			gLastNonDeathY = gY.Int;

			if (attrib & TILE_ATTRIB_HURT)					// see if only hurt
				hurtFlag = true;
		}
	}

	if (hurtFlag)											// see if need to effect hurt
		IGotHurt();


	return(hurtFlag|killFlag);
}

/***************** HANDLE MY CENTER ******************/
//
// Does special stuff for the tile Im standing on
//

void HandleMyCenter(void)
{
register TileAttribType *tileAttribs;
register unsigned short	bits;

	if (gMyNodePtr->MeOnMPlatformFlag)					// ignore center info if on mplatform
	{
		gMeOnIceFlag = false;
		gMeOnWaterFlag = false;
		return;
	}


	tileAttribs = GetFullMapTileAttribs(gX.Int,gY.Int);		// get attribs of center tile
	bits = tileAttribs->bits;

					/* SEE IF ON STAIRS */

	if (bits & TILE_ATTRIB_STAIRS)
	{
		gMyMaxSpeedY >>= 1;									// slow down on stairs
	}

				/* SEE IF FRICTION */

	if (bits & TILE_ATTRIB_FRICTION)
	{
		gMyMaxSpeedY >>= 1;									// slow down on stairs
		gMyMaxSpeedX >>= 1;
	}

					/* SEE IF ON WIND */

	if (bits & TILE_ATTRIB_WIND)
	{
		gMyWindDX = gWindVectorTable[tileAttribs->parm0].h*tileAttribs->parm1;	// parm 0 = direction, parm 1 = force
		gMyWindDY = gWindVectorTable[tileAttribs->parm0].v*tileAttribs->parm1;
	}

					/* SEE IF ON ICE */

	if (bits & TILE_ATTRIB_ICE)
	{
		gMeOnIceFlag = true;
		gMyAcceleration /= 5;								// now ive got poor acceleration
		if (gMyMaxSpeedX < MAX_ICE_SPEED)					// I can go really fast on ice!
			gMyMaxSpeedX = gMyMaxSpeedY = MAX_ICE_SPEED;
	}
	else
		gMeOnIceFlag = false;


					/* SEE IF ON WATER */

	if (bits & TILE_ATTRIB_WATER)
	{
		gMeOnWaterFlag = true;

		if ((gMyNodePtr->SubType < MY_ANIMBASE_SWIM) ||		// see if already swimming
			(gMyNodePtr->SubType > (MY_ANIMBASE_SWIM+3)))
		{
			SetMySwimAnim();								// enter the water
			MakeSplash(gX.Int,gY.Int,gMyNodePtr->Z);
		}
	}
	else
	{
		gMeOnWaterFlag = false;
	}

}


/***************** UPDATE ME *****************/

void UpdateMe(void)
{
	if (gMyBlinkieTimer > 0)							// check blinkie
	{
		if (--gMyBlinkieTimer & b1)
			gMyNodePtr->DrawFlag = false;
		else
			gMyNodePtr->DrawFlag = true;
	}

	gMyX = gX.Int;
	gMyY = gY.Int;

	gMySumDX = gSumDX;
	gMySumDY = gSumDY;
	gMyDX = gDX;
	gMyDY = gDY;

	DoMyScreenScroll();

	CalcObjectBox();
	UpdateObject();
}

/***************** UPDATE ME: NO BLINKIE *****************/
//
// Updates me without checking blinkie
//

void UpdateMe_NoBlinkie(void)
{
	gMyNodePtr->DrawFlag = true;

	gMyX = gX.Int;
	gMyY = gY.Int;

	gMySumDX = gSumDX;
	gMySumDY = gSumDY;
	gMyDX = gDX;
	gMyDY = gDY;

	DoMyScreenScroll();

	CalcObjectBox();
	UpdateObject();
}


/****************** CALC MY AIM ****************/
//
// Find direction values for walking and shooting
//
// OUTPUT: flag set if need to set new animation #
//

Boolean CalcMyAim(void)
{
static	Byte	oldDirection;

	MoveMyController();									// handle user control & friction


				/* FIND CURRENT AIM */

	if (gDY < 0)
	{
		if (gDX < 0)
			gMyDirection = AIM_UP_LEFT;
		else
		if (gDX == 0)
			gMyDirection = AIM_UP;
		else
		if (gDX > 0)
			gMyDirection = AIM_UP_RIGHT;
	}
	else
	if (gDY > 0)
	{
		if (gDX < 0)
			gMyDirection = AIM_DOWN_LEFT;
		else
		if (gDX == 0)
			gMyDirection = AIM_DOWN;
		else
		if (gDX > 0)
			gMyDirection = AIM_DOWN_RIGHT;
	}
	else									// no vertical movement
	{
		if (gDX < 0)
			gMyDirection = AIM_LEFT;
		else
		if (gDX > 0)
			gMyDirection = AIM_RIGHT;
	}

				/**********************************/
				/* SEE IF CHANGED SINCE LAST TIME */
				/**********************************/

	if (oldDirection != gMyDirection)
	{
		oldDirection = gMyDirection;
		return(true);
	}
	else
		return(false);

}


/***************** SET MY WALK ANIM ****************/
//
// Sets me to the appropriate walking animation
//

void SetMyWalkAnim(void)
{
	switch(gMyMode)
	{
		case	MY_MODE_BASICSTAND:
		case	MY_MODE_BASICWALK:
				if (gMyNodePtr->SubType != gMyWalkAnims[gMyDirection])		// see if already correct
					SwitchAnim(gMyNodePtr,gMyWalkAnims[gMyDirection]);		// set to basic walking
				break;

		case	MY_MODE_SHOOTWALK:
		case	MY_MODE_SHOOTSTAND:
				if (gMyNodePtr->SubType != gMyShootWalkAnims[gMyDirection])	// see if already correct
					SwitchAnim(gMyNodePtr,gMyShootWalkAnims[gMyDirection]);	// set to shoot walking
				break;

		default:
				SwitchAnim(gMyNodePtr,gMyWalkAnims[gMyDirection]);			// set to basic walking
	}
}



/***************** SET MY STAND ANIM ****************/
//
// Sets me to the appropriate standing animation
//

void SetMyStandAnim(void)
{
	switch(gMyMode)
	{
		case	MY_MODE_BASICSTAND:
		case	MY_MODE_BASICWALK:
				if (gMyNodePtr->SubType != gMyStandAnims[gMyDirection])		// see if already correct
					SwitchAnim(gMyNodePtr,gMyStandAnims[gMyDirection]);		// set to basic standing
				break;

		case	MY_MODE_SHOOTWALK:
		case	MY_MODE_SHOOTSTAND:
				if (gMyNodePtr->SubType != gMyShootStandAnims[gMyDirection])	// see if already correct
					SwitchAnim(gMyNodePtr,gMyShootStandAnims[gMyDirection]);	// set to shoot standing
				break;

		default:
				SwitchAnim(gMyNodePtr,gMyStandAnims[gMyDirection]);			// set to basic standing
	}
}

/***************** SET MY SWIM ANIM ****************/
//
// Sets me to the appropriate swim animation
//

void SetMySwimAnim(void)
{
	if (gMyNodePtr->SubType != gMySwimAnims[gMyDirection])				// see if already correct
		SwitchAnim(gMyNodePtr,gMySwimAnims[gMyDirection]);				// set
}


/*********************** ME HIT BONUS OBJECT **********************/

void MeHitBonusObject(ObjNode *targetNode)
{
						/* SEE IF HIT WEAPON POWERUP */

	if (targetNode->CType & CTYPE_WEAPONPOW)
	{
		GetAWeapon(targetNode->Kind);						// get the weapon
		targetNode->ItemIndex = nil;						// wont be comin back
		DeleteObject(targetNode);
		return;
	}

						/* SEE IF HIT MISC POWERUP */
	else
	if (targetNode->CType & CTYPE_MISCPOW)
	{
		GetMiscPOW(targetNode->Kind);						// get the POW
		targetNode->ItemIndex = nil;						// wont be comin back
		DeleteObject(targetNode);
		return;
	}

						/* SEE IF GOT A HEALTH POWERUP */

	else
	if (targetNode->CType & CTYPE_HEALTH)
	{
		if (gMyHealth < gMyMaxHealth)						// only get health if need it!
		{
			MakeMikeMessage(MESSAGE_NUM_FOOD);				// put message
			GiveMeHealth();
			targetNode->ItemIndex = nil;					// its never coming back
			DeleteObject(targetNode);
		}
		return;
	}

						/* SEE IF HIT COIN */

	else
	if ((targetNode->Type == ObjType_Coin) && (targetNode->SpriteGroupNum == GroupNum_Coin))
	{
		GetCoins(1);
		DeleteObject(targetNode);
		PlaySound(SOUND_COINS);
	}
						/* SEE IF HIT BUNNY */

	else
	if ((targetNode->Type == ObjType_Bunny) && (targetNode->SpriteGroupNum == GroupNum_Bunny))
	{
		DeleteBunny(targetNode);
		PlaySound(SOUND_SQUEEK);
		DecBunnyCount();
	}

						/* SEE IF HIT KEY */

	else
	if (targetNode->CType & CTYPE_KEY)
	{
		gMyKeys[targetNode->SubType] = true;			// get the key

		targetNode->ItemIndex = nil;				// wont be comin back
		DeleteObject(targetNode);
		ShowKeys();
		PlaySound(SOUND_GETKEY);
	}

					/* SEE IF HIT SPACESHIP */
	else
	if ((gSceneNum == SCENE_BARGAIN) && (targetNode->Type == ObjType_SpaceShip) &&
		(gSpaceShipFlag != true))
	{
		TurnMeIntoShip(targetNode);
	}
}


/*********************** ME HIT ENEMY OBJECT **********************/
//
// Handles stuff when I hit an enemy object.
// NOTE: It does not call IGotHurt, that is called later by the caller routine.
//

void MeHitEnemyObject(ObjNode *targetNode)
{
Boolean	delFlag;

			/* SPECIAL CHECK FOR WITCH-FROG */

	if (gSceneNum == SCENE_FAIRY)
	{
		if ((targetNode->Type == ObjType_Witch) && (targetNode->SpriteGroupNum == GroupNum_Witch))
		{
			if (gMyMode != MY_MODE_FROG)				// if already frog, then just let witch hurt me
			{
				if (gMyBlinkieTimer <= 0)				// dont frog me while blinking
				{
					TurnMeIntoFrog();
					gMyBlinkieTimer = BLINKIE_DURATION;		// make me blink (don't hurt me 1st time)
				}
			}
		}
	}

				/* SEE IF HIT ENEMY BULLET */

	if (targetNode->CType & CTYPE_ENEMYB)
	{
		delFlag = true;								// assume will delete the bullet
		switch(gSceneNum)							// see if do something special with the bullet
		{
			case	SCENE_CLOWN:
					delFlag = false;
					break;

			case	SCENE_FAIRY:
					if (targetNode->Type == ObjType_FairyHealth)	// don't delete poison apples
					{
						targetNode->ItemIndex = nil;				// make sure it won't come back
						targetNode->CType = 0;
						SwitchAnim(targetNode,2);					// make poison apple vaporize
						delFlag = false;
					}
					break;
		}

		if (delFlag)
			DeleteObject(targetNode);				// delete enemy's bullet
	}

}


/***************** I GOT HURT *****************/

void IGotHurt(void)
{
	if ((gMyBlinkieTimer+gShieldTimer) > 0)						// check if I'm in blinkie invincible mode
		return;													// 	or shield mode

			/* DECREASE SHIELD LIFE */

	gMyHealth--;											// lose health

	DisposeFrog();											// undo frog if needed
	DisposeSpaceShip();										// undo spaceship if needed

	if (gMyHealth >= 0)										// see if still alive
	{
		ShowHealth();

		if (gMeOnWaterFlag)										// see which anim to use
			SwitchAnim(gMyNodePtr,gMyWaterHurtAnims[gMyDirection]);
		else
			SwitchAnim(gMyNodePtr,gMyHurtAnims[gMyDirection]);
		gMyBlinkieTimer = BLINKIE_DURATION;						// make me blink (when done w/ hurt anim)
		gMyMode = MY_MODE_HURT;
		MakeMikeMessage(MESSAGE_NUM_OUCH);
	}
	else
	{
			/* IM DEAD */

		if (gMeOnWaterFlag)								// see which anim to use
			SwitchAnim(gMyNodePtr,MY_ANIMBASE_WATERDIE);
		else
			SwitchAnim(gMyNodePtr,MY_ANIMBASE_DIE);
		PlaySound(SOUND_DEATHSCREAM);
		if (gShieldTimer)								// get rid of shield (gets here if stepped on killer tile)
		{
			DeleteObject(gShieldNodePtr);
			gShieldTimer = 0;
		}
	}
}


/**************** REVIVE ME *******************/
//
// Bring me back from the dead (1 player mode)
//

void ReviveMe(void)
{
	gMyNodePtr->X.Int = gLastNonDeathX;					// move to previous non-death spot
	gMyNodePtr->Y.Int = gLastNonDeathY;

	gGlobFlag_MeDoneDead = false;
	SwitchAnim(gMyNodePtr,gMyStandAnims[gMyDirection]);
		gMyBlinkieTimer = BLINKIE_DURATION;			// make me blink

	gMyHealth = gMyMaxHealth;						// restore health
	ShowHealth();
	ShowLives();
}



/************* DRAW MY GUN ******************/
//
// When weapon is shot, I need to be in the correct shooting animation
//

void DrawMyGun(void)
{

				/* SET DRAWN ANIM */

	switch(gMyMode)
	{
		case	MY_MODE_SHOOTSTAND:							// here if already drawn
		case	MY_MODE_SHOOTWALK:
				break;

		case	MY_MODE_BASICSTAND:							// draw standing
				gMyMode = MY_MODE_SHOOTSTAND;
				SetMyStandAnim();
				break;

		case	MY_MODE_BASICWALK:							// draw walking
				gMyMode = MY_MODE_SHOOTWALK;
				SetMyWalkAnim();
				break;

		default:
				gMyMode = MY_MODE_SHOOTSTAND;				// draw stand by default
				SetMyStandAnim();
	}

			/* RESET THE DRAW TIMER */

	gMyGunDrawnTimer = GUN_DRAW_DURATION;					// keep resetting this
}


/************* START MY THROW ******************/
//
// When weapon is tossed, I need to be in the correct throwing animation
//

void StartMyThrow(void)
{
				/* SET THROW ANIM */

	switch(gMyMode)
	{
		case	MY_MODE_SHOOTWALK:
		case	MY_MODE_BASICWALK:							// draw walking
				SwitchAnim(gMyNodePtr,gMyThrowWalkAnims[gMyDirection]);
				break;
		default:
				SwitchAnim(gMyNodePtr,gMyThrowStandAnims[gMyDirection]);
	}
}



/******************* CHECK IF ME ON MOVING PLATFORM *********************/

void CheckIfMeOnMPlatform(void)
{
register	ObjNode		*thisNodePtr;

	gMyNodePtr->MeOnMPlatformFlag = false;				// assume not on mplatform

					/* SCAN FOR MPLATFORMS */

	thisNodePtr = FirstNodePtr;
	do
	{
		if (thisNodePtr->CType & CTYPE_MPLATFORM)
		{
			if ((gX.Int > thisNodePtr->LeftSide) && (gX.Int < thisNodePtr->RightSide) &&		// see if im on it
				(gY.Int > thisNodePtr->TopSide) && (gY.Int < thisNodePtr->BottomSide))
			{
				gSumDX += thisNodePtr->DX;
				gSumDY += thisNodePtr->DY;
				gMyNodePtr->MeOnMPlatformFlag = true;
				break;
			}
		}

		thisNodePtr = (ObjNode *)thisNodePtr->NextNode;		// next node
	}
	while (thisNodePtr != nil);
}


/********************* FIND MY INIT COORDS **************************/
//
// Scans ORIGINAL item list for my init item.
//

void FindMyInitCoords(void)
{
long	offset;
short		*intPtr,itemNum;
ObjectEntryType *itemPtr;
short		numItems,num;

	gMyInitX = 250;						// assume these coords
	gMyInitY = 300;


					/* GET BASIC INFO */

	offset = *(long *)(*gPlayfieldHandle+6);					// get offset to OBJECT_LIST
	intPtr = (short *)(*gPlayfieldHandle+offset);					// get pointer to OBJECT_LIST
	numItems = *intPtr++;										// get # items in file
	if (numItems == 0)
		return;
	itemPtr = (ObjectEntryType *)intPtr;						// point to items in file


				/* SCAN FOR ME */

	for (itemNum = 0; itemNum < numItems; itemNum++)
	{
		num = itemPtr[itemNum].type&ITEM_NUM;					// get item #
		if (num == MY_INIT_ITEM_NUM)							// see if its me
		{
			gMyInitX = itemPtr[itemNum].x;
			gMyInitY = itemPtr[itemNum].y;
			itemPtr[itemNum].type |= ITEM_IN_USE;				// set in-use flag
			break;
		}
	}
}


/**************** MAKE FEET SMOKE *********************/
//
// PUT SMOKE TRAIL ON FEET
//

void MakeFeetSmoke(void)
{
register ObjNode	*newObj;

	if ((Absolute(gMyDX) > MY_WALK_SPEED) || (Absolute(gMyDY) > MY_WALK_SPEED))
	{
		newObj = MakeNewShape(GroupNum_RocketGun,ObjType_RocketGun,8,
							gMyNodePtr->X.Int,gMyNodePtr->Y.Int-4,
							gMyNodePtr->Z,nil,PLAYFIELD_RELATIVE);
		if (newObj != nil)
		{
			newObj->AnimSpeed += MyRandomLong()&0xff;	// random anim speed
			newObj->YOffset.Int = 24;					// move down to feet
		}
	}
}


/**************** START MY LIFTOFF *********************/

void StartMyLiftoff(void)
{
	if (gMyNodePtr->SubType != MY_ANIMBASE_LIFTOFF)
	{
		SwitchAnim(gMyNodePtr,MY_ANIMBASE_LIFTOFF);			// make Me do liftoff
		gMyNodePtr->Flag0 = false;

		PlaySong(SONG_ID_WINLEVEL);							// play win song
	}
}


/****************** MOVE MY FLAME **********************/

void MoveMyFlame(void)
{
	gThisNodePtr->X = gMyNodePtr->X;
	gThisNodePtr->Y = gMyNodePtr->Y;
	gThisNodePtr->Z = gMyNodePtr->Z-1;
	gThisNodePtr->YOffset = gMyNodePtr->YOffset;
}





