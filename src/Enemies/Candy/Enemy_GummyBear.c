/****************************/
/*    	ENEMY_GBear        */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/****************************/
/*    EXTERNALS             */
/****************************/
#include "myglobals.h"
#include "windows.h"
#include "playfield.h"
#include "enemy.h"
#include "object.h"
#include "misc.h"
#include "shape.h"
#include "miscanims.h"
#include "sound2.h"
#include "enemy3.h"
#include "objecttypes.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY,gFrames;
extern	union_gX;
extern	union_gY;
extern	CollisionRec	gCollisionList[];
extern	long				gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	long				gMyX,gMyY;
extern	short				gNumEnemies,gSoundNum_UngaBunga,gSoundNum_GummyHaha;
extern	ObjNode			*gMyNodePtr;
extern	short			gEnemyFreezeTimer;


/****************************/
/*    CONSTANTS             */
/****************************/

#define	GBEAR_MAX_SPEED	0x2a000L
#define	GBEAR_ACCEL		0x3000L
#define	GBEAR_HEALTH	2
#define	GBEAR_WORTH		1
#define GBEAR_DAMAGE_THRESHOLD	1

#define	POP_MAX_DIST		250
#define	POP_SPEED			0x80000L

enum
{
	SUB_WALK_RIGHT = 0,
	SUB_WALK_LEFT,
	SUB_TINY_RIGHT,
	SUB_TINY_LEFT,
	SUB_HAHA
};

/**********************/
/*     VARIABLES      */
/**********************/

long	gLastGummyHahaTime = 0;


/************************ ADD ENEMY: GBEAR********************/

Boolean AddEnemy_GBear(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	if (gNumEnemies >= MAX_ENEMIES)				// check # enemies
		return(false);

	newObj = MakeNewShape(GroupNum_RedGummy,ObjType_RedGummy,SUB_WALK_RIGHT,itemPtr->x,
						itemPtr->y,50,MoveGBear,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;				// remember where this came from
	newObj->CType = CTYPE_ENEMYA;				// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = GBEAR_HEALTH;				// set health
	newObj->TopOff = -25;						// set box
	newObj->BottomOff = 3;
	newObj->LeftOff = -20;
	newObj->RightOff = 20;
	CalcObjectBox2(newObj);
	newObj->Worth = GBEAR_WORTH;				// set worth
	newObj->InjuryThreshold = GBEAR_DAMAGE_THRESHOLD;

	CalcEnemyScatterOffset(newObj);

	gNumEnemies++;

	return(true);								// was added
}


/********************* MOVE GBEAR *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveGBear(void)
{
	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	if (TrackEnemy())									// see if out of range
		return;

	DoGummyHaha();

	GetObjectInfo();

	DoGBearMove();									// move normally

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
	{
		ExplodeGummy();
		return;
	}

	UpdateGBear();
}



/**************** UPDATE GBEAR*******************/

static void UpdateGBear(void)
{
	if (gThisNodePtr->Health != GBEAR_HEALTH)			// see if hit
		goto update;

	if (!(MyRandomLong() & b1111111))					// see if recalc scatter
		CalcEnemyScatterOffset(gThisNodePtr);

	if (gDX < 0)										// check aim anim
	{
		if (gThisNodePtr->SubType != SUB_WALK_LEFT)
			SwitchAnim(gThisNodePtr,SUB_WALK_LEFT);
	}
	else
	if (gDX > 0)
	{
		if (gThisNodePtr->SubType != SUB_WALK_RIGHT)
			SwitchAnim(gThisNodePtr,SUB_WALK_RIGHT);
	}

	gThisNodePtr->AnimSpeed = (Absolute(gDX)+Absolute(gDY))>>8;

update:
	UpdateEnemy();
}


/************** DO GBEARMOVE ******************/

static void DoGBearMove(void)
{

				/* ACCEL TOWARD ME */

	if (gX.Int < (gMyX+gThisNodePtr->EnemyTargetXOff))
		gDX += GBEAR_ACCEL;
	else
	if (gX.Int > (gMyX+gThisNodePtr->EnemyTargetXOff))
		gDX -= GBEAR_ACCEL;

	if (gY.Int > (gMyY+gThisNodePtr->EnemyTargetYOff))
		gDY -= GBEAR_ACCEL;
	else
	if (gY.Int < (gMyY+gThisNodePtr->EnemyTargetYOff))
		gDY += GBEAR_ACCEL;

				/* CHECK MAX DELTAS */

	if (gDX > GBEAR_MAX_SPEED)
		gDX -= GBEAR_ACCEL;
	else
	if (gDX < -GBEAR_MAX_SPEED)
		gDX +=  GBEAR_ACCEL;

	if (gDY > GBEAR_MAX_SPEED)
		gDY -=  GBEAR_ACCEL;
	else
	if (gDY < -GBEAR_MAX_SPEED)
		gDY +=  GBEAR_ACCEL;

					/* MOVE IT */
	gX.L += gDX;
	gY.L += gDY;
}


/************************ EXPLODE GUMMY *******************/

static void ExplodeGummy(void)
{
register	ObjNode		*newObj;
short			i;


	for (i=0; i < 3; i++)
	{
		if (gNumEnemies >= (MAX_ENEMIES+3))				// check # enemies
			return;

		newObj = MakeNewShape(GroupNum_RedGummy,ObjType_RedGummy,SUB_TINY_RIGHT,gX.Int,
							gY.Int,50,MoveTinyGummy,PLAYFIELD_RELATIVE);
		if (newObj == nil)
			return;

		newObj->CType = CTYPE_ENEMYA;				// set collision info
		newObj->CBits = CBITS_TOUCHABLE;
		newObj->Health = 1;							// set health
		newObj->TopOff = -10;						// set box
		newObj->BottomOff = 2;
		newObj->LeftOff = -8;
		newObj->RightOff = 8;
//		CalcObjectBox2(newObj);

		newObj->Worth = 1;							// set worth
		newObj->InjuryThreshold = 1;

		CalcEnemyScatterOffset(newObj);

		gNumEnemies++;

	}
}

/********************* MOVE TINY GUMMY *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveTinyGummy(void)
{
	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	if (TrackEnemy())									// see if out of range
		return;

	GetObjectInfo();

	DoGBearMove();

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	UpdateTinyGummy();
}




/**************** UPDATE TINY GUMMY *******************/

static void UpdateTinyGummy(void)
{

	if (!(MyRandomLong() & b1111111))							// see if recalc scatter
		CalcEnemyScatterOffset(gThisNodePtr);

	if (gDX < 0)										// check aim anim
	{
		if (gThisNodePtr->SubType != SUB_TINY_LEFT)
			SwitchAnim(gThisNodePtr,SUB_TINY_LEFT);
	}
	else
	if (gDX > 0)
	{
		if (gThisNodePtr->SubType != SUB_TINY_RIGHT)
			SwitchAnim(gThisNodePtr,SUB_TINY_RIGHT);
	}

	gThisNodePtr->AnimSpeed = (Absolute(gDX)+Absolute(gDY))>>8;

	UpdateEnemy();
}



/**************** DO GUMMY HAHA *****************/
//
//
//

static void DoGummyHaha(void)
{
register	ObjNode *newObj;

	if ((gFrames-gLastGummyHahaTime) < GAME_FPS)		// see if been enough time
		return;

	if (gThisNodePtr->OwnerToMessageNode != nil)		// see if already has a message
		return;
	if (gMyNodePtr->OwnerToMessageNode != nil)			// not if Mike is talking
		return;
	if (MyRandomLong()&b111111111)							// random
		return;

				/* MAKE MESSAGE SPRITE */

	newObj = MakeNewShape(GroupNum_RedGummy,ObjType_RedGummy,SUB_HAHA,
				gThisNodePtr->X.Int,gThisNodePtr->Y.Int,gThisNodePtr->Z,MoveMessage,
				PLAYFIELD_RELATIVE);

	if (newObj == nil)
		return;

	newObj->MessageTimer = GAME_FPS*3/2;				// set message timer
	newObj->TileMaskFlag = false;						// wont be tile masked
	newObj->MessageToOwnerNode = gThisNodePtr;			// point to owner
	gThisNodePtr->OwnerToMessageNode = newObj;			// point to message

	PlaySound(gSoundNum_GummyHaha);

	gLastGummyHahaTime = gFrames;						// remember when it occurred
}








