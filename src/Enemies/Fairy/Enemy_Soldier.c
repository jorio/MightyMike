/****************************/
/*    	ENEMY_SOLDIER       */
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
#include "externs.h"

/****************************/
/*    CONSTANTS             */
/****************************/

#define	SOLDIER_MAX_SPEED	0x20000L
#define	SOLDIER_HEALTH		3
#define	SOLDIER_WORTH		1
#define	SOLDIER_DAMAGE_THRESHOLD 1

enum
{
	SUB_WALK_RIGHT,
	SUB_WALK_LEFT,
	SUB_WALK_UP,
	SUB_WALK_DOWN
};


/**********************/
/*     VARIABLES      */
/**********************/


/************************ ADD ENEMY: SOLDIER ********************/

Boolean AddEnemy_Soldier(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;


	if (gNumEnemies >= MAX_ENEMIES)			// check # enemies
		return(false);

	newObj = MakeNewShape(GroupNum_Soldier,ObjType_Soldier,SUB_WALK_RIGHT,itemPtr->x,
						itemPtr->y,50,MoveSoldier,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	CalcEnemyScatterOffset(newObj);

	newObj->ItemIndex = itemPtr;					// remember where this came from
	newObj->CType = CTYPE_ENEMYA;					// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = SOLDIER_HEALTH;				// set health
	newObj->TopOff = -30;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -20;
	newObj->RightOff = 20;
	CalcObjectBox2(newObj);
	newObj->Worth = SOLDIER_WORTH;					// set worth
	newObj->InjuryThreshold = SOLDIER_DAMAGE_THRESHOLD;

	gNumEnemies++;

	return(true);									// was added
}


/********************* MOVE SOLDIER *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveSoldier(void)
{
	if (gEnemyFreezeTimer)								// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	if (TrackEnemy())									// see if out of range
		return;

	GetObjectInfo();

	DoSoldierMove();

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	UpdateEnemy();
}




/************** DO SOLDIER MOVE ******************/

void DoSoldierMove(void)
{
short	dist;

				/* ACCEL TOWARD ME */

	dist = gMyX+gThisNodePtr->EnemyTargetXOff;
	if (Absolute(gX.Int - dist) > 5)								// only change direction if far
	{
		if (Absolute(gX.Int < dist) )
		{
			gDX = SOLDIER_MAX_SPEED;									// go right
			gDY = 0;
		}
		else
		{
			gDX = -SOLDIER_MAX_SPEED;									// go left
			gDY = 0;
		}
	}

	dist = (gMyY+gThisNodePtr->EnemyTargetYOff);
	if (Absolute(gY.Int - dist) > 5)								// only change direction if far
	{
		if (gY.Int > dist)
		{
			gDY = -SOLDIER_MAX_SPEED;									// go up
			gDX = 0;
		}
		else
		{
			gDY = SOLDIER_MAX_SPEED;									// go down
			gDX = 0;
		}
	}

			/* SET CORRECT ANIMATION */


	if (gDX < 0)
	{
		if (gThisNodePtr->SubType != SUB_WALK_LEFT)			// left anim
			SwitchAnim(gThisNodePtr,SUB_WALK_LEFT);
	}
	else
	if (gDX > 0)
	{
		if (gThisNodePtr->SubType != SUB_WALK_RIGHT)		// right anim
			SwitchAnim(gThisNodePtr,SUB_WALK_RIGHT);
	}
	else
	if (gDY < 0)
	{
		if (gThisNodePtr->SubType != SUB_WALK_UP)			// UP anim
			SwitchAnim(gThisNodePtr,SUB_WALK_UP);
	}
	else
	if (gDY > 0)
	{
		if (gThisNodePtr->SubType != SUB_WALK_DOWN)			// down anim
			SwitchAnim(gThisNodePtr,SUB_WALK_DOWN);
	}


					/* MOVE IT */
	gX.L += gDX;
	gY.L += gDY;

}



