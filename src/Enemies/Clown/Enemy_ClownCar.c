/****************************/
/*    	ENEMY_CLOWNCAR     	*/
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
#include "shape.h"
#include "enemy2.h"
#include "object.h"
#include "misc.h"
#include "miscanims.h"
#include "objecttypes.h"
#include "sound2.h"
#include "externs.h"

/****************************/
/*    CONSTANTS             */
/****************************/

#define	CLOWNCAR_HEALTH			40
#define	CLOWNCAR_WORTH			5
#define	CLOWNCAR_DAMAGE_THRESHOLD	4

#define		CLOWNCAR_SPEED	0x48000L

enum
{
	CLOWNCAR_SUB_DRIVE_UP,
	CLOWNCAR_SUB_DRIVE_RIGHT,
	CLOWNCAR_SUB_DRIVE_DOWN,
	CLOWNCAR_SUB_DRIVE_LEFT,
	CLOWNCAR_SUB_OPEN_UP,
	CLOWNCAR_SUB_OPEN_RIGHT,
	CLOWNCAR_SUB_OPEN_DOWN,
	CLOWNCAR_SUB_OPEN_LEFT,
	CARCLOWN_SUB_WALK_UP,
	CARCLOWN_SUB_WALK_RIGHT,
	CARCLOWN_SUB_WALK_DOWN,
	CARCLOWN_SUB_WALK_LEFT
};

#define	CARCLOWN_HEALTH				1
#define	CARCLOWN_WORTH				1
#define	CARCLOWN_DAMAGE_THRESHOLD	1

#define	STOP_DIST		90
#define	CARCLOWN_ACCEL	0x3000L
#define	CARCLOWN_MAX_SPEED	0x20000L


/**********************/
/*     VARIABLES      */
/**********************/

#define	ClownOutFlag	Flag0

/************************ ADD ENEMY: CLOWNCAR ********************/

Boolean AddEnemy_ClownCar(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;


	if (gNumEnemies >= MAX_ENEMIES)			// check # enemies
		return(false);

	newObj = MakeNewShape(GroupNum_ClownCar,ObjType_ClownCar,CLOWNCAR_SUB_DRIVE_UP,
			itemPtr->x,itemPtr->y,50,MoveClownCar_Driving,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CType = CTYPE_ENEMYA;		// set collision info
	newObj->CBits = CBITS_ALLSOLID;

	newObj->TopOff = -10;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -10;
	newObj->RightOff = 10;
	CalcObjectBox2(newObj);

	newObj->Worth = CLOWNCAR_WORTH;					// set worth
	newObj->Health = CLOWNCAR_HEALTH;				// set health
	newObj->InjuryThreshold = CLOWNCAR_DAMAGE_THRESHOLD;
	newObj->ClownOutFlag = false;

	gNumEnemies++;

	return(true);									// was added
}


/********************* MOVE CLOWNCAR: DRIVING *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveClownCar_Driving(void)
{
	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}
	if (TrackEnemy())											// see if out of range
		return;

	GetObjectInfo();
	switch(MoveOnPath(CLOWNCAR_SPEED,false))						// see which way to aim
	{
		case	ALT_TILE_DIR_UP:
				if (gThisNodePtr->SubType != CLOWNCAR_SUB_DRIVE_UP)
					SwitchAnim(gThisNodePtr,CLOWNCAR_SUB_DRIVE_UP);
				break;

		case	ALT_TILE_DIR_RIGHT:
				if (gThisNodePtr->SubType != CLOWNCAR_SUB_DRIVE_RIGHT)
					SwitchAnim(gThisNodePtr,CLOWNCAR_SUB_DRIVE_RIGHT);
				break;

		case	ALT_TILE_DIR_DOWN:
				if (gThisNodePtr->SubType != CLOWNCAR_SUB_DRIVE_DOWN)
					SwitchAnim(gThisNodePtr,CLOWNCAR_SUB_DRIVE_DOWN);
				break;

		case	ALT_TILE_DIR_LEFT:
				if (gThisNodePtr->SubType != CLOWNCAR_SUB_DRIVE_LEFT)
					SwitchAnim(gThisNodePtr,CLOWNCAR_SUB_DRIVE_LEFT);
				break;
	}

				/* DO COLLISION DETECT */

	if (DoEnemyCollisionDetect(ENEMY_NO_BG_COLLISION))			// returns true if died
		return;

					/* SEE IF STOP */

	if ((Absolute(gX.Int-gMyX) < STOP_DIST) && ((Absolute(gY.Int-gMyY) < STOP_DIST)))
	{
		SwitchAnim(gThisNodePtr,gThisNodePtr->SubType+4);		// do "stop" anim
		gThisNodePtr->MoveCall = MoveClownCar_Stopped;
		PlaySound(gSoundNum_Skid);								// screech!!
	}


	UpdateEnemy();
}


/********************* MOVE CLOWNCAR: STOPPED *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveClownCar_Stopped(void)
{
	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}
	if (TrackEnemy())											// see if out of range
		return;

	GetObjectInfo();

	Decay(&gDX,0x3000);											// slow down
	Decay(&gDY,0x3000);
	gX.L += gDX;												// move
	gY.L += gDY;

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))			// returns true if died
		return;

	if (gThisNodePtr->ClownOutFlag)
	{
		gThisNodePtr->ClownOutFlag = false;
		MakeCarClown();
	}

	UpdateEnemy();
}


/*********************** MAKE CAR CLOWN ***********************/

void MakeCarClown(void)
{
register	ObjNode		*newObj;

	if (gNumEnemies >= (MAX_ENEMIES*3/2))					// check # enemies
		return;

	newObj = MakeNewShape(GroupNum_ClownCar,ObjType_ClownCar,CARCLOWN_SUB_WALK_UP,
			gX.Int,gY.Int+9,gThisNodePtr->Z,MoveCarClown,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return;

	newObj->CType = CTYPE_ENEMYA;					// set collision info
	newObj->CBits = CBITS_TOUCHABLE;

	newObj->TopOff = -8;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -10;
	newObj->RightOff = 10;
	CalcObjectBox2(newObj);

	newObj->Worth = CARCLOWN_WORTH;					// set worth
	newObj->Health = CARCLOWN_HEALTH;				// set health
	newObj->InjuryThreshold = CARCLOWN_DAMAGE_THRESHOLD;

	CalcEnemyScatterOffset(newObj);

	gNumEnemies++;
}

/********************** MOVE CAR CLOWN *********************/

void MoveCarClown(void)
{
	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	if (TrackEnemy())									// see if out of range
		return;

	GetObjectInfo();

	DoCarClownMove();

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	UpdateCarClown();


}

/************** DO CAR CLOWN MOVE ******************/

void DoCarClownMove(void)
{
long	xAcc,yAcc;

				/* ACCEL TOWARD ME */

	if (gX.Int < (gMyX+gThisNodePtr->EnemyTargetXOff))
		xAcc = CARCLOWN_ACCEL;										// accel right
	else
	if (gX.Int > (gMyX+gThisNodePtr->EnemyTargetXOff))
		xAcc = -CARCLOWN_ACCEL;										// accel left
	else
		xAcc = 0;

	if (gY.Int > (gMyY+gThisNodePtr->EnemyTargetYOff))
		yAcc = -CARCLOWN_ACCEL;										// accel up
	else
	if (gY.Int < (gMyY+gThisNodePtr->EnemyTargetYOff))
		yAcc = CARCLOWN_ACCEL;										// accel down
	else
		yAcc = 0;

	gDX += xAcc;
	gDY += yAcc;

			/* SET CORRECT ANIMATION */

	if ((Absolute(gDY)>>1) > Absolute(gDX))						// see if predominantly vertical or horiz
	{
		if (yAcc < 0)
		{
			if (gThisNodePtr->SubType != CARCLOWN_SUB_WALK_UP)			// up anim
				SwitchAnim(gThisNodePtr,CARCLOWN_SUB_WALK_UP);
		}
		else
		{
			if (gThisNodePtr->SubType != CARCLOWN_SUB_WALK_DOWN)		// down anim
				SwitchAnim(gThisNodePtr,CARCLOWN_SUB_WALK_DOWN);
		}
	}
	else														// HORIZ
	{
		if (xAcc < 0)
		{
			if (gThisNodePtr->SubType != CARCLOWN_SUB_WALK_LEFT)		// left anim
				SwitchAnim(gThisNodePtr,CARCLOWN_SUB_WALK_LEFT);
		}
		else
		{
			if (gThisNodePtr->SubType != CARCLOWN_SUB_WALK_RIGHT)		// right anim
				SwitchAnim(gThisNodePtr,CARCLOWN_SUB_WALK_RIGHT);
		}
	}

				/* CHECK MAX DELTAS */

	if (gDX > CARCLOWN_MAX_SPEED)
		gDX -= CARCLOWN_ACCEL;
	else
	if (gDX < -CARCLOWN_MAX_SPEED)
		gDX +=  CARCLOWN_ACCEL;

	if (gDY > CARCLOWN_MAX_SPEED)
		gDY -=  CARCLOWN_ACCEL;
	else
	if (gDY < -CARCLOWN_MAX_SPEED)
		gDY +=  CARCLOWN_ACCEL;

					/* MOVE IT */
	gX.L += gDX;
	gY.L += gDY;

}


/**************** UPDATE CAR CLOWN *******************/

void UpdateCarClown(void)
{
	if (!(MyRandomLong() & b1111111))							// see if recalc scatter
		CalcEnemyScatterOffset(gThisNodePtr);

	gThisNodePtr->AnimSpeed = (Absolute(gDX)+Absolute(gDY))>>8;

	UpdateEnemy();
}


