/****************************/
/*    	ENEMY               */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/****************************/
/*    EXTERNALS             */
/****************************/
#include "myglobals.h"
#include "objecttypes.h"
#include "window.h"
#include "playfield.h"
#include "enemy.h"
#include "object.h"
#include "bonus.h"
#include "miscanims.h"
#include "infobar.h"
#include "sound2.h"
#include "shape.h"
#include "weapon.h"
#include "collision.h"
#include "misc.h"
#include "io.h"
#include "externs.h"

/****************************/
/*    CONSTANTS             */
/****************************/

#define	SPLAT_TIME				(GAME_FPS*2)

/**********************/
/*     VARIABLES      */
/**********************/

short	gNumEnemies;

short	gEnemyFreezeTimer;

#define	SplatTimer	Special1


/******************** INIT ENEMIES ***********************/

void InitEnemies(void)
{
	gNumEnemies = 0;
	gEnemyFreezeTimer = 0;
	gLastUngaTime = gLastGummyHahaTime = gLastWitchHahaTime =
	gLastDogRoarTime = gLastRobotDangerTime = gLastClownLaughTime = 0;
}


/***************** DO ENEMY COLLISION DETECT ********************/
//
// Returns true if was killed during this collision check.
//

static inline Boolean IsTileImpassable(unsigned short x, unsigned short y)
{
	const TileAttribType* tileAttribs = GetFullMapTileAttribs(x, y);	// get attribs of center tile
	uint16_t bits = tileAttribs ? tileAttribs->bits : 0;
	return bits & (TILE_ATTRIB_WATER | TILE_ATTRIB_DEATH);				// if water or death, then move to old coords
}

Boolean DoEnemyCollisionDetect(unsigned long CType)
{
int16_t offset;

	gSumDX = gDX;											// set sum deltas
	gSumDY = gDY;
	CalcObjectBox();										// calc newest box
	CollisionDetect(gThisNodePtr,CType);					// get collision info

	int16_t originalX = gX.Int;								// remember starting coords
	int16_t originalY = gY.Int;

	for (int i = 0; i < gNumCollisions; i++)				// handle all collisions
	{
		if (gCollisionList[i].type == COLLISION_TYPE_TILE)
		{
					/**************************/
					/* HANDLE TILE COLLISIONS */
					/**************************/

			if (gCollisionList[i].sides & SIDE_BITS_TOP)	// SEE IF HIT TOP
			{
				offset = TILE_SIZE-(gTopSide&(TILE_SIZE-1)); // see how far over it went
				gY.Int = originalY+offset;					// adjust y coord
			}
			else
			if (gCollisionList[i].sides & SIDE_BITS_BOTTOM)	// SEE IF HIT BOTTOM
			{
				offset = (gBottomSide&(TILE_SIZE-1))+1;		// see how far over it went
				gY.Int = originalY-offset;					// adjust y coord
			}


			if (gCollisionList[i].sides & SIDE_BITS_LEFT)	// SEE IF HIT LEFT
			{
				offset = TILE_SIZE-(gLeftSide&(TILE_SIZE-1)); // see how far over it went
				gX.Int = originalX+offset;					// adjust x coord
			}
			else
			if (gCollisionList[i].sides & SIDE_BITS_RIGHT)	// SEE IF HIT RIGHT
			{
				offset = (gRightSide&(TILE_SIZE-1))+1;				// see how far over it went
				gX.Int = originalX-offset;					// adjust x coord
			}

		}
		else
		if (gCollisionList[i].type == COLLISION_TYPE_OBJ)
		{
					/****************************/
					/* HANDLE OBJECT COLLISIONS */
					/****************************/

			if (gCollisionList[i].sides & SIDE_BITS_TOP)	// SEE IF HIT TOP
			{
				offset = (gCollisionList[i].objectPtr->BottomSide-gTopSide)+1;	// see how far over it went
				gY.Int = originalY+offset;					// adjust y coord
			}
			else
			if (gCollisionList[i].sides & SIDE_BITS_BOTTOM)	// SEE IF HIT BOTTOM
			{
				offset = (gBottomSide-gCollisionList[i].objectPtr->TopSide)+1;	// see how far over it went
				gY.Int = originalY-offset;					// adjust y coord
			}


			if (gCollisionList[i].sides & SIDE_BITS_LEFT)	// SEE IF HIT LEFT
			{
				offset = (gCollisionList[i].objectPtr->RightSide-gLeftSide)+1;	// see how far over it went
				gX.Int = originalX+offset;					// adjust x coord
			}
			else
			if (gCollisionList[i].sides & SIDE_BITS_RIGHT)	// SEE IF HIT RIGHT
			{
				offset = (gRightSide-gCollisionList[i].objectPtr->LeftSide)+1;	// see how far over it went
				gX.Int = originalX-offset;					// adjust x coord
			}

						/* SEE IF ENEMY GOT HIT BY BULLET */

			if (gCollisionList[i].objectPtr->CType & CTYPE_MYBULLET)
			{
				WeaponHitEnemy(gCollisionList[i].objectPtr);				// tell weapon manager what happened
				if (EnemyLoseHealth(gThisNodePtr,gCollisionList[i].objectPtr->WeaponPower))		// lose health & see if was killed
					return(true);
			}
			else
					/* SEE IF ENEMY HIT BY MISC HURT THING */

			if (gCollisionList[i].objectPtr->CType & CTYPE_HURTENEMY)
			{
				if (EnemyLoseHealth(gThisNodePtr,10))					// lose health & see if was killed (pass arbitrary hurt power)
					return(true);
			}
		}
	}

					/*************************/
					/* CHECK IMPASSABLE AREA */
					/*************************/

	if (   IsTileImpassable(gX.Int, gY.Int - 16)					// top
		|| IsTileImpassable(gX.Int, gY.Int + 16)					// bottom
		|| IsTileImpassable(gX.Int - 16, gY.Int)					// left
		|| IsTileImpassable(gX.Int + 16, gY.Int))					// right
	{
		// if water or death, then move to old coords
		gX = gThisNodePtr->OldX;
		gY = gThisNodePtr->OldY;
	}

	return false;
}


/***************** UPDATE ENEMY *****************/

void UpdateEnemy(void)
{
	CalcObjectBox();
	UpdateObject();
}


/***************** CALC ENEMY SCATTER OFFSET ****************/
//
// For making enemies come at me, but not exactly.
//

void CalcEnemyScatterOffset(ObjNode *node)
{
	node->EnemyTargetXOff = (MyRandomLong()&0b1111111)-64;	// set coordinate jitter
	node->EnemyTargetYOff = (MyRandomLong()&0b1111111)-64;
}


/********************* ENEMY LOSE HEALTH ********************/
//
// Return true if lost health caused death
//

Boolean EnemyLoseHealth(ObjNode *theEnemy, short amount)
{

	if (gDifficultySetting != DIFFICULTY_EASY)		// no damage thresholds in easy mode
	{
		if (amount < theEnemy->InjuryThreshold)			// see if damage is at minimum threshold to do anything
		{
			PlaySound(SOUND_BADHIT);
			return(false);
		}
	}
	else
		amount *= 2;									// double the damage in EASY mode

	if (gDifficultySetting == DIFFICULTY_HARD)		// half the damage in HARD mode
		if (amount > 1)
			amount /= 2;


	theEnemy->Health -= amount;
	if (theEnemy->Health <= 0)						// see if died
	{
			/* ENEMY WAS KILLED */

		KillEnemy(theEnemy);
		return(true);
	}

			/* ENEMY WAS ONLY HURT */

	return(false);
}


/******************** KILL ENEMY ***************************/

void KillEnemy(ObjNode *theEnemy)
{
register ObjNode *newObj;
register	Byte	i;
register	short		x,y,z;

	GetPoints(100);
	PlaySound(SOUND_CRUNCH);

	x = theEnemy->X.Int;
	y = theEnemy->Y.Int;
	z = theEnemy->Z+1;
	MakeCoins(x,y,z,theEnemy->Worth);							// make coins

	theEnemy->ItemIndex = nil;									// never comin' back
	DeleteEnemy(theEnemy);										// delete the enemy


	PutBonusPOW(x,y);											// try to make special POW

				/* MAKE ENEMY SPLATTERS */

	for (i=0; i < 4; i++)
	{
		newObj = MakeNewShape(GroupNum_Splat,ObjType_Splat,0,x,y,z,MoveEnemySplat,PLAYFIELD_RELATIVE);
		if (newObj == nil)
			return;

		newObj->SplatTimer = (MyRandomLong() & 0b11111) + SPLAT_TIME;	// set life of splat

		InitYOffset(newObj, -15);
		newObj->DZ = -0x80000L-MyRandomShort();						// start bouncing up
		newObj->DX = ((long)MyRandomShort()*4)-0x10000L;				// random vector
		newObj->DY = ((long)MyRandomShort()*4)-0x10000L;
	}

					/* MAKE MESSAGE */

	if (gMyNodePtr->OwnerToMessageNode == nil)
	{
		if (!(MyRandomLong()&0b1100))
			MakeMikeMessage(MESSAGE_NUM_TAKETHAT+(gFrames&1));
	}
}


/**************** MOVE ENEMY SPLAT ********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveEnemySplat(void)
{
register	ObjNode		*theNode;

	theNode = gThisNodePtr;

	if (--theNode->SplatTimer <= 0)							// see if timed out
	{
		DeleteObject(theNode);
		return;
	}

	theNode->YOffset.L += (theNode->DZ += 0x10000L);		// gravity & move it
	if (theNode->YOffset.Int > -1)							// see if landed
	{
		DeleteObject(theNode);
		return;
	}

	theNode->X.L += theNode->DX;							// move x/y
	theNode->Y.L += theNode->DY;
}


/********************* DELETE ENEMY ********************/

void DeleteEnemy(ObjNode *theEnemy)
{
	DeleteObject(theEnemy);										// delete the enemy
	gNumEnemies--;
}

/***************** TRACK ENEMY **********************/
//
// See if enemy is out of item window range
//

Boolean TrackEnemy(void)
{
register ObjNode	*theNode;

	theNode = gThisNodePtr;

	if ((theNode->X.Int < gItemDeleteWindow_Left) ||				// check bounds
		(theNode->X.Int > gItemDeleteWindow_Right) ||
		(theNode->Y.Int < gItemDeleteWindow_Top) ||
		(theNode->Y.Int > gItemDeleteWindow_Bottom))
	{
		DeleteEnemy(theNode);
		return(true);
	}
	return(false);
}


/***************** TRACK ENEMY 2 **********************/
//
// CHECKS A WIDER RANGE
//

Boolean TrackEnemy2(void)
{
register ObjNode	*theNode;

	theNode = gThisNodePtr;

	if ((theNode->X.Int < (gItemDeleteWindow_Left-300)) ||				// check bounds
		(theNode->X.Int > (gItemDeleteWindow_Right+300)) ||
		(theNode->Y.Int < (gItemDeleteWindow_Top-300)) ||
		(theNode->Y.Int > (gItemDeleteWindow_Bottom+300)))
	{
		DeleteEnemy(theNode);
		return(true);
	}
	return(false);
}


/********************* MOVE FROZEN ENEMY *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveFrozenEnemy(void)
{
	if (TrackEnemy())									// see if out of range
		return;

	GetObjectInfo();

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	UpdateObject();
}







