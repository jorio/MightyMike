/****************************/
/*    	ENEMY_WITCH       	*/
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
#include "sound2.h"
#include "externs.h"

/****************************/
/*    CONSTANTS             */
/****************************/

#define	WITCH_MAX_SPEED			0x70000L
#define	WITCH_ACCEL				0x3000L
#define	WITCH_HEALTH			2
#define	WITCH_WORTH				3
#define	WITCH_DAMAGE_THRESHOLD 	1

enum
{
	SUB_WALK_RIGHT,
	SUB_WALK_LEFT,
	SUB_HAHA
};


/**********************/
/*     VARIABLES      */
/**********************/

long	gLastWitchHahaTime;


/************************ ADD ENEMY: WITCH ********************/

Boolean AddEnemy_Witch(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;


	if (gNumEnemies >= MAX_ENEMIES)			// check # enemies
		return(false);

	newObj = MakeNewShape(GroupNum_Witch,ObjType_Witch,SUB_WALK_RIGHT,itemPtr->x,
						itemPtr->y,50,MoveWitch,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	CalcEnemyScatterOffset(newObj);

	newObj->ItemIndex = itemPtr;					// remember where this came from
	newObj->CType = CTYPE_ENEMYA;					// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = WITCH_HEALTH;					// set health

	newObj->TopOff = -22;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -20;
	newObj->RightOff = 20;
	CalcObjectBox2(newObj);

	newObj->Worth = WITCH_WORTH;						// set worth
	newObj->InjuryThreshold = WITCH_DAMAGE_THRESHOLD;

	newObj->YOffset.Int = -20;

	newObj->ShadowIndex = MakeShadow(newObj,SHADOWSIZE_GIANT);		// allocate shadow


	gNumEnemies++;

	return(true);									// was added
}


/********************* MOVE WITCH *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveWitch(void)
{
	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	if (TrackEnemy())									// see if out of range
		return;

	DoWitchHaha();

	GetObjectInfo();

	DoWitchMove();

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	UpdateWitch();
}



/**************** UPDATE WITCH *******************/

void UpdateWitch(void)
{
	if (!(MyRandomLong() & 0b1111111))							// see if recalc scatter
		CalcEnemyScatterOffset(gThisNodePtr);

	gThisNodePtr->AnimSpeed = (Absolute(gDX)+Absolute(gDY))>>8;

	UpdateEnemy();
}


/************** DO WITCH MOVE ******************/

void DoWitchMove(void)
{
long	xAcc,yAcc;

				/* ACCEL TOWARD ME */

	if (gX.Int < (gMyX+gThisNodePtr->EnemyTargetXOff))
		xAcc = WITCH_ACCEL;										// accel right
	else
	if (gX.Int > (gMyX+gThisNodePtr->EnemyTargetXOff))
		xAcc = -WITCH_ACCEL;										// accel left
	else
		xAcc = 0;

	if (gY.Int > (gMyY+gThisNodePtr->EnemyTargetYOff))
		yAcc = -WITCH_ACCEL;										// accel up
	else
	if (gY.Int < (gMyY+gThisNodePtr->EnemyTargetYOff))
		yAcc = WITCH_ACCEL;										// accel down
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

	if (gDX > WITCH_MAX_SPEED)
		gDX -= WITCH_ACCEL;
	else
	if (gDX < -WITCH_MAX_SPEED)
		gDX +=  WITCH_ACCEL;

	if (gDY > WITCH_MAX_SPEED)
		gDY -=  WITCH_ACCEL;
	else
	if (gDY < -WITCH_MAX_SPEED)
		gDY +=  WITCH_ACCEL;

					/* MOVE IT */
	gX.L += gDX;
	gY.L += gDY;

}


/**************** DO WITH HAHA *****************/
//
//
//

void DoWitchHaha(void)
{
register	ObjNode *newObj;

	if ((gFrames-gLastWitchHahaTime) < GAME_FPS)		// see if been enough time
		return;

	if (gThisNodePtr->OwnerToMessageNode != nil)		// see if already has a message
		return;
	if (gMyNodePtr->OwnerToMessageNode != nil)			// not if Mike is talking
		return;
	if (MyRandomLong()&0b111111111)						// random
		return;

				/* MAKE MESSAGE SPRITE */

	newObj = MakeNewShape(GroupNum_Witch,ObjType_Witch,SUB_HAHA,
				gThisNodePtr->X.Int,gThisNodePtr->Y.Int,gThisNodePtr->Z,MoveMessage,
				PLAYFIELD_RELATIVE);

	if (newObj == nil)
		return;

	newObj->MessageTimer = GAME_FPS*2;					// set message timer
	newObj->TileMaskFlag = false;						// wont be tile masked
	newObj->MessageToOwnerNode = gThisNodePtr;			// point to owner
	gThisNodePtr->OwnerToMessageNode = newObj;			// point to message

	PlaySound(gSoundNum_WitchHaha);

	gLastWitchHahaTime = gFrames;						// remember when it occurred
}


