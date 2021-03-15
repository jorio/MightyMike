/****************************/
/*    	ENEMY_BATTERY   	*/
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
#include "enemy5.h"
#include "shape.h"
#include "object.h"
#include "misc.h"
#include "objecttypes.h"
#include "collision.h"
#include "externs.h"

/****************************/
/*    CONSTANTS             */
/****************************/

#define	BATTERY_HEALTH			7
#define BATTERY_WORTH				10
#define	BATTERY_DAMAGE_THRESHOLD	3

#define	CHARGING_RANGE_X		300
#define	CHARGING_RANGE_Y		90
#define	BATTERY_SPEED			0x30000L

/**********************/
/*     VARIABLES      */
/**********************/


/************************ ADD ENEMY: BATTERY ********************/
//
// parm[0] = aim direction 0..7, or 8 = best pick
//

Boolean AddEnemy_Battery(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;
short			animNum;
static	short animList[] = {5,1,7,2,4,0,6,3};
static	long dxList[] = {0,BATTERY_SPEED,BATTERY_SPEED,BATTERY_SPEED,0
					,-BATTERY_SPEED,-BATTERY_SPEED,-BATTERY_SPEED};
static	long dyList[] = {-BATTERY_SPEED,-BATTERY_SPEED,0,BATTERY_SPEED,
					BATTERY_SPEED,BATTERY_SPEED,0,-BATTERY_SPEED};

	if (gNumEnemies >= MAX_ENEMIES)					// check # enemies
		return(false);



	if ((animNum = itemPtr->parm[0]) == 8)			// see if best guess @ aim
		animNum = GuessBatteryAnim(itemPtr->x,itemPtr->y);

	newObj = MakeNewShape(GroupNum_BadBattery,ObjType_BadBattery,animList[animNum],
			itemPtr->x,itemPtr->y,50,MoveBattery,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CType = CTYPE_ENEMYA;					// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = BATTERY_HEALTH;				// set health

	newObj->TopOff = -20;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -20;
	newObj->RightOff = 20;
	CalcObjectBox2(newObj);

	newObj->Worth = BATTERY_WORTH;						// set worth
	newObj->InjuryThreshold = BATTERY_DAMAGE_THRESHOLD;

	newObj->DX = dxList[animNum];
	newObj->DY = dyList[animNum];

	gNumEnemies++;

	return(true);									// was added
}


/********************* MOVE BATTERY *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveBattery(void)
{

	if (gEnemyFreezeTimer)								// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	if (TrackEnemy())									// see if out of range
		return;

	GetObjectInfo();

	gX.L += gDX;										// do the move
	gY.L += gDY;

	if (DoPointCollision(gX.Int,gY.Int,CTYPE_BGROUND|CTYPE_MISC))	// see if hit something
	{
		DeleteEnemy(gThisNodePtr);
		return;
	}

					/* UPDATE */

	UpdateEnemy();
}


/***************** GUESS BATTERY ANIM *************************/

short	GuessBatteryAnim(short x, short y)
{
short	diffX,diffY;

	diffX = gMyX - x;
	diffY = gMyY - y;

	if (Absolute(diffX) < 50)							// see if go vertical
	{
		if (diffY < 0)
			return(0);									// go up
		else
			return(4);									// go down
	}

	if (Absolute(diffY) < 50)							// see if go horiz
	{
		if (diffX < 0)
			return(6);									// go left
		else
			return(2);									// go right
	}

	if (diffX < 0)
	{
		if (diffY < gMyY)
			return(7);									// go u/l
		else
			return(5);									// go d/l
	}
	else
	{
		if (diffY < gMyY)
			return(1);									// go u/r
		else
			return(3);									// go d/r
	}
}


