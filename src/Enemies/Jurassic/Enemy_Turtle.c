/****************************/
/*    	ENEMY_TURTLE       	*/
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
#include "shape.h"
#include "misc.h"
#include "miscanims.h"
#include "objecttypes.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY;
extern	union_gX;
extern	union_gY;
extern	CollisionRec	gCollisionList[];
extern	long				gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	long				gMyX,gMyY;
extern	short				gNumEnemies;
extern	short			gEnemyFreezeTimer;

/****************************/
/*    CONSTANTS             */
/****************************/

#define	TURTLE_HEALTH			40
#define	TURTLE_WORTH			10
#define	TURTLE_DAMAGE_THRESHOLD	7

#define		TURTLE_SPEED	0x28000L

enum
{
	TURTLE_SUB_UP,
	TURTLE_SUB_RIGHT,
	TURTLE_SUB_DOWN,
	TURTLE_SUB_LEFT
};



/**********************/
/*     VARIABLES      */
/**********************/


/************************ ADD ENEMY: TURTLE ********************/

Boolean AddEnemy_Turtle(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	newObj = MakeNewShape(GroupNum_Turtle,ObjType_Turtle,0,
			itemPtr->x,itemPtr->y,FARTHEST_Z,MoveTurtle,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CType = CTYPE_MISC;						// set collision info
	newObj->CBits = CBITS_ALLSOLID;

	newObj->TopOff = -35;							// set box
	newObj->BottomOff = 35;
	newObj->LeftOff = -40;
	newObj->RightOff = 40;
	CalcObjectBox2(newObj);

	newObj->Worth = TURTLE_WORTH;				// set worth
	newObj->Health = TURTLE_HEALTH;				// set health
	newObj->InjuryThreshold = TURTLE_DAMAGE_THRESHOLD;

	gNumEnemies++;

	return(true);								// was added
}


/********************* MOVE TURTLE *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveTurtle(void)
{
	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	if (TrackEnemy())											// see if out of range
		return;

	GetObjectInfo();



	switch(MoveOnPath(TURTLE_SPEED,false))						// see which way to aim
	{
		case	ALT_TILE_DIR_UP:
				if (gThisNodePtr->SubType != TURTLE_SUB_UP)
					SwitchAnim(gThisNodePtr,TURTLE_SUB_UP);
				break;

		case	ALT_TILE_DIR_RIGHT:
				if (gThisNodePtr->SubType != TURTLE_SUB_RIGHT)
					SwitchAnim(gThisNodePtr,TURTLE_SUB_RIGHT);
				break;

		case	ALT_TILE_DIR_DOWN:
				if (gThisNodePtr->SubType != TURTLE_SUB_DOWN)
					SwitchAnim(gThisNodePtr,TURTLE_SUB_DOWN);
				break;

		case	ALT_TILE_DIR_LEFT:
				if (gThisNodePtr->SubType != TURTLE_SUB_LEFT)
					SwitchAnim(gThisNodePtr,TURTLE_SUB_LEFT);
				break;
	}

	if (DoEnemyCollisionDetect(ENEMY_NO_BG_COLLISION))			// returns true if died
		return;

	UpdateEnemy();
}



