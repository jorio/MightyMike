/****************************/
/*    	ENEMY_DRAGON       	*/
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
#include "enemy4.h"

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

#define	DRAGON_MAX_SPEED	0x50000L
#define	DRAGON_ACCEL		0x3000L
#define	DRAGON_HEALTH		6
#define	DRAGON_WORTH		2
#define	DRAGON_DAMAGE_THRESHOLD 3

enum
{
	SUB_WALK_RIGHT,
	SUB_WALK_LEFT
};


/**********************/
/*     VARIABLES      */
/**********************/


/************************ ADD ENEMY: DRAGON ********************/

Boolean AddEnemy_Dragon(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;


	if (gNumEnemies >= MAX_ENEMIES)			// check # enemies
		return(false);

	newObj = MakeNewShape(GroupNum_Dragon,ObjType_Dragon,SUB_WALK_RIGHT,itemPtr->x,
						itemPtr->y,50,MoveDragon,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	CalcEnemyScatterOffset(newObj);

	newObj->ItemIndex = itemPtr;					// remember where this came from
	newObj->CType = CTYPE_ENEMYA;					// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = DRAGON_HEALTH;					// set health
	newObj->TopOff = -22;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -20;
	newObj->RightOff = 20;
	CalcObjectBox2(newObj);
	newObj->Worth = DRAGON_WORTH;						// set worth
	newObj->InjuryThreshold = DRAGON_DAMAGE_THRESHOLD;

	gNumEnemies++;

	return(true);									// was added
}


/********************* MOVE DRAGON *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveDragon(void)
{
	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	if (TrackEnemy())									// see if out of range
		return;

	GetObjectInfo();

	DoDragonMove();

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	UpdateDragon();
}



/**************** UPDATE DRAGON *******************/

void UpdateDragon(void)
{
	if (!(MyRandomLong() & b1111111))							// see if recalc scatter
		CalcEnemyScatterOffset(gThisNodePtr);

	gThisNodePtr->AnimSpeed = (Absolute(gDX)+Absolute(gDY))>>8;

	UpdateEnemy();
}


/************** DO DRAGON MOVE ******************/

void DoDragonMove(void)
{
long	xAcc,yAcc;

				/* ACCEL TOWARD ME */

	if (gX.Int < (gMyX+gThisNodePtr->EnemyTargetXOff))
		xAcc = DRAGON_ACCEL;										// accel right
	else
	if (gX.Int > (gMyX+gThisNodePtr->EnemyTargetXOff))
		xAcc = -DRAGON_ACCEL;										// accel left
	else
		xAcc = 0;

	if (gY.Int > (gMyY+gThisNodePtr->EnemyTargetYOff))
		yAcc = -DRAGON_ACCEL;										// accel up
	else
	if (gY.Int < (gMyY+gThisNodePtr->EnemyTargetYOff))
		yAcc = DRAGON_ACCEL;										// accel down
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

	if (gDX > DRAGON_MAX_SPEED)
		gDX -= DRAGON_ACCEL;
	else
	if (gDX < -DRAGON_MAX_SPEED)
		gDX +=  DRAGON_ACCEL;

	if (gDY > DRAGON_MAX_SPEED)
		gDY -=  DRAGON_ACCEL;
	else
	if (gDY < -DRAGON_MAX_SPEED)
		gDY +=  DRAGON_ACCEL;

					/* MOVE IT */
	gX.L += gDX;
	gY.L += gDY;

}



