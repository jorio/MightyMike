/****************************/
/*    	ENEMY_CBALLOON     	*/
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
#include "enemy2.h"
#include "objecttypes.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY;
extern	union_gX;
extern	union_gY;
extern	CollisionRec	gCollisionList[];
extern	short				gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	short				gMyX,gMyY;
extern	short				gNumEnemies;
extern	short			gEnemyFreezeTimer;

/****************************/
/*    CONSTANTS             */
/****************************/

#define	CBALLOON_MAX_SPEED			0x10000L
#define	CBALLOON_ACCEL				0x120L
#define	CBALLOON_HEALTH				1
#define	CBALLOON_WORTH				5
#define	CBALLOON_DAMAGE_THRESHOLD	1

#define	BOMB_DROP_DIST				100

enum
{
	SUB_FLOAT,
	SUB_THROW,
	SUB_BOMBFALL
};


/**********************/
/*     VARIABLES      */
/**********************/

#define	DropFlag	Flag0


/************************ ADD ENEMY: CBALLOON ********************/

Boolean AddEnemy_ClownBalloon(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;


	if (gNumEnemies >= MAX_ENEMIES)			// check # enemies
		return(FALSE);

	newObj = MakeNewShape(GroupNum_ClownBalloon,ObjType_ClownBalloon,SUB_FLOAT,itemPtr->x,
						itemPtr->y,50,MoveCBalloon,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(FALSE);

	CalcEnemyScatterOffset(newObj);

	newObj->ItemIndex = itemPtr;				// remember where this came from
	newObj->CType = CTYPE_ENEMYA;				// set collision info
	newObj->CBits = 0;
	newObj->Health = CBALLOON_HEALTH;			// set health
	newObj->TopOff = -22;						// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -20;
	newObj->RightOff = 20;
	CalcObjectBox2(newObj);

	newObj->Worth = CBALLOON_WORTH;				// set worth
	newObj->InjuryThreshold = CBALLOON_DAMAGE_THRESHOLD;

	newObj->YOffset.Int = -140;

	newObj->DropFlag = FALSE;

			/* MAKE SHADOW */

	newObj->ShadowIndex = MakeShadow(newObj,SHADOWSIZE_LARGE); 	// remember ptr to shadow


	gNumEnemies++;

	return(TRUE);								// was added
}


/********************* MOVE CBALLOON *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveCBalloon(void)
{
	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	if (TrackEnemy())									// see if out of range
		return;

	GetObjectInfo();

	DoCBalloonMove();


				/* SEE IF START 2 THROW */

	if (gThisNodePtr->SubType == SUB_FLOAT)
	{
		if ((Absolute(gX.Int - gMyX) < BOMB_DROP_DIST) &&
			((Absolute(gY.Int - gMyY) < BOMB_DROP_DIST)))
		{
			if (!(MyRandomLong() & b1111))
				SwitchAnim(gThisNodePtr,SUB_THROW);
		}
	}

				/* SEE IF READY 2 DROP BOMB */

	if (gThisNodePtr->DropFlag)
	{
		CBalloonDropBomb();
		gThisNodePtr->DropFlag = FALSE;
	}

				/* DO COLLISION DETECT */

	if (DoEnemyCollisionDetect(CTYPE_BGROUND))		// returns TRUE if died
		return;

	UpdateCBalloon();
}


/**************** UPDATE CBALLOON *******************/

void UpdateCBalloon(void)
{
	if (!(MyRandomLong() & b1111111))							// see if recalc scatter
		CalcEnemyScatterOffset(gThisNodePtr);

	UpdateEnemy();
}


/************** DO CBALLOON MOVE ******************/

void DoCBalloonMove(void)
{

				/* ACCEL TOWARD ME */

	if (gX.Int < (gMyX+gThisNodePtr->EnemyTargetXOff))
		gDX += CBALLOON_ACCEL;
	else
	if (gX.Int > (gMyX+gThisNodePtr->EnemyTargetXOff))
		gDX -= CBALLOON_ACCEL;

	if (gY.Int > (gMyY+gThisNodePtr->EnemyTargetYOff))
		gDY -= CBALLOON_ACCEL;
	else
	if (gY.Int < (gMyY+gThisNodePtr->EnemyTargetYOff))
		gDY += CBALLOON_ACCEL;

				/* CHECK MAX DELTAS */

	if (gDX > CBALLOON_MAX_SPEED)
		gDX -= CBALLOON_ACCEL;
	else
	if (gDX < -CBALLOON_MAX_SPEED)
		gDX +=  CBALLOON_ACCEL;

	if (gDY > CBALLOON_MAX_SPEED)
		gDY -=  CBALLOON_ACCEL;
	else
	if (gDY < -CBALLOON_MAX_SPEED)
		gDY +=  CBALLOON_ACCEL;

					/* MOVE IT */
	gX.L += gDX;
	gY.L += gDY;

}


/****************** DROP BOMB *******************/

void CBalloonDropBomb(void)
{
register	ObjNode		*newObj;

				/* MAKE BOMB SHAPE */

	newObj = MakeNewShape(GroupNum_ClownBalloon,ObjType_ClownBalloon,SUB_BOMBFALL,gThisNodePtr->X.Int,
						gThisNodePtr->Y.Int,gThisNodePtr->Z,MoveCBalloonBomb,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return;

	newObj->CType = CTYPE_ENEMYB;				// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->TopOff = -8;						// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -8;
	newObj->RightOff = 8;
	CalcObjectBox2(newObj);
	newObj->YOffset.Int = gThisNodePtr->YOffset.Int;
	newObj->ShadowIndex = MakeShadow(newObj,SHADOWSIZE_SMALL);	// allocate shadow

	newObj->DZ = -0x40000L;

	newObj->DX = (long)MyRandomLong()&0x1ffffL-0xffffL;
}


/************** MOVE BOMB *****************/

void MoveCBalloonBomb(void)
{
register	ObjNode *theNode;

	theNode = gThisNodePtr;

	theNode->X.L += theNode->DX;

	theNode->DZ += 0x10000L;							// add gravity
	theNode->YOffset.L += theNode->DZ;					// move it

	if (theNode->YOffset.Int >= 0)						// see if landed
	{
		DeleteObject(gThisNodePtr);
	}

}
