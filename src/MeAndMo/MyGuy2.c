/****************************/
/*    	MYGUY 2             */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/****************************/
/*    EXTERNALS             */
/****************************/
#include "myglobals.h"
#include "objecttypes.h"
#include "myguy.h"
#include "myguy2.h"
#include "object.h"
#include "misc.h"
#include "sound2.h"
#include "weapon.h"
#include "shape.h"
#include "io.h"
#include "playfield.h"
#include "input.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr,*gShieldNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY;
extern	union_gX;
extern	union_gY;
extern	CollisionRec	gCollisionList[];
extern	short			gNumCollisions;
extern	long				gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	Boolean			gShootButtonDownFlag,gTeleportingFlag;
extern	unsigned char	gInterlaceMode;
extern	ObjNode			*gMyNodePtr;
extern	long		gMyNormalMaxSpeed;
extern	long		gMyMaxSpeedX,gMyMaxSpeedY,gMyAcceleration;
extern	short			gEnemyFreezeTimer,gSoundNum_ExitShip;
extern	long		gMyWindDX,gMyWindDY;
extern	Byte		gMyMode;
extern	short		gMyDirection,gSoundNum_Ship,gSoundNum_Frog;



/****************************/
/*    CONSTANTS             */
/****************************/

#define	FROG_JUMP_SPEED		0x70000L;
#define	FROG_DURATION		(GAME_FPS*7)

#define	SPACESHIP_DURATION		(GAME_FPS*15)

#define	SPACESHIP_ACCELERATION	0x20000L
#define	SPACESHIP_SPEED			0xb0000L

/**********************/
/*     VARIABLES      */
/**********************/

ObjNode		*gRealMePtr;							// ptr to me when not a frog or ship
Boolean		gFrogFlag = false;
Boolean		gSpaceShipFlag = false;

long		gFrogTimer,gSpaceShipTimer;

short		gShipSoundChannelNum;


/********************* TURN ME INTO FROG **********************/

void TurnMeIntoFrog(void)
{
				/* HIDE REAL ME */

	gMyNodePtr->DrawFlag = false;			// stop drawing & moving the "real" me
	gMyNodePtr->MoveFlag = false;
	gRealMePtr = gMyNodePtr;				// remember me

			/* MAKE FROG OBJECT */

	gMyNodePtr = MakeNewShape(GroupNum_MeFrog,ObjType_MeFrog,2,
					gRealMePtr->X.Int,gRealMePtr->Y.Int,gRealMePtr->Z,MoveMeFrog,
					PLAYFIELD_RELATIVE);
	if (gMyNodePtr == nil)
		DoFatalAlert("\pCouldnt init Frog!");

	gMyNodePtr->TopOff = gRealMePtr->TopOff;		// set box
	gMyNodePtr->BottomOff = gRealMePtr->BottomOff;
	gMyNodePtr->LeftOff = gRealMePtr->LeftOff;
	gMyNodePtr->RightOff = gRealMePtr->RightOff;

	gMyNormalMaxSpeed = MY_WALK_SPEED;			// reset normal speed

	gFrogFlag = true;

	gFrogTimer = FROG_DURATION;


				/* MAKE SPARKLE */

	PlaySound(gSoundNum_Frog);					// make frog sound

	MakeNewShape(GroupNum_FrogPoof,ObjType_FrogPoof,0,
				gRealMePtr->X.Int,gRealMePtr->Y.Int,gRealMePtr->Z,nil,
				PLAYFIELD_RELATIVE);
}


/****************** DISPOSE FROG ******************/
//
// Get rid of frog and restore me to normal
//

void DisposeFrog(void)
{
	if (gFrogFlag)
	{
		DeleteObject(gMyNodePtr);				// delete frog
		gMyNodePtr = gRealMePtr;				// restore me
		gMyNodePtr->DrawFlag = true;
		gMyNodePtr->MoveFlag = true;
		gFrogFlag = false;
	}
}

/****************** MOVE ME: FROG ********************/

void MoveMeFrog(void)
{

static	void(*myMoveTable[])(void) =
				{
					MoveMeFrog_Sit,				// sit up
					MoveMeFrog_Sit,				// sit r
					MoveMeFrog_Sit,				// sit d
					MoveMeFrog_Sit,				// sit l
					MoveMeFrog_Jump,			// sit up
					MoveMeFrog_Jump,			// sit r
					MoveMeFrog_Jump,			// sit d
					MoveMeFrog_Jump,			// sit l
				};

	if (--gFrogTimer <= 0)								// see if frog timed out
	{
		DisposeFrog();
		return;
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
}


/*************** MOVE ME FROG: JUMP ****************/

void MoveMeFrog_Jump(void)
{
	gMyMode = MY_MODE_FROG;

	HandleMyCenter();									// check any center spot stuff

	DoMyMove();											// do the move
	CalcMyAim();										// calc aim value (for scroll window)

	if (DoMyCollisionDetect())							// handle collision
		goto update;


update:
	UpdateMe();
}

/*************** MOVE ME FROG:  SIT ****************/

void MoveMeFrog_Sit(void)
{
	gMyMode = MY_MODE_FROG;

	gDX = gDY = 0;										// make sure not moving

	HandleMyCenter();									// check any center spot stuff
	DoMyMove();

	if (DoMyCollisionDetect())							// handle collision
		goto update;

					/* DO JUMP KEYS */

	if (GetKeyState(kKey_Forward))	// see if go up
	{
		gDY = -FROG_JUMP_SPEED;
		SwitchAnim(gThisNodePtr,4);
	}
	else
	if (GetKeyState(kKey_Right)) // see if go right
	{
		gDX = FROG_JUMP_SPEED;
		SwitchAnim(gThisNodePtr,5);
	}
	else
	if (GetKeyState(kKey_Backward)) // see if go down
	{
		gDY = FROG_JUMP_SPEED;
		SwitchAnim(gThisNodePtr,6);
	}
	else
	if (GetKeyState(kKey_Left)) // see if go left
	{
		gDX = -FROG_JUMP_SPEED;
		SwitchAnim(gThisNodePtr,7);
	}

update:
	UpdateMeFrog();
}


/*************** UPDATE ME FROG ***************/

static void UpdateMeFrog(void)
{
	gRealMePtr->X.Int = gThisNodePtr->X.Int;			// update the "real" me with some info
	gRealMePtr->Y.Int = gThisNodePtr->Y.Int;

	UpdateMe();
}



//===================================== SPACESHIP ============================================


/********************* TURN ME INTO SHIP **********************/
//
// shipNode = node of powerup that turns me into ship
//

void TurnMeIntoShip(ObjNode	*shipNode)
{
	shipNode->ItemIndex = nil;						// its no longer a map item - not coming back
	shipNode->MoveCall = MoveMeSpaceShip;			// change move routine
	shipNode->CType = CTYPE_HURTENEMY;				// make hurt enemy


				/* HIDE REAL ME */

	gMyNodePtr->DrawFlag = false;					// stop drawing & moving the "real" me
	gMyNodePtr->MoveFlag = false;
	gRealMePtr = gMyNodePtr;						// remember me

			/* MAKE FROG OBJECT */

	gMyNodePtr = shipNode;							// ship is now me

	gMyNormalMaxSpeed = SPACESHIP_SPEED;				// reset normal speed

	gSpaceShipFlag = true;

	gSpaceShipTimer = SPACESHIP_DURATION;

	gShipSoundChannelNum = PlaySound(gSoundNum_Ship);
}


/****************** DISPOSE SPACESHIP ******************/
//
// Get rid of frog and restore me to normal
//

void DisposeSpaceShip(void)
{
	if (gSpaceShipFlag)
	{
		DeleteObject(gMyNodePtr);				// delete ship
		gMyNodePtr = gRealMePtr;				// restore me
		gMyNodePtr->DrawFlag = true;
		gMyNodePtr->MoveFlag = true;
		gSpaceShipFlag = false;

		gMyNormalMaxSpeed = MY_WALK_SPEED;		// reset normal speed

		StopAChannel(gShipSoundChannelNum);		// make ship sound stop
		PlaySound(gSoundNum_ExitShip);			// play stop sound
	}
}

/****************** MOVE ME: SPACESHIP ********************/

void MoveMeSpaceShip(void)
{
	gMyMode = MY_MODE_SPACESHIP;

	if (--gSpaceShipTimer <= 0)								// see if timed out
	{
		DisposeSpaceShip();
		return;
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
	gMyAcceleration = SPACESHIP_ACCELERATION;			// assume normal acceleration
	gMyWindDX = gMyWindDY = 0;							// assume no wind


	MoveMyController();									// do key controls
	DoMyMove();											// do the move


	if (CalcMyAim())									// calc aim value
	{
		SwitchAnim(gThisNodePtr,gMyDirection);
	}


	HandleMyCenter();									// check any center spot stuff


	if (DoMyCollisionDetect())							// handle collision
		goto update;


update:
	UpdateMeFrog();										// call frog update which sets some important stuff
}





