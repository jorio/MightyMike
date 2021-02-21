/****************************/
/*    	ENEMY_DINOEGG       */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/****************************/
/*    EXTERNALS             */
/****************************/
#include "myglobals.h"
#include "shape.h"
#include "windows.h"
#include "playfield.h"
#include "enemy.h"
#include "object.h"
#include "misc.h"
#include "miscanims.h"
#include "objecttypes.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY;
extern	union_gX;
extern	union_gY;
extern	CollisionRec	gCollisionList[];
extern	Byte			gTotalSides;
extern	long				gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	long				gMyX,gMyY;
extern	short				gNumEnemies;
extern	short			gEnemyFreezeTimer;


/****************************/
/*    CONSTANTS             */
/****************************/

#define	HATCHLING_HEALTH	1
#define	HATCH_RANGE			100
#define	HATCHLING_WORTH		1
#define	HATCHLING_DAMAGE_THRESHOLD	1

#define	NUM_HATCHERS		6					// # hatchlings to come out of egg

enum
{
	DINOEGG_SUB_WAIT,
	DINOEGG_SUB_HATCH,
	DINOEGG_SUB_HATCHLING_LEFT,
	DINOEGG_SUB_HATCHLING_RIGHT
};


/**********************/
/*     VARIABLES      */
/**********************/

#define	HatchFlag	Flag0

static	long	gHatchlingDXY[16] = {0xe000L,0x11000L,0x12000L,0x1a000L,
							0x08000L,0x0c000L,0x04000L,0x0f000L,
							-0xe000L,-0x11000L,-0x12000L,-0xa000L,
							-0x08000L,-0x0c000L,-0x04000L,-0x0f000};

/****************** ADD DINO EGG ******************/
//
// Add the egg which isnt actually an enemy yet.
//

Boolean AddDinoEgg(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	newObj = MakeNewShape(GroupNum_DinoEgg,ObjType_DinoEgg,DINOEGG_SUB_WAIT,
			itemPtr->x,itemPtr->y,50,MoveDinoEgg,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CType = CTYPE_MISC;						// set collision info
	newObj->CBits = CBITS_ALLSOLID;

	newObj->TopOff = -14;						// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -10;
	newObj->RightOff = 10;
	CalcObjectBox2(newObj);

	newObj->HatchFlag = false;						// not hatched yet

	return(true);									// was added
}


/******************* MOVE DINO EGG **********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveDinoEgg(void)
{
short		i;
register	ObjNode  *theNode;

	if (gEnemyFreezeTimer)									// see if frozen
	{
		SimpleObjectMove();
		return;
	}

	theNode = gThisNodePtr;

	if (TrackItem())										// see if out of range
	{
		DeleteObject(theNode);
		return;
	}

	if (theNode->SubType == DINOEGG_SUB_WAIT)				// see if hatch
	{
		if ((Absolute(theNode->X.Int - gMyX) < HATCH_RANGE) &&	// see if Im in range
			(Absolute(theNode->Y.Int - gMyY) < HATCH_RANGE))
		{
			SwitchAnim(theNode,DINOEGG_SUB_HATCH);
		}
	}
	else													// hatching
	{
		if (theNode->HatchFlag)								// see if spawn
		{
			theNode->HatchFlag = false;
			for (i=0; i < NUM_HATCHERS; i++)
			{
				MakeHatchling(theNode->X.Int,theNode->Y.Int);
			}
		}
	}
}


/************************ MAKE HATCHLING ********************/

void MakeHatchling(short x, short y)
{
register	ObjNode		*newObj;

	if (gNumEnemies >= 15)					// check # enemies
		return;

	newObj = MakeNewShape(GroupNum_DinoEgg,ObjType_DinoEgg,DINOEGG_SUB_HATCHLING_RIGHT,
						x,y,50,MoveHatchling,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return;

	newObj->CType = CTYPE_ENEMYA;				// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = HATCHLING_HEALTH;			// set health
	newObj->TopOff = -8;						// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -6;
	newObj->RightOff = 6;
	CalcObjectBox2(newObj);

	newObj->DX = gHatchlingDXY[MyRandomLong()&b1111];
	newObj->DY = gHatchlingDXY[MyRandomLong()&b1111];

	newObj->YOffset.Int = -25;
	newObj->DZ = -0xf0000;						// start bouncing up

	newObj->Worth = HATCHLING_WORTH;			// set worth
	newObj->InjuryThreshold = HATCHLING_DAMAGE_THRESHOLD;

	gNumEnemies++;
}


/********************* MOVE HATCHLING *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveHatchling(void)
{
	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	if (TrackEnemy())									// see if out of range
		return;

	GetObjectInfo();

	gX.L += gDX;										// move it
	gY.L += gDY;

	if (gThisNodePtr->YOffset.Int < 0)					// see if still in air
	{
		gThisNodePtr->DZ += 0x28000L;					// add gravity
		gThisNodePtr->YOffset.L += gThisNodePtr->DZ;	// move it

		if (gThisNodePtr->YOffset.Int > 0)				// see if landed
		{
			gThisNodePtr->YOffset.Int = 0;
			gThisNodePtr->DZ = 0;
		}
	}

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	if (gTotalSides&SIDE_BITS_TOP)						// see if bounce
		gDY = -gDY;
	else
	if (gTotalSides&SIDE_BITS_BOTTOM)
		gDY = -gDY;

	if (gTotalSides&SIDE_BITS_LEFT)
		gDX = -gDX;
	else
	if (gTotalSides&SIDE_BITS_RIGHT)
		gDX = -gDX;


	if (gDX < 0)										// check aim
	{
		if (gThisNodePtr->SubType != DINOEGG_SUB_HATCHLING_LEFT)
			SwitchAnim(gThisNodePtr,DINOEGG_SUB_HATCHLING_LEFT);
	}
	else
	{
		if (gThisNodePtr->SubType != DINOEGG_SUB_HATCHLING_RIGHT)
			SwitchAnim(gThisNodePtr,DINOEGG_SUB_HATCHLING_RIGHT);
	}

	UpdateEnemy();
}










