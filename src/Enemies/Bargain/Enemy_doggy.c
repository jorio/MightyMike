/****************************/
/*    	ENEMY_DOGGY       	*/
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/****************************/
/*    EXTERNALS             */
/****************************/
#include "myglobals.h"
#include "window.h"
#include "playfield.h"
#include "enemy.h"
#include "object.h"
#include "shape.h"
#include "misc.h"
#include "miscanims.h"
#include "objecttypes.h"
#include "enemy5.h"
#include "sound2.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr,*gMyNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY,gFrames;
extern	MikeFixed		gX;
extern	MikeFixed		gY;
extern	CollisionRec	gCollisionList[];
extern	long			gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	long			gMyX,gMyY;
extern	short			gNumEnemies,gSoundNum_DogRoar;
extern	short			gEnemyFreezeTimer;

/****************************/
/*    CONSTANTS             */
/****************************/

#define	DOGGY_MAX_SPEED	0x20000L
#define	DOGGY_ACCEL		0x8000L
#define	DOGGY_HEALTH	2
#define	DOGGY_WORTH		1
#define	DOGGY_DAMAGE_THRESHOLD 1

enum
{
	SUB_WALK_RIGHT,
	SUB_WALK_LEFT,
	SUB_WAG_RIGHT,
	SUB_WAG_LEFT,
	SUB_FLIP_RIGHT,
	SUB_FLIP_LEFT,
	SUB_ROAR
};

#define	WAG_TIME	(GAME_FPS*2)

/**********************/
/*     VARIABLES      */
/**********************/

#define WagTimer	Special3
#define	DoneJumpFlag	Flag0

long	gLastDogRoarTime = 0;


/************************ ADD ENEMY: DOGGY ********************/

Boolean AddEnemy_Doggy(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;


	if (gNumEnemies >= MAX_ENEMIES)					// check # enemies
		return(false);

	newObj = MakeNewShape(GroupNum_Doggy,ObjType_Doggy,SUB_WALK_RIGHT,itemPtr->x,
						itemPtr->y,50,MoveDoggy,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	CalcEnemyScatterOffset(newObj);

	newObj->ItemIndex = itemPtr;					// remember where this came from
	newObj->CType = CTYPE_ENEMYA;					// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = DOGGY_HEALTH;					// set health
	newObj->TopOff = -20;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -15;
	newObj->RightOff = 15;
	CalcObjectBox2(newObj);
	newObj->Worth = DOGGY_WORTH;					// set worth
	newObj->InjuryThreshold = DOGGY_DAMAGE_THRESHOLD;

	gNumEnemies++;

	return(true);									// was added
}


/********************* MOVE DOGGY *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveDoggy(void)
{

static	void(*moveTable[])(void) =
				{
					MoveDoggy_Walk,			// walk right
					MoveDoggy_Walk,			// walk left
					MoveDoggy_Wag,			// wag right
					MoveDoggy_Wag,			// wag left
					MoveDoggy_Jump,			// jump right
					MoveDoggy_Jump			// jump left
				};

	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	DoDogRoar();

	GetObjectInfo();
	moveTable[gThisNodePtr->SubType]();
}


/******************** MOVE DOGGY: WALK *************************/

void MoveDoggy_Walk(void)
{
	if (TrackEnemy())								// see if out of range
		return;

	GetObjectInfo();

	DoDoggyMove();

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

					/* SEE IF DO FUNKY MOVE */

	if (!(MyRandomLong()&b1111111))
	{
		if (MyRandomLong()&1)							// see if wag or flip
		{
						/* WAG */

			SwitchAnim(gThisNodePtr,SUB_WAG_RIGHT+gThisNodePtr->SubType);
			gDX = gDY = 0;
			gThisNodePtr->WagTimer = WAG_TIME+(MyRandomLong()&b111111);
		}
		else
		{
						/* FLIP */

			SwitchAnim(gThisNodePtr,SUB_FLIP_RIGHT+gThisNodePtr->SubType);
			gDX = gDY = 0;
			gThisNodePtr->DoneJumpFlag = false;
		}
	}

	UpdateDoggy();
}


/******************** MOVE DOGGY: WAG *************************/

void MoveDoggy_Wag(void)
{
	if (TrackEnemy())								// see if out of range
		return;

	GetObjectInfo();

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	if (--gThisNodePtr->WagTimer < 0)
	{
		SwitchAnim(gThisNodePtr,gThisNodePtr->SubType-2);
	}

	UpdateDoggy();
}


/******************** MOVE DOGGY: JUMP *************************/

void MoveDoggy_Jump(void)
{
	if (TrackEnemy())								// see if out of range
		return;

	GetObjectInfo();

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	if (gThisNodePtr->DoneJumpFlag)
	{
		SwitchAnim(gThisNodePtr,gThisNodePtr->SubType-4);
	}

	UpdateDoggy();
}


/**************** UPDATE DOGGY *******************/

void UpdateDoggy(void)
{
	if (!(MyRandomLong() & b1111111))							// see if recalc scatter
		CalcEnemyScatterOffset(gThisNodePtr);

	UpdateEnemy();
}


/************** DO DOGGY MOVE ******************/

void DoDoggyMove(void)
{
long	xAcc,yAcc;

				/* ACCEL TOWARD ME */

	if (gX.Int < (gMyX+gThisNodePtr->EnemyTargetXOff))
		xAcc = DOGGY_ACCEL;										// accel right
	else
	if (gX.Int > (gMyX+gThisNodePtr->EnemyTargetXOff))
		xAcc = -DOGGY_ACCEL;									// accel left
	else
		xAcc = 0;

	if (gY.Int > (gMyY+gThisNodePtr->EnemyTargetYOff))
		yAcc = -DOGGY_ACCEL;									// accel up
	else
	if (gY.Int < (gMyY+gThisNodePtr->EnemyTargetYOff))
		yAcc = DOGGY_ACCEL;										// accel down
	else
		yAcc = 0;

	gDX += xAcc;
	gDY += yAcc;

			/* SET CORRECT ANIMATION */


	if (xAcc < 0)
	{
		if (gThisNodePtr->SubType != SUB_WALK_LEFT)			// left anim
			SwitchAnim(gThisNodePtr,SUB_WALK_LEFT);
	}
	else
	{
		if (gThisNodePtr->SubType != SUB_WALK_RIGHT)		// right anim
			SwitchAnim(gThisNodePtr,SUB_WALK_RIGHT);
	}

				/* CHECK MAX DELTAS */

	if (gDX > DOGGY_MAX_SPEED)
		gDX -= DOGGY_ACCEL;
	else
	if (gDX < -DOGGY_MAX_SPEED)
		gDX +=  DOGGY_ACCEL;

	if (gDY > DOGGY_MAX_SPEED)
		gDY -=  DOGGY_ACCEL;
	else
	if (gDY < -DOGGY_MAX_SPEED)
		gDY +=  DOGGY_ACCEL;

					/* MOVE IT */
	gX.L += gDX;
	gY.L += gDY;

}



/**************** DO DOG ROAR *****************/

void DoDogRoar(void)
{
register	ObjNode *newObj;

	if ((gFrames-gLastDogRoarTime) < GAME_FPS)			// see if been enough time
		return;

	if (gThisNodePtr->OwnerToMessageNode != nil)		// see if already has a message
		return;
	if (gMyNodePtr->OwnerToMessageNode != nil)			// not if Mike is talking
		return;
	if (MyRandomLong()&b11111111)						// random
		return;

				/* MAKE MESSAGE SPRITE */

	newObj = MakeNewShape(GroupNum_Doggy,ObjType_Doggy,SUB_ROAR,
				gThisNodePtr->X.Int,gThisNodePtr->Y.Int,gThisNodePtr->Z,MoveMessage,
				PLAYFIELD_RELATIVE);

	if (newObj == nil)
		return;

	newObj->MessageTimer = GAME_FPS;					// set message timer
	newObj->TileMaskFlag = false;						// wont be tile masked
	newObj->MessageToOwnerNode = gThisNodePtr;			// point to owner
	gThisNodePtr->OwnerToMessageNode = newObj;			// point to message

	PlaySound(gSoundNum_DogRoar);

	gLastDogRoarTime = gFrames;						// remember when it occurred
}

