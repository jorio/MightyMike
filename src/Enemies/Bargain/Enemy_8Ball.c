/****************************/
/*    	ENEMY_8BALL        */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/****************************/
/*    EXTERNALS             */
/****************************/
#include "myglobals.h"
#include "window.h"
//#include "playfield.h"
#include "enemy.h"
#include "object.h"
#include "misc.h"
#include "shape.h"
#include "miscanims.h"
#include "enemy5.h"
#include "objecttypes.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY;
extern	MikeFixed		gX;
extern	MikeFixed		gY;
extern	CollisionRec	gCollisionList[];
extern	long			gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	long			gMyX,gMyY;
extern	short			gNumEnemies;
extern	ObjNode			*gMyNodePtr;
extern	Byte			gTotalSides;
extern	short			gEnemyFreezeTimer;


/****************************/
/*    CONSTANTS             */
/****************************/

#define	x8BALL_HEALTH			2
#define	x8BALL_WORTH				1
#define x8BALL_DAMAGE_THRESHOLD	2

#define	x8BALL_SPEED				0x30000L


/**********************/
/*     VARIABLES      */
/**********************/


/************************ ADD ENEMY: 8BALL********************/

Boolean AddEnemy_8Ball(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	if (gNumEnemies >= MAX_ENEMIES)				// check # enemies
		return(false);

	newObj = MakeNewShape(GroupNum_8Ball,ObjType_8Ball,0,itemPtr->x,
						itemPtr->y,50,Move8Ball,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;				// remember where this came from
	newObj->CType = CTYPE_ENEMYA;				// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = x8BALL_HEALTH;				// set health
	newObj->TopOff = -16;						// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -16;
	newObj->RightOff = 16;
	CalcObjectBox2(newObj);

	newObj->Worth = x8BALL_WORTH;				// set worth
	newObj->InjuryThreshold = x8BALL_DAMAGE_THRESHOLD;

	if (itemPtr->x < gMyX)						// set random deltas
		newObj->DX = x8BALL_SPEED;
	else
		newObj->DX = -x8BALL_SPEED;

	if (itemPtr->y < gMyY)
		newObj->DY = x8BALL_SPEED;
	else
		newObj->DY = -x8BALL_SPEED;


	gNumEnemies++;

	return(true);								// was added
}


/********************* MOVE 8BALL*********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void Move8Ball(void)
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




