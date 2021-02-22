/****************************/
/*    	ENEMY_ROBOT       	*/
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
#include "enemy5.h"
#include "sound2.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr,*gMyNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY,gFrames;
extern	union_gX;
extern	union_gY;
extern	CollisionRec	gCollisionList[];
extern	long			gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	long			gMyX,gMyY;
extern	short			gNumEnemies;
extern	short			gEnemyFreezeTimer,gSoundNum_RobotDanger;

/****************************/
/*    CONSTANTS             */
/****************************/

#define	ROBOT_MAX_SPEED	0x30000L
#define	ROBOT_HEALTH	9
#define	ROBOT_WORTH		3
#define	ROBOT_DAMAGE_THRESHOLD 3

enum
{
	SUB_WALK_DOWN,
	SUB_WALK_UP,
	SUB_WALK_RIGHT,
	SUB_WALK_LEFT,
	SUB_DANGER
};


/**********************/
/*     VARIABLES      */
/**********************/

long	gLastRobotDangerTime;


/************************ ADD ENEMY: ROBOT ********************/

Boolean AddEnemy_Robot(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;


	if (gNumEnemies >= MAX_ENEMIES)			// check # enemies
		return(false);

	newObj = MakeNewShape(GroupNum_Robot,ObjType_Robot,SUB_WALK_RIGHT,itemPtr->x,
						itemPtr->y,50,MoveRobot,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	CalcEnemyScatterOffset(newObj);

	newObj->ItemIndex = itemPtr;					// remember where this came from
	newObj->CType = CTYPE_ENEMYA;					// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = ROBOT_HEALTH;				// set health
	newObj->TopOff = -30;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -20;
	newObj->RightOff = 20;
	CalcObjectBox2(newObj);
	newObj->Worth = ROBOT_WORTH;					// set worth
	newObj->InjuryThreshold = ROBOT_DAMAGE_THRESHOLD;

	gNumEnemies++;

	return(true);									// was added
}


/********************* MOVE ROBOT *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveRobot(void)
{
	if (gEnemyFreezeTimer)								// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	if (TrackEnemy())									// see if out of range
		return;

	GetObjectInfo();

	DoRobotMove();
	DoRobotDanger();

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	UpdateEnemy();
}


/************** DO ROBOT MOVE ******************/

void DoRobotMove(void)
{
short	dist;

				/* ACCEL TOWARD ME */

	dist = gMyX+gThisNodePtr->EnemyTargetXOff;
	if (Absolute(gX.Int - dist) > 15)								// only change direction if far
	{
		if (Absolute(gX.Int < dist) )
		{
			gDX = ROBOT_MAX_SPEED;									// go right
			gDY = 0;
		}
		else
		{
			gDX = -ROBOT_MAX_SPEED;									// go left
			gDY = 0;
		}
	}

	dist = (gMyY+gThisNodePtr->EnemyTargetYOff);
	if (Absolute(gY.Int - dist) > 15)								// only change direction if far
	{
		if (gY.Int > dist)
		{
			gDY = -ROBOT_MAX_SPEED;									// go up
			gDX = 0;
		}
		else
		{
			gDY = ROBOT_MAX_SPEED;									// go down
			gDX = 0;
		}
	}

			/* SET CORRECT ANIMATION */


	if (gDX < 0)
	{
		if (gThisNodePtr->SubType != SUB_WALK_LEFT)			// left anim
			SwitchAnim(gThisNodePtr,SUB_WALK_LEFT);
	}
	else
	if (gDX > 0)
	{
		if (gThisNodePtr->SubType != SUB_WALK_RIGHT)		// right anim
			SwitchAnim(gThisNodePtr,SUB_WALK_RIGHT);
	}
	else
	if (gDY < 0)
	{
		if (gThisNodePtr->SubType != SUB_WALK_UP)			// UP anim
			SwitchAnim(gThisNodePtr,SUB_WALK_UP);
	}
	else
	if (gDY > 0)
	{
		if (gThisNodePtr->SubType != SUB_WALK_DOWN)			// down anim
			SwitchAnim(gThisNodePtr,SUB_WALK_DOWN);
	}


					/* MOVE IT */
	gX.L += gDX;
	gY.L += gDY;

}



/**************** DO ROBOT DANGER *****************/

void DoRobotDanger(void)
{
register	ObjNode *newObj;

	if ((gFrames-gLastRobotDangerTime) < GAME_FPS)		// see if been enough time
		return;

	if (gThisNodePtr->OwnerToMessageNode != nil)		// see if already has a message
		return;
	if (gMyNodePtr->OwnerToMessageNode != nil)			// not if Mike is talking
		return;
	if (MyRandomLong()&b11111111)						// random
		return;

				/* MAKE MESSAGE SPRITE */

	newObj = MakeNewShape(GroupNum_Robot,ObjType_Robot,SUB_DANGER,
				gThisNodePtr->X.Int,gThisNodePtr->Y.Int,gThisNodePtr->Z,MoveMessage,
				PLAYFIELD_RELATIVE);

	if (newObj == nil)
		return;

	newObj->MessageTimer = GAME_FPS*3/2;				// set message timer
	newObj->TileMaskFlag = false;						// wont be tile masked
	newObj->MessageToOwnerNode = gThisNodePtr;			// point to owner
	gThisNodePtr->OwnerToMessageNode = newObj;			// point to message

	PlaySound(gSoundNum_RobotDanger);

	gLastRobotDangerTime = gFrames;						// remember when it occurred
}

