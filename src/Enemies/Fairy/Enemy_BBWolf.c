/****************************/
/*    	ENEMY_BBWOLF       	*/
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
#include "shape.h"
#include "misc.h"
#include "miscanims.h"
#include "objecttypes.h"
#include "enemy4.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY;
extern	union_gX;
extern	union_gY;
extern	CollisionRec	gCollisionList[];
extern	long				gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	long				gMyX,gMyY;
extern	short				gNumEnemies;
extern	short			gEnemyFreezeTimer;

/****************************/
/*    CONSTANTS             */
/****************************/

#define	BBWOLF_MAX_SPEED	0x40000L
#define	BBWOLF_ACCEL		0x5000L
#define	BBWOLF_HEALTH		3
#define	BBWOLF_WORTH		1
#define	BBWOLF_DAMAGE_THRESHOLD 1

enum
{
	SUB_WALK_RIGHT,
	SUB_WALK_LEFT
};


/**********************/
/*     VARIABLES      */
/**********************/


/************************ ADD ENEMY: BBWOLF ********************/

Boolean AddEnemy_BBWolf(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;


	if (gNumEnemies >= MAX_ENEMIES)			// check # enemies
		return(false);

	newObj = MakeNewShape(GroupNum_BBWolf,ObjType_BBWolf,SUB_WALK_RIGHT,itemPtr->x,
						itemPtr->y,50,MoveBBWolf,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	CalcEnemyScatterOffset(newObj);

	newObj->ItemIndex = itemPtr;					// remember where this came from
	newObj->CType = CTYPE_ENEMYA;					// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = BBWOLF_HEALTH;					// set health
	newObj->TopOff = -22;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -20;
	newObj->RightOff = 20;
	CalcObjectBox2(newObj);
	newObj->Worth = BBWOLF_WORTH;						// set worth
	newObj->InjuryThreshold = BBWOLF_DAMAGE_THRESHOLD;

	gNumEnemies++;

	return(true);									// was added
}


/********************* MOVE BBWOLF *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveBBWolf(void)
{
	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	if (TrackEnemy())									// see if out of range
		return;

	GetObjectInfo();

	DoBBWolfMove();

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	UpdateBBWolf();
}



/**************** UPDATE BBWOLF *******************/

void UpdateBBWolf(void)
{
	if (!(MyRandomLong() & b1111111))							// see if recalc scatter
		CalcEnemyScatterOffset(gThisNodePtr);

	gThisNodePtr->AnimSpeed = (Absolute(gDX)+Absolute(gDY))>>8;

	UpdateEnemy();
}


/************** DO BBWOLF MOVE ******************/

void DoBBWolfMove(void)
{
long	xAcc,yAcc;

				/* ACCEL TOWARD ME */

	if (gX.Int < (gMyX+gThisNodePtr->EnemyTargetXOff))
		xAcc = BBWOLF_ACCEL;										// accel right
	else
	if (gX.Int > (gMyX+gThisNodePtr->EnemyTargetXOff))
		xAcc = -BBWOLF_ACCEL;										// accel left
	else
		xAcc = 0;

	if (gY.Int > (gMyY+gThisNodePtr->EnemyTargetYOff))
		yAcc = -BBWOLF_ACCEL;										// accel up
	else
	if (gY.Int < (gMyY+gThisNodePtr->EnemyTargetYOff))
		yAcc = BBWOLF_ACCEL;										// accel down
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

	if (gDX > BBWOLF_MAX_SPEED)
		gDX -= BBWOLF_ACCEL;
	else
	if (gDX < -BBWOLF_MAX_SPEED)
		gDX +=  BBWOLF_ACCEL;

	if (gDY > BBWOLF_MAX_SPEED)
		gDY -=  BBWOLF_ACCEL;
	else
	if (gDY < -BBWOLF_MAX_SPEED)
		gDY +=  BBWOLF_ACCEL;

					/* MOVE IT */
	gX.L += gDX;
	gY.L += gDY;

}



