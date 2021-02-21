/****************************/
/*    	ENEMY_GBREAD        */
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
#include "shape.h"
#include "miscanims.h"
#include "enemy3.h"
#include "objecttypes.h"
#include "collision.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY;
extern	union_gX;
extern	union_gY;
extern	CollisionRec	gCollisionList[];
extern	long				gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	long				gMyX,gMyY;
extern	short				gNumEnemies;
extern	ObjNode			*gMyNodePtr;
extern	short			gEnemyFreezeTimer;


/****************************/
/*    CONSTANTS             */
/****************************/

#define	GBREAD_MAX_SPEED	0x35000L
#define	GBREAD_ACCEL		0x4000L
#define	GBREAD_HEALTH		7
#define	GBREAD_WORTH		2
#define GBREAD_DAMAGE_THRESHOLD	1

#define	POP_MAX_DIST		250
#define	POP_SPEED			0x80000L

enum
{
	SUB_WALK_RIGHT,
	SUB_BUTTON,
	SUB_WALK_LEFT,
	SUB_POP_RIGHT,
	SUB_POP_LEFT
};

/**********************/
/*     VARIABLES      */
/**********************/

#define PopFlag 	Flag0
#define	ButtonCount	Special1


/************************ ADD ENEMY: GBREAD********************/

Boolean AddEnemy_GBread(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	if (gNumEnemies >= MAX_ENEMIES)				// check # enemies
		return(false);

	newObj = MakeNewShape(GroupNum_GBread,ObjType_GBread,SUB_WALK_RIGHT,itemPtr->x,
						itemPtr->y,50,MoveGBread,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;				// remember where this came from
	newObj->CType = CTYPE_ENEMYA;				// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = GBREAD_HEALTH;				// set health
	newObj->TopOff = -22;						// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -14;
	newObj->RightOff = 14;
	CalcObjectBox2(newObj);
	newObj->Worth = GBREAD_WORTH;				// set worth
	newObj->InjuryThreshold = GBREAD_DAMAGE_THRESHOLD;

	CalcEnemyScatterOffset(newObj);

	newObj->PopFlag = false;
	newObj->ButtonCount = 0;


	gNumEnemies++;

	return(true);								// was added
}


/********************* MOVE GBREAD*********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveGBread(void)
{
static	void(*moveTable[])(void) =
				{
					MoveGBread_Walk,			// walk right
					nil,						// button
					MoveGBread_Walk,			// walk left
					MoveGBread_Pop,				// pop right
					MoveGBread_Pop				// pop left
				};

	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	GetObjectInfo();
	moveTable[gThisNodePtr->SubType]();
}


/********************* MOVE GBREAD: WALK *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveGBread_Walk(void)
{
short		distX,distY;

	if (TrackEnemy())									// see if out of range
		return;

	DoGBreadMove();

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	distX = Absolute(gX.Int - gMyX);					// see if in range
	if (distX < POP_MAX_DIST)
	{
		distY = Absolute(gY.Int - gMyY);
		if (distY < POP_MAX_DIST)
		{
			if (!(MyRandomLong()&b11111))
			{
				if (gThisNodePtr->SubType == SUB_WALK_RIGHT)
					SwitchAnim(gThisNodePtr,SUB_POP_RIGHT);
				else
					SwitchAnim(gThisNodePtr,SUB_POP_LEFT);
				gDY = gDX = 0;
				UpdateEnemy();
				return;
			}
		}
	}

	UpdateGBread();
}


/********************* MOVE GBREAD: POP  *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//
// This is called WHILE it is throwing
//

void MoveGBread_Pop(void)
{
	if (TrackEnemy())									// see if out of range
		return;

	if (gThisNodePtr->PopFlag)
	{
		gThisNodePtr->PopFlag = false;
		PopAButton();
	}

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	UpdateEnemy();
}


/**************** UPDATE GBREAD*******************/

void UpdateGBread(void)
{
	if (!(MyRandomLong() & b1111111))							// see if recalc scatter
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


/************** DO GBREADMOVE ******************/

void DoGBreadMove(void)
{

				/* ACCEL TOWARD ME */

	if (gX.Int < (gMyX+gThisNodePtr->EnemyTargetXOff))
		gDX += GBREAD_ACCEL;
	else
	if (gX.Int > (gMyX+gThisNodePtr->EnemyTargetXOff))
		gDX -= GBREAD_ACCEL;

	if (gY.Int > (gMyY+gThisNodePtr->EnemyTargetYOff))
		gDY -= GBREAD_ACCEL;
	else
	if (gY.Int < (gMyY+gThisNodePtr->EnemyTargetYOff))
		gDY += GBREAD_ACCEL;

				/* CHECK MAX DELTAS */

	if (gDX > GBREAD_MAX_SPEED)
		gDX -= GBREAD_ACCEL;
	else
	if (gDX < -GBREAD_MAX_SPEED)
		gDX +=  GBREAD_ACCEL;

	if (gDY > GBREAD_MAX_SPEED)
		gDY -=  GBREAD_ACCEL;
	else
	if (gDY < -GBREAD_MAX_SPEED)
		gDY +=  GBREAD_ACCEL;

					/* MOVE IT */
	gX.L += gDX;
	gY.L += gDY;
}

/*=========================================== POP BUTTON ================================================*/

/*********** POP A BUTTON ***************/
//
//

void PopAButton(void)
{
register	ObjNode		*newObj;
short			x;
long		dx;

				/* GET INFO */

	if (gThisNodePtr->SubType == SUB_POP_RIGHT)
	{
		x = gThisNodePtr->X.Int + 10;				// throw right
		dx = POP_SPEED;
	}
	else
	{
		x = gThisNodePtr->X.Int - 10;				// throw left
		dx = -POP_SPEED;
	}

				/* CREATE BUTTON */

	newObj = MakeNewShape(GroupNum_GBread,ObjType_GBread,SUB_BUTTON,x,
						gThisNodePtr->Y.Int+4,gThisNodePtr->Z,MoveButton,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return;

	newObj->DX = dx;
	newObj->DY = (long)(gMyY - gY.Int) * 3000L;

	newObj->CType = CTYPE_ENEMYB;				// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->TopOff = -8;						// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -8;
	newObj->RightOff = 8;
	CalcObjectBox2(newObj);

	newObj->YOffset.Int = -40-(gThisNodePtr->ButtonCount * 9);		// see which button
	if (++gThisNodePtr->ButtonCount >= 3)
		gThisNodePtr->ButtonCount = 0;

			/* MAKE SHADOW */

	newObj->ShadowIndex = MakeShadow(newObj,SHADOWSIZE_SMALL);	// allocate shadow

}


/****************** MOVE BUTTON *********************/
//
// gThisNodePtr = ptr to caveman doing the rolling
//

void MoveButton(void)
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


