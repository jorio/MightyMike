/****************************/
/*    	ENEMY_REX       	*/
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
#include "sound2.h"
#include "objecttypes.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY;
extern	union_gX;
extern	union_gY;
extern	CollisionRec	gCollisionList[];
extern	long				gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	long				gMyX,gMyY;
extern	short				gNumEnemies,gSoundNum_UngaBunga;
extern	short			gEnemyFreezeTimer;

/****************************/
/*    CONSTANTS             */
/****************************/

#define	REX_MAX_SPEED	0x48000L
#define	REX_ACCEL		0x3000L
#define	REX_HEALTH		7
#define	REX_WORTH		2
#define	HATCHLING_DAMAGE_THRESHOLD 3

enum
{
	SUB_WALK_RIGHT,
	SUB_WALK_LEFT
};


/**********************/
/*     VARIABLES      */
/**********************/


/************************ ADD ENEMY: REX ********************/

Boolean AddEnemy_Rex(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;


	if (gNumEnemies >= MAX_ENEMIES)			// check # enemies
		return(false);

	newObj = MakeNewShape(GroupNum_Rex,ObjType_Rex,SUB_WALK_RIGHT,itemPtr->x,
						itemPtr->y,50,MoveRex,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	CalcEnemyScatterOffset(newObj);

	newObj->ItemIndex = itemPtr;					// remember where this came from
	newObj->CType = CTYPE_ENEMYA;					// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = REX_HEALTH;					// set health
	newObj->TopOff = -22;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -20;
	newObj->RightOff = 20;
	CalcObjectBox2(newObj);
	newObj->Worth = REX_WORTH;						// set worth
	newObj->InjuryThreshold = HATCHLING_DAMAGE_THRESHOLD;

	gNumEnemies++;

	return(true);									// was added
}


/********************* MOVE REX *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveRex(void)
{
	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	if (TrackEnemy())									// see if out of range
		return;

	GetObjectInfo();

	DoRexMove();

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	UpdateRex();
}



/**************** UPDATE REX *******************/

void UpdateRex(void)
{
	if (!(MyRandomLong() & b1111111))							// see if recalc scatter
		CalcEnemyScatterOffset(gThisNodePtr);

	gThisNodePtr->AnimSpeed = (Absolute(gDX)+Absolute(gDY))>>8;

	UpdateEnemy();
}


/************** DO REX MOVE ******************/

void DoRexMove(void)
{
long	xAcc,yAcc;

				/* ACCEL TOWARD ME */

	if (gX.Int < (gMyX+gThisNodePtr->EnemyTargetXOff))
		xAcc = REX_ACCEL;										// accel right
	else
	if (gX.Int > (gMyX+gThisNodePtr->EnemyTargetXOff))
		xAcc = -REX_ACCEL;										// accel left
	else
		xAcc = 0;

	if (gY.Int > (gMyY+gThisNodePtr->EnemyTargetYOff))
		yAcc = -REX_ACCEL;										// accel up
	else
	if (gY.Int < (gMyY+gThisNodePtr->EnemyTargetYOff))
		yAcc = REX_ACCEL;										// accel down
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

	if (gDX > REX_MAX_SPEED)
		gDX -= REX_ACCEL;
	else
	if (gDX < -REX_MAX_SPEED)
		gDX +=  REX_ACCEL;

	if (gDY > REX_MAX_SPEED)
		gDY -=  REX_ACCEL;
	else
	if (gDY < -REX_MAX_SPEED)
		gDY +=  REX_ACCEL;

					/* MOVE IT */
	gX.L += gDX;
	gY.L += gDY;

}



