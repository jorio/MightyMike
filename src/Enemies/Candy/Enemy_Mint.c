/****************************/
/*    	ENEMY_MINT        */
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

extern	ObjNode			*gThisNodePtr,*FirstNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY;
extern	MikeFixed		gX;
extern	MikeFixed		gY;
extern	CollisionRec	gCollisionList[];
extern	long				gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	long				gMyX,gMyY;
extern	short				gNumEnemies;
extern	ObjNode			*gMyNodePtr;
extern	Byte			gTotalSides;
extern	short			gEnemyFreezeTimer;


/****************************/
/*    CONSTANTS             */
/****************************/

#define	MINT_SPEED				0x2a000L
#define	MINT_HEALTH				1
#define	MINT_WORTH				1
#define MINT_DAMAGE_THRESHOLD	1

/**********************/
/*     VARIABLES      */
/**********************/


/************************ ADD ENEMY: MINT********************/

Boolean AddEnemy_Mint(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	if (gNumEnemies >= MAX_ENEMIES)				// check # enemies
		return(false);

	newObj = MakeNewShape(GroupNum_Mint,ObjType_Mint,0,itemPtr->x,
						itemPtr->y,50,MoveMint,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;				// remember where this came from
	newObj->CType = CTYPE_ENEMYA;				// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = MINT_HEALTH;				// set health
	newObj->TopOff = -8;						// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -8;
	newObj->RightOff = 8;
	CalcObjectBox2(newObj);

	newObj->Worth = MINT_WORTH;				// set worth
	newObj->InjuryThreshold = MINT_DAMAGE_THRESHOLD;

	if (itemPtr->x < gMyX)							// set random deltas
		newObj->DX = MINT_SPEED;
	else
		newObj->DX = -MINT_SPEED;

	if (itemPtr->y < gMyY)
		newObj->DY = MINT_SPEED;
	else
		newObj->DY = -MINT_SPEED;


	gNumEnemies++;

	return(true);								// was added
}


/********************* MOVE MINT*********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveMint(void)
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

	UpdateEnemy();
}




