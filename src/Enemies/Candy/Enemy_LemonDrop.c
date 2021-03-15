/****************************/
/*    	ENEMY_LEMONDROP        */
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
#include "misc.h"
#include "shape.h"
#include "miscanims.h"
#include "enemy3.h"
#include "objecttypes.h"
#include "collision.h"
#include "externs.h"

/****************************/
/*    CONSTANTS             */
/****************************/

#define	LEMONDROP_SPEED				0x20000L
#define	LEMONDROP_HEALTH			7
#define	LEMONDROP_WORTH				3
#define LEMONDROP_DAMAGE_THRESHOLD	1

#define	DRIP_SPEED		0x50000L			// speed of moving driplet

/**********************/
/*     VARIABLES      */
/**********************/

#define	OldHealth		Special1


/************************ ADD ENEMY: LEMONDROP********************/

Boolean AddEnemy_LemonDrop(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;


	if (gNumEnemies >= MAX_ENEMIES)				// check # enemies
		return(false);

	newObj = MakeNewShape(GroupNum_LemonDrop,ObjType_LemonDrop,0,itemPtr->x,
						itemPtr->y,50,MoveLemon,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;				// remember where this came from
	newObj->CType = CTYPE_ENEMYA;				// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->TopOff = -8;						// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -8;
	newObj->RightOff = 8;
	CalcObjectBox2(newObj);

	newObj->Worth = LEMONDROP_WORTH;				// set worth
	newObj->InjuryThreshold = LEMONDROP_DAMAGE_THRESHOLD;

	if (itemPtr->x < gMyX)							// set random deltas
		newObj->DX = LEMONDROP_SPEED;
	else
		newObj->DX = -LEMONDROP_SPEED;

	if (itemPtr->y < gMyY)
		newObj->DY = LEMONDROP_SPEED;
	else
		newObj->DY = -LEMONDROP_SPEED;


	newObj->Health = newObj->OldHealth = LEMONDROP_HEALTH;		// set health

	gNumEnemies++;

	return(true);								// was added
}


/********************* MOVE LEMON*********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveLemon(void)
{
	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	if (TrackEnemy2())									// see if out of range
		return;

	GetObjectInfo();

					/* MOVE IT */
	gX.L += gDX;
	gY.L += gDY;

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	if (gTotalSides & (SIDE_BITS_TOP|SIDE_BITS_BOTTOM))		// see if bounce
		gDY = -gDY;

	if (gTotalSides & (SIDE_BITS_LEFT|SIDE_BITS_RIGHT))
		gDX = -gDX;

	if (gThisNodePtr->OldHealth != gThisNodePtr->Health)		// see if got hit
	{
		gThisNodePtr->OldHealth = gThisNodePtr->Health;
		SqueezeLemon();
	}

	UpdateEnemy();
}

/****************** SQUEEZE LEMON *********************/

void SqueezeLemon(void)
{
static	long lemonDX[8] = {0,DRIP_SPEED,DRIP_SPEED,DRIP_SPEED,0,-DRIP_SPEED,-DRIP_SPEED,-DRIP_SPEED};
static	long lemonDY[8] = {-DRIP_SPEED,-DRIP_SPEED,0,DRIP_SPEED,DRIP_SPEED,DRIP_SPEED,0,-DRIP_SPEED};
register	short	i;
register	ObjNode	*newObj;

	for (i=0; i < 8; i++)
	{
		newObj = MakeNewShape(GroupNum_LemonDrop,ObjType_LemonDrop,1+i,gX.Int,
							gY.Int,gThisNodePtr->Z,MoveLemonDrop,PLAYFIELD_RELATIVE);
		if (newObj == nil)
			return;

		newObj->CType = CTYPE_ENEMYB;				// set collision info
		newObj->CBits = CBITS_TOUCHABLE;
		newObj->TopOff = -8;						// set box
		newObj->BottomOff = 0;
		newObj->LeftOff = -8;
		newObj->RightOff = 8;

		newObj->DX = lemonDX[i];
		newObj->DY = lemonDY[i];

	}
}


/**************** MOVE LEMON DROP ******************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveLemonDrop(void)
{
	if (TrackItem())									// see if out of range
	{
		DeleteObject(gThisNodePtr);
		return;
	}

	GetObjectInfo();

	gX.L += gDX;
	gY.L += gDY;

	if (DoPointCollision(gX.Int,gY.Int,CTYPE_BGROUND|CTYPE_MISC))	// see if hit something
	{
		DeleteObject(gThisNodePtr);
		return;
	}

	CalcObjectBox();
	UpdateObject();
}

