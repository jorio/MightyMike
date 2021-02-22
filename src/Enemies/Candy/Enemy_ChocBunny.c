/****************************/
/*    	ENEMY_CHOCBUNNY      */
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
#include "enemy3.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY;
extern	union_gX;
extern	union_gY;
extern	CollisionRec	gCollisionList[];
extern	long				gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	long				gMyX,gMyY;
extern	short				gNumEnemies,gSoundNum_ChocoBunny;
extern	short			gEnemyFreezeTimer;


/****************************/
/*    CONSTANTS             */
/****************************/

enum{
		CHOCBUNNY_SUB_STAND,
		CHOCBUNNY_SUB_HOP,
		CHOCBUNNY_SUB_LAND
};

#define	CHOCBUNNY_HEALTH			10
#define CHOCBUNNY_WORTH				2
#define	CHOCBUNNY_DAMAGE_THRESHOLD	3					// min weapon power needed to do damage

#define STAND_TIME		(MyRandomLong()&b11111+20)

#define	JUMP_FACTOR		0x130000L

/**********************/
/*     VARIABLES      */
/**********************/

#define StandCount		Special1		// counter for enemy standing b4 hop


/************************ ADD ENEMY: CHOCBUNNY ********************/

Boolean AddEnemy_ChocBunny(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	if (gNumEnemies >= MAX_ENEMIES)					// check # enemies
		return(false);


	newObj = MakeNewShape(GroupNum_ChocBunny,ObjType_ChocBunny,CHOCBUNNY_SUB_STAND,
			itemPtr->x,itemPtr->y,50,MoveChocBunny,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CType = CTYPE_ENEMYA;					// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = CHOCBUNNY_HEALTH;				// set health

	newObj->TopOff = -15;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -20;
	newObj->RightOff = 20;
	CalcObjectBox2(newObj);

	newObj->Worth = CHOCBUNNY_WORTH;					// set worth
	newObj->StandCount = STAND_TIME;				// set stand duration
	newObj->InjuryThreshold = CHOCBUNNY_DAMAGE_THRESHOLD;

			/* MAKE SHADOW */

	newObj->ShadowIndex = MakeShadow(newObj,SHADOWSIZE_GIANT); 	// remember ptr to shadow

	gNumEnemies++;
	return(true);									// was added
}


/********************* MOVE CHOCBUNNY *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveChocBunny(void)
{
static	void(*moveTable[])(void) =
				{
					MoveChoc_Stand,				// stand
					MoveChoc_Hop,				// hop
					MoveChoc_Land				// land
				};

	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	GetObjectInfo();
	moveTable[gThisNodePtr->SubType]();
}


/**************** MOVE CHOC: STAND ********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveChoc_Stand(void)
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
		SwitchAnim(theNode,CHOCBUNNY_SUB_HOP);
		theNode->DZ = -JUMP_FACTOR;						// start bouncing up

		gDX = (long)(gMyX - gX.Int)*0x600L;				// go toward me
		gDY = (long)(gMyY - gY.Int)*0x600L;

		PlaySound(gSoundNum_ChocoBunny);
	}

					/* UPDATE */
	UpdateEnemy();
}


/**************** MOVE CHOC: HOP ********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveChoc_Hop(void)
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
		SwitchAnim(theNode,CHOCBUNNY_SUB_LAND);
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


/**************** MOVE CHOC: LAND ********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveChoc_Land(void)
{
	if (TrackEnemy())									// see if out of range
		return;

					/* DO COLLISION */

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

					/* UPDATE */

	UpdateEnemy();
}





