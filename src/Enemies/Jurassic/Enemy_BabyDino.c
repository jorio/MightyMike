/****************************/
/*    	ENEMY_BABYDINO      */
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
#include "sound2.h"
#include "objecttypes.h"
#include "shape.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY;
extern	union_gX;
extern	union_gY;
extern	CollisionRec	gCollisionList[];
extern	short			gNumCollisions;
extern	long				gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	long				gMyX,gMyY;
extern	short				gNumEnemies;
extern	short				gSoundNum_DinoBoom,gSoundNum_BarneyJump;
extern	short			gEnemyFreezeTimer;

/****************************/
/*    CONSTANTS             */
/****************************/

enum{
		BABYDINO_SUB_STAND,
		BABYDINO_SUB_HOP,
		BABYDINO_SUB_LAND
};

#define	BABYDINO_HEALTH			6
#define BABYDINO_WORTH			4
#define	BABYDINO_DAMAGE_THRESHOLD 3					// min weapon power needed to do damage

#define STAND_TIME		(GAME_FPS*2)

/**********************/
/*     VARIABLES      */
/**********************/

#define StandCount		Special1		// counter for enemy standing b4 hop


/************************ ADD ENEMY: BABYDINO ********************/

Boolean AddEnemy_BabyDino(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	if (gNumEnemies >= MAX_ENEMIES)					// check # enemies
		return(false);


	newObj = MakeNewShape(GroupNum_BabyDino,ObjType_BabyDino,BABYDINO_SUB_STAND,
			itemPtr->x,itemPtr->y,50,MoveBabyDino,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CType = CTYPE_ENEMYA;					// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = BABYDINO_HEALTH;				// set health

	newObj->TopOff = -22;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -30;
	newObj->RightOff = 30;
	CalcObjectBox2(newObj);

	newObj->Worth = BABYDINO_WORTH;					// set worth
	newObj->StandCount = STAND_TIME;				// set stand duration
	newObj->InjuryThreshold = BABYDINO_DAMAGE_THRESHOLD;

			/* MAKE SHADOW */

	newObj->ShadowIndex = MakeShadow(newObj,SHADOWSIZE_GIANT); 	// remember ptr to shadow


	gNumEnemies++;
	return(true);									// was added
}


/********************* MOVE BABYDINO *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveBabyDino(void)
{
static	void(*moveTable[])(void) =
				{
					MoveBaby_Stand,				// stand
					MoveBaby_Hop,				// hop
					MoveBaby_Land				// land
				};

	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	GetObjectInfo();
	moveTable[gThisNodePtr->SubType]();
}


/**************** MOVE BABY: STAND ********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveBaby_Stand(void)
{
register	ObjNode *theNode;

	if (TrackEnemy())									// see if out of range
		return;

	theNode = gThisNodePtr;

				/* DO ENEMY COLLISION */

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

				/* SEE IF HOP */

	if (--theNode->StandCount < 0)
	{
		theNode->StandCount = STAND_TIME;				// reset stand timer
		SwitchAnim(theNode,BABYDINO_SUB_HOP);
		theNode->DZ = -0x1e0000L;						// start bouncing up

		gDX = (long)(gMyX - gX.Int)*0x600L;				// go toward me
		gDY = (long)(gMyY - gY.Int)*0x600L;

		PlaySound(gSoundNum_BarneyJump);
	}

					/* UPDATE */
	UpdateEnemy();
}


/**************** MOVE BABY: HOP ********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveBaby_Hop(void)
{
register	ObjNode *theNode;
long		ctype;

	if (TrackEnemy())									// see if out of range
		return;

	theNode = gThisNodePtr;

	gX.L += gDX;										// move it
	gY.L += gDY;

				/* DO VERTICAL FALL */

	theNode->DZ += 0x18000L;						// add gravity
	theNode->YOffset.L += theNode->DZ;				// move it

	if (theNode->YOffset.Int >= 0)					// see if landed
	{
		SwitchAnim(theNode,BABYDINO_SUB_LAND);
		StartShakeyScreen(GAME_FPS/2);
		PlaySound(gSoundNum_DinoBoom);				// play sound
		theNode->DY = theNode->DX = 0;
	}

	if (theNode->YOffset.Int >= -40)				// see if close enough for collision
	{
		theNode->CType = CTYPE_ENEMYA;
		ctype = FULL_ENEMY_COLLISION;
	}
	else
	{
		theNode->CType = 0;
		ctype = ENEMY_NO_BULLET_COLLISION;
	}

					/* DO COLLISION */

	if (DoEnemyCollisionDetect(ctype))				// returns true if died
		return;

					/* UPDATE */

	UpdateEnemy();
}


/**************** MOVE BABY: LAND ********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveBaby_Land(void)
{
	if (TrackEnemy())									// see if out of range
		return;

					/* DO COLLISION */

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

					/* UPDATE */

	UpdateEnemy();
}





