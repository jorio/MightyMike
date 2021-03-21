/****************************/
/*    	ENEMY_FLOWERCLOWN   */
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
#include "enemy2.h"
#include "objecttypes.h"
#include "collision.h"
#include "externs.h"

/****************************/
/*    CONSTANTS             */
/****************************/

#define	FLOWERCLOWN_MAX_SPEED	0x21000L
#define	FLOWERCLOWN_ACCEL		0x2000L
#define	FLOWERCLOWN_HEALTH				2
#define	FLOWERCLOWN_WORTH				1
#define FLOWERCLOWN_DAMAGE_THRESHOLD	2

#define STONE_WHEEL_SPEED	0x68000L

#define	SQUIRT_MAX_DIST		250
#define	SQUIRT_SPEED			0x80000L

enum
{
	SUB_WALK_RIGHT,
	SUB_WALK_LEFT,
	SUB_THROW_RIGHT,
	SUB_THROW_LEFT,
	SUB_SQUIRT_RIGHT,
	SUB_SQUIRT_LEFT
};

/**********************/
/*     VARIABLES      */
/**********************/

#define ThrowFlag 	Flag0


/************************ ADD ENEMY: FLOWERCLOWN ********************/

Boolean AddEnemy_FlowerClown(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	if (gNumEnemies >= MAX_ENEMIES)				// check # enemies
		return(false);

	newObj = MakeNewShape(GroupNum_FlowerClown,ObjType_FlowerClown,SUB_WALK_RIGHT,itemPtr->x,
						itemPtr->y,50,MoveFlowerClown,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;				// remember where this came from
	newObj->CType = CTYPE_ENEMYA;				// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = FLOWERCLOWN_HEALTH;		// set health
	newObj->TopOff = -22;						// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -14;
	newObj->RightOff = 14;
	CalcObjectBox2(newObj);
	newObj->Worth = FLOWERCLOWN_WORTH;			// set worth
	newObj->InjuryThreshold = FLOWERCLOWN_DAMAGE_THRESHOLD;

	CalcEnemyScatterOffset(newObj);
	newObj->ThrowFlag = false;


	gNumEnemies++;

	return(true);								// was added
}


/********************* MOVE FLOWERCLOWN *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveFlowerClown(void)
{
static	void(*moveTable[])(void) =
				{
					MoveFlowerClown_Walk,			// walk right
					MoveFlowerClown_Walk,			// walk left
					MoveFlowerClown_Squirt,			// squirt right
					MoveFlowerClown_Squirt			// squirt left
				};

	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	GetObjectInfo();
	moveTable[gThisNodePtr->SubType]();
}


/********************* MOVE FLOWERCLOWN: WALK *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveFlowerClown_Walk(void)
{
short		distX,distY;

	if (TrackEnemy())									// see if out of range
		return;

	GetObjectInfo();

	DoFlowerClownMove();

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	distX = Absolute(gX.Int - gMyX);					// see if in range
	if (distX < SQUIRT_MAX_DIST)
	{
		distY = Absolute(gY.Int - gMyY);
		if (distY < SQUIRT_MAX_DIST)
		{
			if (!(MyRandomLong()&0b1111111))
			{
				if (gThisNodePtr->SubType == SUB_WALK_RIGHT)
					SwitchAnim(gThisNodePtr,SUB_THROW_RIGHT);
				else
					SwitchAnim(gThisNodePtr,SUB_THROW_LEFT);
				gDY = gDX = 0;
				UpdateEnemy();
				return;
			}
		}
	}

				/* SEE IF START TO THROW */

	UpdateFlowerClown();
}


/********************* MOVE FLOWERCLOWN: SQUIRT  *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//
// This is called WHILE it is throwing

void MoveFlowerClown_Squirt(void)
{
	if (TrackEnemy())									// see if out of range
		return;

	GetObjectInfo();

	if (gThisNodePtr->ThrowFlag)
	{
		gThisNodePtr->ThrowFlag = false;
		ThrowFlowerClownSquirt();
	}

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	UpdateEnemy();
}


/**************** UPDATE FLOWERCLOWN *******************/

void UpdateFlowerClown(void)
{
	if (!(MyRandomLong() & 0b1111111))							// see if recalc scatter
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

	UpdateEnemy();
}


/************** DO FLOWERCLOWN MOVE ******************/

void DoFlowerClownMove(void)
{

				/* ACCEL TOWARD ME */

	if (gX.Int < (gMyX+gThisNodePtr->EnemyTargetXOff))
		gDX += FLOWERCLOWN_ACCEL;
	else
	if (gX.Int > (gMyX+gThisNodePtr->EnemyTargetXOff))
		gDX -= FLOWERCLOWN_ACCEL;

	if (gY.Int > (gMyY+gThisNodePtr->EnemyTargetYOff))
		gDY -= FLOWERCLOWN_ACCEL;
	else
	if (gY.Int < (gMyY+gThisNodePtr->EnemyTargetYOff))
		gDY += FLOWERCLOWN_ACCEL;

				/* CHECK MAX DELTAS */

	if (gDX > FLOWERCLOWN_MAX_SPEED)
		gDX -= FLOWERCLOWN_ACCEL;
	else
	if (gDX < -FLOWERCLOWN_MAX_SPEED)
		gDX +=  FLOWERCLOWN_ACCEL;

	if (gDY > FLOWERCLOWN_MAX_SPEED)
		gDY -=  FLOWERCLOWN_ACCEL;
	else
	if (gDY < -FLOWERCLOWN_MAX_SPEED)
		gDY +=  FLOWERCLOWN_ACCEL;

					/* MOVE IT */
	gX.L += gDX;
	gY.L += gDY;
}

/*=========================================== SQUIRT ================================================*/

/*********** THROW A SQUIRT ***************/
//
//

void ThrowFlowerClownSquirt(void)
{
register	ObjNode		*newObj;
short			x,sub;
long		dx;

				/* GET INFO */

	if (gThisNodePtr->SubType == SUB_THROW_RIGHT)
	{
		x = gThisNodePtr->X.Int + 30;				// throw right
		dx = SQUIRT_SPEED;
		sub = SUB_SQUIRT_RIGHT;
	}
	else
	{
		x = gThisNodePtr->X.Int - 30;				// throw left
		dx = -SQUIRT_SPEED;
		sub = SUB_SQUIRT_LEFT;
	}

				/* CREATE SQUIRT */

	newObj = MakeNewShape(GroupNum_FlowerClown,ObjType_FlowerClown,sub,x,
						gThisNodePtr->Y.Int,gThisNodePtr->Z,MoveFlowerClownSquirt,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return;

	newObj->DX = dx;
	newObj->DY = (long)(gMyY - gY.Int) * 3000L;

	newObj->CType = CTYPE_ENEMYB;				// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->TopOff = -15;						// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -15;
	newObj->RightOff = 15;
	CalcObjectBox2(newObj);

	InitYOffset(newObj, -40);


			/* MAKE SHADOW */

	newObj->ShadowIndex = MakeShadow(newObj,SHADOWSIZE_SMALL);
}


/****************** MOVE SQUIRT *********************/
//
// gThisNodePtr = ptr to caveman doing the rolling
//

void MoveFlowerClownSquirt(void)
{

	if (TrackItem())							// see if out of range
	{
		DeleteObject(gThisNodePtr);
		return;
	}

	GetObjectInfo();

	gX.L += gDX;								// move it
	gY.L += gDY;

	gSumDX = gDX;								// see if hit wall
	gSumDY = gDY;
	CalcObjectBox();
	if (HandleCollisions(CTYPE_MISC) || (GetMapTileAttribs(gX.Int,gY.Int)&ALL_SOLID_SIDES))
	{
		DeleteObject(gThisNodePtr);
		return;
	}

	UpdateObject();
}



