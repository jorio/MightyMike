/****************************/
/*    	ENEMY_HATBUNNY      */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/****************************/
/*    EXTERNALS             */
/****************************/
#include "myglobals.h"
#include "windows.h"
#include "playfield.h"
#include "shape.h"
#include "enemy.h"
#include "object.h"
#include "misc.h"
#include "miscanims.h"
#include "sound2.h"
#include "enemy2.h"
#include "objecttypes.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY;
extern	union_gX;
extern	union_gY;
extern	CollisionRec	gCollisionList[];
extern	long				gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	long				gMyX,gMyY;
extern	short				gNumEnemies,gSoundNum_CarrotThrow;
extern	ObjNode			*gMyNodePtr;
extern	short			gEnemyFreezeTimer;

/****************************/
/*    CONSTANTS             */
/****************************/

#define	HATBUNNY_MAX_SPEED	0x30000L
#define	HATBUNNY_ACCEL		0x3000L
#define	HATBUNNY_HEALTH		4
#define	HATBUNNY_WORTH		5
#define HATBUNNY_DAMAGE_THRESHOLD	3

enum
{
	SUB_HAT_WAIT,
	SUB_HAT_SPIT,
	SUB_WALK_RIGHT,
	SUB_WALK_LEFT
};

#define HatSpitFlag		Flag0

#define	BOUNCE_FACTOR	-0xb0000L

/**********************/
/*     VARIABLES      */
/**********************/

//============================================================================================

/*************** ADD MAGICHAT ******************/

Boolean AddMagicHat(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	newObj = MakeNewShape(GroupNum_MagicHat,ObjType_MagicHat,SUB_HAT_WAIT,
			itemPtr->x,itemPtr->y,50,MoveMagicHat,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CType = CTYPE_MISC;						// set collision info
	newObj->CBits = CBITS_ALLSOLID;

	newObj->TopOff = -16;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -16;
	newObj->RightOff = 16;
	CalcObjectBox2(newObj);

	newObj->HatSpitFlag = false;

	return(true);									// was added
}


/******************** MOVE MAGICHAT ******************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveMagicHat(void)
{

	if (TrackItem())									// see if out of range
	{
		DeleteObject(gThisNodePtr);
		return;
	}

	if (gEnemyFreezeTimer)					// see if frozen
		return;

	if (gThisNodePtr->SubType == 0)
	{
				/* WAITING */

		if ((!(MyRandomLong()&b1111111)) && (gNumEnemies < MAX_ENEMIES))
		{
			SwitchAnim(gThisNodePtr,SUB_HAT_SPIT);		// start to spit
		}
	}
	else
	{
				/* SPITTING */

		if (gThisNodePtr->HatSpitFlag)
		{
			gThisNodePtr->HatSpitFlag = false;
			MakeHatBunny();

		}
	}
}


/************************ MAKE HATBUNNY ********************/

static void MakeHatBunny(void)
{
register	ObjNode		*newObj;

	newObj = MakeNewShape(GroupNum_MagicHat,ObjType_MagicHat,SUB_WALK_RIGHT,gThisNodePtr->X.Int,
						gThisNodePtr->Y.Int,gThisNodePtr->Z,MoveHatBunny,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return;

	CalcEnemyScatterOffset(newObj);

	newObj->CType = CTYPE_ENEMYA;				// set collision info
	newObj->CBits = 0;
//	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = HATBUNNY_HEALTH;			// set health
	newObj->TopOff = -30;						// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -14;
	newObj->RightOff = 14;
	CalcObjectBox2(newObj);
	newObj->Worth = HATBUNNY_WORTH;				// set worth
	newObj->InjuryThreshold = HATBUNNY_DAMAGE_THRESHOLD;

	newObj->YOffset.Int = -40;
	newObj->DZ = BOUNCE_FACTOR*2;				// start bouncing up


			/* MAKE SHADOW */

	newObj->ShadowIndex = MakeShadow(newObj,SHADOWSIZE_MEDIUM);	// allocate shadow


	gNumEnemies++;

}


/********************* MOVE HATBUNNY *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveHatBunny(void)
{
register ObjNode	*theNode;

	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	theNode = gThisNodePtr;

	if (TrackEnemy())									// see if out of range
		return;

	GetObjectInfo();

				/* BOUNCE */

	theNode->DZ += 0x14000L;							// add gravity
	theNode->YOffset.L += theNode->DZ;					// move it

	if (theNode->YOffset.Int > 0)						// see if landed
	{
		theNode->YOffset.Int = 0;
		theNode->DZ = 0;
		theNode->CBits = CBITS_TOUCHABLE;
	}

	DoHatBunnyMove();

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	UpdateHatBunny();
}




/**************** UPDATE HATBUNNY *******************/

static void UpdateHatBunny(void)
{
	if (!(MyRandomLong() & b1111111))							// see if recalc scatter
		CalcEnemyScatterOffset(gThisNodePtr);

	if (gDX < 0)										// check aim anim
	{
		if (gThisNodePtr->SubType != SUB_WALK_LEFT)
			SwitchAnim(gThisNodePtr,SUB_WALK_LEFT);
	}
	else
	if (gDX > 0)
	{
		if (gThisNodePtr->SubType != SUB_WALK_RIGHT)
			SwitchAnim(gThisNodePtr,SUB_WALK_RIGHT);
	}

	gThisNodePtr->AnimSpeed = (Absolute(gDX)+Absolute(gDY))>>8;

	UpdateEnemy();
}


/************** DO HATBUNNY MOVE ******************/

static void DoHatBunnyMove(void)
{

				/* ACCEL TOWARD ME */

	if (gX.Int < (gMyX+gThisNodePtr->EnemyTargetXOff))
		gDX += HATBUNNY_ACCEL;
	else
	if (gX.Int > (gMyX+gThisNodePtr->EnemyTargetXOff))
		gDX -= HATBUNNY_ACCEL;

	if (gY.Int > (gMyY+gThisNodePtr->EnemyTargetYOff))
		gDY -= HATBUNNY_ACCEL;
	else
	if (gY.Int < (gMyY+gThisNodePtr->EnemyTargetYOff))
		gDY += HATBUNNY_ACCEL;

				/* CHECK MAX DELTAS */

	if (gDX > HATBUNNY_MAX_SPEED)
		gDX -= HATBUNNY_ACCEL;
	else
	if (gDX < -HATBUNNY_MAX_SPEED)
		gDX +=  HATBUNNY_ACCEL;

	if (gDY > HATBUNNY_MAX_SPEED)
		gDY -=  HATBUNNY_ACCEL;
	else
	if (gDY < -HATBUNNY_MAX_SPEED)
		gDY +=  HATBUNNY_ACCEL;

					/* MOVE IT */
	gX.L += gDX;
	gY.L += gDY;
}

