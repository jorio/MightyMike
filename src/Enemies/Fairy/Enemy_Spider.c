/****************************/
/*    	ENEMY_SPIDER         */
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
#include "miscanims.h"
#include "objecttypes.h"
#include "shape.h"
#include "enemy4.h"
#include "weapon.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY;
extern	union_gX;
extern	union_gY;
extern	CollisionRec	gCollisionList[];
extern	short			gNumCollisions;
extern	long				gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	long				gMyX,gMyY;
extern	short				gNumEnemies;
extern	short			gEnemyFreezeTimer;

/****************************/
/*    CONSTANTS             */
/****************************/

enum{
		SPIDER_SUB_FALL,
		SPIDER_SUB_CRAWL
};

#define	SPIDER_HEALTH			2
#define SPIDER_WORTH			1
#define	SPIDER_DAMAGE_THRESHOLD 1					// min weapon power needed to do damage

#define	SPIDER_MAX_SPEED		0x30000L

/**********************/
/*     VARIABLES      */
/**********************/

/************************ ADD ENEMY: SPIDER ********************/

Boolean AddEnemy_Spider(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	if (gNumEnemies >= (MAX_ENEMIES*2))				// check # enemies
		return(false);

	newObj = MakeNewShape(GroupNum_Spider,ObjType_Spider,SPIDER_SUB_CRAWL,
			itemPtr->x,itemPtr->y,50,MoveSpider,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = SPIDER_HEALTH;					// set health
	newObj->CType = CTYPE_ENEMYA;					// set collision info

	newObj->TopOff = -20;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -20;
	newObj->RightOff = 20;
	CalcObjectBox2(newObj);

	newObj->Worth = SPIDER_WORTH;					// set worth
	newObj->InjuryThreshold = SPIDER_DAMAGE_THRESHOLD;

	gNumEnemies++;
	return(true);									// was added
}


/********************* MOVE SPIDER *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveSpider(void)
{
short	dist,anim;

	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	GetObjectInfo();

	if (TrackEnemy())										// see if out of range
		return;

				/* ACCEL TOWARD ME */

	anim = gThisNodePtr->SubType;

	dist = gMyX+gThisNodePtr->EnemyTargetXOff;
	if (Absolute(gX.Int - dist) > 10)						// only change direction if far
	{
		if (Absolute(gX.Int < dist) )
		{
			gDX = SPIDER_MAX_SPEED;							// go right
			gDY = 0;
			anim = 1;
		}
		else
		{
			gDX = -SPIDER_MAX_SPEED;						// go left
			gDY = 0;
			anim = 3;
		}
	}

	dist = (gMyY+gThisNodePtr->EnemyTargetYOff);
	if (Absolute(gY.Int - dist) > 10)						// only change direction if far
	{
		if (gY.Int > dist)
		{
			gDY = -SPIDER_MAX_SPEED;						// go up
			gDX = 0;
			anim = 2;
		}
		else
		{
			gDY = SPIDER_MAX_SPEED;							// go down
			gDX = 0;
			anim = 0;
		}
	}

	if (gThisNodePtr->SubType != anim)						// set correct anim
		SwitchAnim(gThisNodePtr,anim);


					/* MOVE IT */
	gX.L += gDX;
	gY.L += gDY;

			/* COLLISION */

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))		// returns true if died
		return;

	if (!(MyRandomLong() & b111111))						// see if recalc scatter
		CalcEnemyScatterOffset(gThisNodePtr);

	UpdateEnemy();
}

