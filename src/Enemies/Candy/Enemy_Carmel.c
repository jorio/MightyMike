/****************************/
/*    	ENEMY_Carmel        */
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
#include "sound2.h"
#include "enemy3.h"
#include "objecttypes.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY;
extern	union_gX;
extern	union_gY;
extern	CollisionRec	gCollisionList[];
extern	long				gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	long				gMyX,gMyY;
extern	short				gNumEnemies;
extern	ObjNode			*gMyNodePtr;
extern	short			gEnemyFreezeTimer;
extern	short				gSoundNum_Carmel;

/****************************/
/*    CONSTANTS             */
/****************************/

#define	CARMEL_ACCEL		0x90000L
#define	CARMEL_FRICTION		0x10000L

#define	CARMEL_HEALTH			2
#define	CARMEL_WORTH			1
#define CARMEL_DAMAGE_THRESHOLD	1

#define	ACTIVATE_DIST		150

enum
{
	SUB_APPEAR_RIGHT,
	SUB_APPEAR_LEFT,
	SUB_SHOOT,
	SUB_DROP,
	SUB_WALK_RIGHT,
	SUB_WALK_LEFT
};


/**********************/
/*     VARIABLES      */
/**********************/

#define	MovePulse		Flag0			// flag gets set when need to pulse the carmel dude's movement
#define	OkChangeFlag	Flag1			// flag set when @ end of a walk cycle

#define	ShootDrop		Flag0


/************************ ADD ENEMY: CARMEL********************/

Boolean AddEnemy_Carmel(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;
short		anim;

	if (gNumEnemies >= MAX_ENEMIES)				// check # enemies
		return(false);

	if (itemPtr->parm[0] == 0)					// see which type
	{
					/********************/
					/* ADD WALKING TYPE */
					/********************/

		if (gMyX < itemPtr->x)					// see which way to start going
			anim = SUB_APPEAR_LEFT;
		else
			anim = SUB_APPEAR_RIGHT;

		newObj = MakeNewShape(GroupNum_Carmel,ObjType_Carmel,anim,itemPtr->x,
							itemPtr->y,50,MoveCarmel,PLAYFIELD_RELATIVE);
		if (newObj == nil)
			return(false);

		newObj->ItemIndex = itemPtr;				// remember where this came from
		newObj->CType = 0;							// set collision info
		newObj->CBits = CBITS_TOUCHABLE;
		newObj->Health = CARMEL_HEALTH;				// set health
		newObj->TopOff = -30;						// set box
		newObj->BottomOff = 0;
		newObj->LeftOff = -30;
		newObj->RightOff = 30;
		CalcObjectBox2(newObj);
		newObj->Worth = CARMEL_WORTH;				// set worth
		newObj->InjuryThreshold = CARMEL_DAMAGE_THRESHOLD;

		newObj->MovePulse = false;
		newObj->OkChangeFlag = false;

		newObj->DrawFlag = 							// DEACTIVATE FOR NOW
		newObj->EraseFlag =
		newObj->AnimFlag = false;

	}
	else
	{
					/*********************/
					/* ADD SHOOTING TYPE */
					/*********************/

		newObj = MakeNewShape(GroupNum_Carmel,ObjType_Carmel,SUB_SHOOT,itemPtr->x,
							itemPtr->y,50,MoveCarmel,PLAYFIELD_RELATIVE);
		if (newObj == nil)
			return(false);

		newObj->ItemIndex = itemPtr;				// remember where this came from
		newObj->CType = 0;							// set collision info
		newObj->CBits = CBITS_TOUCHABLE;
		newObj->Health = CARMEL_HEALTH;				// set health
		newObj->TopOff = -30;						// set box
		newObj->BottomOff = 0;
		newObj->LeftOff = -20;
		newObj->RightOff = 20;
		CalcObjectBox2(newObj);
		newObj->Worth = CARMEL_WORTH;				// set worth
		newObj->InjuryThreshold = CARMEL_DAMAGE_THRESHOLD;

		newObj->ShootDrop = false;

		newObj->DrawFlag = 							// DEACTIVATE FOR NOW
		newObj->EraseFlag =
		newObj->AnimFlag = false;

	}


	gNumEnemies++;

	return(true);								// was added
}


/********************* MOVE CARMEL *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveCarmel(void)
{
static	void(*moveTable[])(void) =
				{
					MoveCarmel_Appear,
					MoveCarmel_Appear,
					MoveCarmel_Shoot,
					MoveCarmelDrop,
					MoveCarmel_Walk,
					MoveCarmel_Walk,
				};

	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	if (TrackEnemy())									// see if out of range
		return;

	GetObjectInfo();
	moveTable[gThisNodePtr->SubType]();
}


/*************** MOVE CARMEL: APPEAR ********************/

void MoveCarmel_Appear(void)
{

	if (gThisNodePtr->AnimFlag == false)			// see if in deactive mode
	{
		if ((Absolute(gMyX - gX.Int) < ACTIVATE_DIST) &&		// see if in range to activate
			(Absolute(gMyY - gY.Int) < ACTIVATE_DIST))
		{
			gThisNodePtr->DrawFlag =
			gThisNodePtr->EraseFlag =
			gThisNodePtr->AnimFlag = true;
			gThisNodePtr->CType = CTYPE_ENEMYA;
		}
	}
}


/*************** MOVE CARMEL: WALK **********************/

void MoveCarmel_Walk(void)
{
	Decay(&gDX,0x10000L);			// decay move
	Decay(&gDY,0x10000L);


				/* PULSE TOWARD ME */

	if (gThisNodePtr->MovePulse)
	{
		gThisNodePtr->MovePulse = false;

		if (gX.Int < gMyX)
			gDX = CARMEL_ACCEL;
		else
			gDX = -CARMEL_ACCEL;

		if (gY.Int > gMyY)
			gDY = -CARMEL_ACCEL;
		else
			gDY = CARMEL_ACCEL;

		PlaySound(gSoundNum_Carmel);				// make sound

	}

					/* MOVE IT */
	gX.L += gDX;
	gY.L += gDY;


	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

					/* UPDATE */

	if (gThisNodePtr->OkChangeFlag)						// see if can change directions now
	{
		gThisNodePtr->OkChangeFlag = false;
		if (gMyX < gX.Int)
		{
			if (gThisNodePtr->SubType != SUB_WALK_LEFT)
				SwitchAnim(gThisNodePtr,SUB_WALK_LEFT);
		}
		else
		{
			if (gThisNodePtr->SubType != SUB_WALK_RIGHT)
				SwitchAnim(gThisNodePtr,SUB_WALK_RIGHT);
		}
	}

	UpdateEnemy();
}



/*************** MOVE CARMEL: SHOOT ********************/

void MoveCarmel_Shoot(void)
{
ObjNode	*newObj;

				/* SEE IF WAITING TO ACTIVATE */

	if (gThisNodePtr->AnimFlag == false)
	{
		if ((Absolute(gMyX - gX.Int) < ACTIVATE_DIST) &&		// see if in range to activate
			(Absolute(gMyY - gY.Int) < ACTIVATE_DIST))
		{
			gThisNodePtr->DrawFlag =
			gThisNodePtr->EraseFlag =
			gThisNodePtr->AnimFlag = true;
			gThisNodePtr->CType = CTYPE_ENEMYA;
		}
	}
	else
	{
					/* READY TO GO */

		if (gThisNodePtr->ShootDrop)						// see if shoot
		{
			gThisNodePtr->ShootDrop = false;

			newObj = MakeNewShape(GroupNum_Carmel,ObjType_Carmel,SUB_DROP,gX.Int,gY.Int,
								gThisNodePtr->Z,MoveCarmelDrop,PLAYFIELD_RELATIVE);
			if (newObj != nil)
			{
				newObj->CType = CTYPE_ENEMYB;				// set collision info
				newObj->CBits = CBITS_TOUCHABLE;
				newObj->TopOff = -8;						// set box
				newObj->BottomOff = 0;
				newObj->LeftOff = -8;
				newObj->RightOff = 8;
				CalcObjectBox2(newObj);

				newObj->YOffset.Int = -70;
				newObj->DZ = -0xb0000L;						// start bouncing up

				newObj->DX = (long)(gMyX-gThisNodePtr->X.Int)*0x1000L;		// aim at me
				newObj->DY = (long)(gMyY-gThisNodePtr->Y.Int)*0x1000L;

				if (newObj->DX > 0x50000L)					// limit max speed
					newObj->DX = 0x50000L;
				else
				if (newObj->DX < -0x50000L)
					newObj->DX = -0x50000L;

				if (newObj->DY > 0x50000L)
					newObj->DY = 0x50000L;
				else
				if (newObj->DY < -0x50000L)
					newObj->DY = -0x50000L;

				newObj->ShadowIndex = MakeShadow(newObj,SHADOWSIZE_SMALL);	// allocate shadow
			}
		}
	}
}

/*************** MOVE CARMEL DROP ********************/

void MoveCarmelDrop(void)
{
register	ObjNode		*theNode;

	theNode = gThisNodePtr;

	theNode->YOffset.L += (theNode->DZ += 0x0f000L);		// gravity & move it
	if (theNode->YOffset.Int > -1)							// see if landed
	{
		DeleteObject(theNode);
		return;
	}

	theNode->X.L += theNode->DX;							// move x/y
	theNode->Y.L += theNode->DY;

	CalcObjectBox2(theNode);
}


