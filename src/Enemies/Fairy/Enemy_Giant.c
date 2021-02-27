/****************************/
/*    	ENEMY_GIANT         */
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
#include "miscanims.h"
#include "sound2.h"
#include "objecttypes.h"
#include "shape.h"
#include "enemy4.h"
#include "weapon.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY;
extern	MikeFixed		gX;
extern	MikeFixed		gY;
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
		GIANT_SUB_STAND,
		GIANT_SUB_HOP,
		GIANT_SUB_LAND
};

#define	GIANT_HEALTH			6
#define GIANT_WORTH			4
#define	GIANT_DAMAGE_THRESHOLD 3					// min weapon power needed to do damage

#define STAND_TIME		(GAME_FPS*1)

/**********************/
/*     VARIABLES      */
/**********************/

#define StandCount		Special1		// counter for enemy standing b4 hop


/************************ ADD ENEMY: GIANT ********************/

Boolean AddEnemy_Giant(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	if (gNumEnemies >= MAX_ENEMIES)					// check # enemies
		return(false);


	newObj = MakeNewShape(GroupNum_Giant,ObjType_Giant,GIANT_SUB_STAND,
			itemPtr->x,itemPtr->y,50,MoveGiant,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CType = CTYPE_ENEMYA;					// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = GIANT_HEALTH;				// set health

	newObj->TopOff = -22;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -30;
	newObj->RightOff = 30;
	CalcObjectBox2(newObj);

	newObj->Worth = GIANT_WORTH;					// set worth
	newObj->StandCount = STAND_TIME;				// set stand duration
	newObj->InjuryThreshold = GIANT_DAMAGE_THRESHOLD;

			/* MAKE SHADOW */

	newObj->ShadowIndex = MakeShadow(newObj,SHADOWSIZE_GIANT); 	// remember ptr to shadow


	gNumEnemies++;
	return(true);									// was added
}


/********************* MOVE GIANT *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveGiant(void)
{
static	void(*moveTable[])(void) =
				{
					MoveGiant_Stand,				// stand
					MoveGiant_Hop,				// hop
					MoveGiant_Land				// land
				};

	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	GetObjectInfo();
	moveTable[gThisNodePtr->SubType]();
}


/**************** MOVE GIANT: STAND ********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveGiant_Stand(void)
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
		SwitchAnim(theNode,GIANT_SUB_HOP);
		theNode->DZ = -0x1e0000L;						// start bouncing up

		PlaySound(gSoundNum_BarneyJump);
	}

					/* UPDATE */
	UpdateEnemy();
}


/**************** MOVE GIANT: HOP ********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveGiant_Hop(void)
{
register	ObjNode *theNode;
long		ctype;

	if (TrackEnemy())									// see if out of range
		return;

	theNode = gThisNodePtr;

				/* DO VERTICAL FALL */

	theNode->DZ += 0x18000L;						// add gravity
	theNode->YOffset.L += theNode->DZ;				// move it

	if (theNode->YOffset.Int >= 0)					// see if landed
	{
		SwitchAnim(theNode,GIANT_SUB_LAND);
		StartShakeyScreen(GAME_FPS/2);
		PlaySound(gSoundNum_DinoBoom);	// play sound
		MakeGiantDeathRing();
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


/**************** MOVE GIANT: LAND ********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveGiant_Land(void)
{
	if (TrackEnemy())									// see if out of range
		return;

					/* DO COLLISION */

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

					/* UPDATE */

	UpdateEnemy();
}



/******************** MAKE GIANT DEATH RING ********************/

void MakeGiantDeathRing(void)
{
register ObjNode	*newNode;
short	i;
static	float	sinTbl[16] =	{0,0.38268,0.7071,0.92387,
								1,0.92387,0.7071,0.38268,
								0,-0.38268,-0.7071,-0.92387,
								-1,-0.92387,-0.7071,-0.38268};

static	float	cosTbl[16] =	{-1,-0.92387,-0.7071,-0.38268,
								0,0.38268,0.7071,0.92387,
								1,0.92387,0.7071,0.38268,
								0,-0.38268,-0.7071,-0.92387};


	for (i=0; i < 16; i++)
	{
		newNode = MakeNewShape(GroupNum_RocketGun,ObjType_RocketGun,8,
								gX.Int,gY.Int,gThisNodePtr->Z,MoveGiantDeathRing,
								PLAYFIELD_RELATIVE);
		if (newNode == nil)
			return;

		newNode->CType = CTYPE_ENEMYC;
		newNode->CBits = CBITS_TOUCHABLE;

		newNode->TopOff = -10;						// set collision box
		newNode->BottomOff = 10;
		newNode->LeftOff = -10;
		newNode->RightOff = 10;

		newNode->DX = (float)sinTbl[i]*(float)0x80000L;
		newNode->DY = (float)cosTbl[i]*(float)0x80000L;

		newNode->TileMaskFlag = false;				// ignore prioritized tiles

		newNode->AnimSpeed = 0x70;					// slow it down
	}
}


/*************** MOVE GIANT DEATH RING ********************/
//
// gThisNodePtr = ptr to current node
//

void MoveGiantDeathRing(void)
{
	if (TrackItem())
	{
		DeleteObject(gThisNodePtr);
		return;
	}

	GetObjectInfo();

	gX.L += gDX;									// move it
	gY.L += gDY;

				/* UPDATE OBJECT */

	CalcObjectBox();
	UpdateObject();
}


