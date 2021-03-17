/****************************/
/*    	ENEMY_SLINKY   		*/
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
#include "enemy5.h"
#include "shape.h"
#include "object.h"
#include "misc.h"
#include "objecttypes.h"
#include "collision.h"
#include "externs.h"

/****************************/
/*    CONSTANTS             */
/****************************/

#define	SLINKY_HEALTH			2
#define SLINKY_WORTH			2
#define	SLINKY_DAMAGE_THRESHOLD	1

enum
{
	UP_TRANSPOSE	=	-16,
	RIGHT_TRANSPOSE	=	48,
	DOWN_TRANSPOSE	=	16,
	LEFT_TRANSPOSE	=	-46
};

/**********************/
/*     VARIABLES      */
/**********************/

#define	DoneFlag	Flag0

/************************ ADD ENEMY: SLINKY ********************/

Boolean AddEnemy_Slinky(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;
short			animNum;

	if (gNumEnemies >= MAX_ENEMIES)					// check # enemies
		return(false);

	animNum = GuessSlinkyAnim(0,itemPtr->x,itemPtr->y);

	newObj = MakeNewShape(GroupNum_Slinky,ObjType_Slinky,animNum,
			itemPtr->x,itemPtr->y,50,MoveSlinky,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CType = CTYPE_ENEMYA;					// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = SLINKY_HEALTH;					// set health

	newObj->TopOff = -20;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -20;
	newObj->RightOff = 20;
	CalcObjectBox2(newObj);

	newObj->Worth = SLINKY_WORTH;					// set worth
	newObj->InjuryThreshold = SLINKY_DAMAGE_THRESHOLD;

	newObj->DoneFlag = false;

	gNumEnemies++;
	return(true);									// was added
}


/********************* MOVE SLINKY *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveSlinky(void)
{
short	anim;

	if (gEnemyFreezeTimer)								// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	if (TrackEnemy())									// see if out of range
		return;

	GetObjectInfo();

				/* SEE IF DONE WITH ANIM */

	if (gThisNodePtr->DoneFlag)
	{
		gThisNodePtr->DoneFlag = false;
		anim = GuessSlinkyAnim(gThisNodePtr->SubType,gX.Int,gY.Int);			// choose another direction
		SwitchAnim(gThisNodePtr,anim);
		CalcSlinkyBox();
	}

	if (DoEnemyCollisionDetect(ENEMY_NO_BG_COLLISION))	// returns true if died
		return;


					/* UPDATE */

	UpdateEnemy();
}



/***************** GUESS SLINKY ANIM *************************/

short	GuessSlinkyAnim(short fromAnim, short x, short y)
{
short	diffX,diffY,toAnim,i;
Boolean	wall;


					/* DETERMINE THE ANIM */

	diffX = gMyX - x;
	diffY = gMyY - y;

	if (Absolute(diffX) < 50)				// see if go vertical
	{
		if (diffY < 0)
			toAnim = 0;						// go up
		else
			toAnim = 2;						// go down
	}
	else
	{
		if (diffX < 0)
			toAnim = 3;						// go left
		else
			toAnim = 1;						// go right
	}

			/* SEE IF DESIRED ANIM IS OKAY */

	for (i=0;i<4;i++)						// at worst, try all 4 directions
	{
		switch(toAnim)
		{
			case	0:
					wall = DoPointCollision(gX.Int,gY.Int-32,CTYPE_BGROUND);
					break;
			case	1:
					wall = DoPointCollision(gX.Int+32,gY.Int,CTYPE_BGROUND);
					break;
			case	2:
					wall = DoPointCollision(gX.Int,gY.Int+32,CTYPE_BGROUND);
					break;
			case	3:
					wall = DoPointCollision(gX.Int-32,gY.Int,CTYPE_BGROUND);
					break;
		}

		if (!wall)								// see if legal
			break;
		else
			toAnim = (toAnim+1)&0b11;			// nope, try next direction
	}


				/* TRANSPOSE */

	switch(fromAnim)
	{
		case	0:
				switch(toAnim)
				{
					case	0:								// from UP to UP
					case	3:								// from UP to LEFT
							gY.Int += UP_TRANSPOSE;
							break;
					case	1:								// from UP to RIGHT
							gX.Int += RIGHT_TRANSPOSE;
							gY.Int += UP_TRANSPOSE;
							break;
				}
				break;

		case	1:
				switch(toAnim)
				{
					case	1:								// from RIGHT to RIGHT
							gX.Int += RIGHT_TRANSPOSE;
							break;
					case	2:								// from RIGHT to DOWN
							gY.Int += DOWN_TRANSPOSE;
							break;
				}
				break;

		case	2:
				switch(toAnim)
				{
					case	1:								// from DOWN to RIGHT
							gX.Int += RIGHT_TRANSPOSE;
							break;
					case	2:								// from DOWN to DOWN
							gY.Int += DOWN_TRANSPOSE;
							break;
				}
				break;

		case	3:
				switch(toAnim)
				{
					case	0:								// from LEFT to UP
							gX.Int += LEFT_TRANSPOSE;
							break;
					case	2:								// from LEFT to DOWN
							gX.Int += LEFT_TRANSPOSE;
							gY.Int += DOWN_TRANSPOSE;
							break;
					case	3:								// from LEFT to LEFT
							gX.Int += LEFT_TRANSPOSE;
							break;
				}
				break;
	}

	return(toAnim);
}



/************** CALC SLINKY BOX ****************/

void CalcSlinkyBox(void)
{
	switch(gThisNodePtr->SubType)
	{
		case	0:
		case	2:
				gTopSide = -30;
				gBottomSide = 0;
				gLeftSide = -10;
				gRightSide = 10;
				break;

		case	1:
		case	3:
				gTopSide = -20;
				gBottomSide = 0;
				gLeftSide = -5;
				gRightSide = 30;
				break;

	}
}


