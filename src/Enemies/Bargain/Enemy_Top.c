/****************************/
/*    	ENEMY_TOP       	*/
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

extern	ObjNode			*gThisNodePtr,*FirstNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY;
extern	MikeFixed		gX;
extern	MikeFixed		gY;
extern	CollisionRec	gCollisionList[];
extern	long				gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	long				gMyX,gMyY;
extern	short				gNumEnemies;
extern	short			gEnemyFreezeTimer;

/****************************/
/*    CONSTANTS             */
/****************************/

#define	TOP_MAX_SPEED	0x50000L
#define	TOP_ACCEL		0x2000L
#define	TOP_HEALTH		16
#define	TOP_WORTH		5
#define	TOP_DAMAGE_THRESHOLD 4


/**********************/
/*     VARIABLES      */
/**********************/


/************************ ADD ENEMY: TOP ********************/

Boolean AddEnemy_Top(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;


	if (gNumEnemies >= MAX_ENEMIES)			// check # enemies
		return(false);

	newObj = MakeNewShape(GroupNum_Top,ObjType_Top,0,itemPtr->x,
						itemPtr->y,50,MoveTop,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	CalcEnemyScatterOffset(newObj);

	newObj->ItemIndex = itemPtr;					// remember where this came from
	newObj->CType = CTYPE_ENEMYA;					// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = TOP_HEALTH;					// set health
	newObj->TopOff = -22;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -20;
	newObj->RightOff = 20;
	CalcObjectBox2(newObj);
	newObj->Worth = TOP_WORTH;						// set worth
	newObj->InjuryThreshold = TOP_DAMAGE_THRESHOLD;

	gNumEnemies++;

	return(true);									// was added
}


/********************* MOVE TOP *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveTop(void)
{
	if (gEnemyFreezeTimer)							// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	if (TrackEnemy())								// see if out of range
		return;

	GetObjectInfo();

	DoTopMove();

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	UpdateTop();
}


/**************** UPDATE TOP *******************/

void UpdateTop(void)
{
	if (!(MyRandomLong() & b1111111))							// see if recalc scatter
		CalcEnemyScatterOffset(gThisNodePtr);

	gThisNodePtr->AnimSpeed = (Absolute(gDX)+Absolute(gDY))>>8;

	UpdateEnemy();
}


/************** DO TOP MOVE ******************/

void DoTopMove(void)
{
long	xAcc,yAcc;

				/* ACCEL TOWARD ME */

	if (gX.Int < (gMyX+gThisNodePtr->EnemyTargetXOff))
		xAcc = TOP_ACCEL;										// accel right
	else
	if (gX.Int > (gMyX+gThisNodePtr->EnemyTargetXOff))
		xAcc = -TOP_ACCEL;										// accel left
	else
		xAcc = 0;

	if (gY.Int > (gMyY+gThisNodePtr->EnemyTargetYOff))
		yAcc = -TOP_ACCEL;										// accel up
	else
	if (gY.Int < (gMyY+gThisNodePtr->EnemyTargetYOff))
		yAcc = TOP_ACCEL;										// accel down
	else
		yAcc = 0;

	gDX += xAcc;
	gDY += yAcc;

				/* CHECK MAX DELTAS */

	if (gDX > TOP_MAX_SPEED)
		gDX -= TOP_ACCEL;
	else
	if (gDX < -TOP_MAX_SPEED)
		gDX +=  TOP_ACCEL;

	if (gDY > TOP_MAX_SPEED)
		gDY -=  TOP_ACCEL;
	else
	if (gDY < -TOP_MAX_SPEED)
		gDY +=  TOP_ACCEL;

					/* MOVE IT */
	gX.L += gDX;
	gY.L += gDY;

}



