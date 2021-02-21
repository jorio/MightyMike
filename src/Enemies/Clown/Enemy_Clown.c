/****************************/
/*    	ENEMY_CLOWN         */
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
#include "enemy2.h"
#include "objecttypes.h"
#include "collision.h"
#include "sound2.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY,gFrames;
extern	union_gX;
extern	union_gY;
extern	CollisionRec	gCollisionList[];
extern	long				gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	long				gMyX,gMyY;
extern	short				gNumEnemies;
extern	ObjNode			*gMyNodePtr;
extern	short			gEnemyFreezeTimer,gSoundNum_ClownLaugh;

/****************************/
/*    CONSTANTS             */
/****************************/

#define	CLOWN_MAX_SPEED	0x21000L
#define	CLOWN_ACCEL		0x2000L

#define	CLOWN_HEALTH			1
#define	CLOWN_WORTH				2
#define CLOWN_DAMAGE_THRESHOLD	1

#define STONE_WHEEL_SPEED	0x68000L

#define	PIE_MAX_DIST		250
#define	PIE_SPEED			0x80000L

enum
{
	SUB_WALK_RIGHT,
	SUB_WALK_LEFT,
	SUB_THROW_RIGHT,
	SUB_THROW_LEFT,
	SUB_PIE_RIGHT,
	SUB_PIE_LEFT,
	SUB_LAUGH
};

/**********************/
/*     VARIABLES      */
/**********************/

#define ThrowFlag 	Flag0

long	gLastClownLaughTime;


/************************ ADD ENEMY: CLOWN ********************/

Boolean AddEnemy_Clown(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	switch(itemPtr->parm[0])						// see which kind of clown to create
	{
		case 0:
						/***********************/
						/* ADD WALKING CLOWN   */
						/***********************/

			if (gNumEnemies >= MAX_ENEMIES)			// check # enemies
				return(false);

			newObj = MakeNewShape(GroupNum_Clown,ObjType_Clown,SUB_WALK_RIGHT,itemPtr->x,
								itemPtr->y,50,MoveClown_Walker,PLAYFIELD_RELATIVE);
			if (newObj == nil)
				return(false);

			CalcEnemyScatterOffset(newObj);
			break;

		case 1:
					/*****************************/
					/* ADD PIE THROWING CLOWN 	 */
					/*****************************/

			if (gNumEnemies >= MAX_ENEMIES)			// check # enemies
				return(false);

			newObj = MakeNewShape(GroupNum_Clown,ObjType_Clown,SUB_WALK_RIGHT,itemPtr->x,
								itemPtr->y,50,MoveClown_PieThrower,PLAYFIELD_RELATIVE);
			if (newObj == nil)
				return(false);

			CalcEnemyScatterOffset(newObj);
			newObj->ThrowFlag = false;

			break;

		default:									// put this here just in case get bad input parameter
			return(false);
	}


				/* SET STANDARD STUFF */

	newObj->ItemIndex = itemPtr;				// remember where this came from
	newObj->CType = CTYPE_ENEMYA;				// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = CLOWN_HEALTH;				// set health
	newObj->TopOff = -22;						// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -14;
	newObj->RightOff = 14;
	CalcObjectBox2(newObj);
	newObj->Worth = CLOWN_WORTH;				// set worth
	newObj->InjuryThreshold = CLOWN_DAMAGE_THRESHOLD;

	gNumEnemies++;

	return(true);								// was added
}


/********************* MOVE CLOWN: WALKER *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveClown_Walker(void)
{
	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	if (TrackEnemy())									// see if out of range
		return;

	GetObjectInfo();

	DoClownMove();
	DoClownLaugh();

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	UpdateClown();
}


/********************* MOVE CLOWN: PIE THROWER *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveClown_PieThrower(void)
{
short		distX,distY;
	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	if (TrackEnemy())									// see if out of range
		return;

	GetObjectInfo();

	DoClownMove();

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	distX = Absolute(gX.Int - gMyX);					// see if at range
	if (distX < PIE_MAX_DIST)
	{
		distY = Absolute(gY.Int - gMyY);
		if (distY < PIE_MAX_DIST)
		{
			if (!(MyRandomLong()&b1111111))
			{
				if (gThisNodePtr->SubType == SUB_WALK_RIGHT)
					SwitchAnim(gThisNodePtr,SUB_THROW_RIGHT);
				else
					SwitchAnim(gThisNodePtr,SUB_THROW_LEFT);
				gDY = gDX = 0;
				gThisNodePtr->MoveCall = MoveClown_PieThrower2;
				UpdateEnemy();
				return;
			}
		}
	}

				/* SEE IF START TO THROW */

	UpdateClown();
}


/********************* MOVE CLOWN: PIE THROWER2 *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//
// This is called WHILE it is throwing

void MoveClown_PieThrower2(void)
{
	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}
	if (TrackEnemy())									// see if out of range
		return;

	GetObjectInfo();

	if (gThisNodePtr->ThrowFlag)
	{
		gThisNodePtr->ThrowFlag = false;
		ThrowClownPie();
	}

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	if (gThisNodePtr->SubType < SUB_THROW_RIGHT)
		gThisNodePtr->MoveCall = MoveClown_PieThrower;

	UpdateEnemy();
}


/**************** UPDATE CLOWN *******************/

void UpdateClown(void)
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


/************** DO CLOWN MOVE ******************/

void DoClownMove(void)
{

				/* ACCEL TOWARD ME */

	if (gX.Int < (gMyX+gThisNodePtr->EnemyTargetXOff))
		gDX += CLOWN_ACCEL;
	else
	if (gX.Int > (gMyX+gThisNodePtr->EnemyTargetXOff))
		gDX -= CLOWN_ACCEL;

	if (gY.Int > (gMyY+gThisNodePtr->EnemyTargetYOff))
		gDY -= CLOWN_ACCEL;
	else
	if (gY.Int < (gMyY+gThisNodePtr->EnemyTargetYOff))
		gDY += CLOWN_ACCEL;

				/* CHECK MAX DELTAS */

	if (gDX > CLOWN_MAX_SPEED)
		gDX -= CLOWN_ACCEL;
	else
	if (gDX < -CLOWN_MAX_SPEED)
		gDX +=  CLOWN_ACCEL;

	if (gDY > CLOWN_MAX_SPEED)
		gDY -=  CLOWN_ACCEL;
	else
	if (gDY < -CLOWN_MAX_SPEED)
		gDY +=  CLOWN_ACCEL;

					/* MOVE IT */
	gX.L += gDX;
	gY.L += gDY;
}

/*=========================================== PIE ================================================*/

/*********** THROW A PIE ***************/
//
//

void ThrowClownPie(void)
{
register	ObjNode		*newObj;
short			x,sub;
long		dx;

				/* GET INFO */

	if (gThisNodePtr->SubType == SUB_THROW_RIGHT)
	{
		x = gThisNodePtr->X.Int + 30;				// throw right
		dx = PIE_SPEED;
		sub = SUB_PIE_RIGHT;
	}
	else
	{
		x = gThisNodePtr->X.Int - 30;				// throw left
		dx = -PIE_SPEED;
		sub = SUB_PIE_LEFT;
	}

				/* CREATE PIE */

	newObj = MakeNewShape(GroupNum_Clown,ObjType_Clown,sub,x,
						gThisNodePtr->Y.Int,gThisNodePtr->Z,MoveClownPie,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return;

	newObj->DX = dx;
	newObj->DY = (long)(gMyY - gY.Int) * 3000L;

	newObj->CType = CTYPE_ENEMYB;				// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->TopOff = -15;						// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -15;
	newObj->RightOff = 15;
	CalcObjectBox2(newObj);

	newObj->YOffset.Int = -40;


			/* MAKE SHADOW */

	newObj->ShadowIndex = MakeShadow(newObj,SHADOWSIZE_MEDIUM);	// allocate shadow

}


/****************** MOVE PIE *********************/
//
// gThisNodePtr = ptr to caveman doing the rolling
//

void MoveClownPie(void)
{

	if (TrackItem())							// see if out of range
	{
		DeleteObject(gThisNodePtr);
		return;
	}

	GetObjectInfo();

	gX.L += gDX;								// move it
	gY.L += gDY;

	gSumDX = gDX;								// see if hit wall
	gSumDY = gDY;
	CalcObjectBox();
	if (HandleCollisions(CTYPE_MISC) || (GetMapTileAttribs(gX.Int,gY.Int)&ALL_SOLID_SIDES))
	{
		DeleteObject(gThisNodePtr);
		return;
	}

	UpdateObject();
}


/**************** DO CLOWN LAUGH *****************/

void DoClownLaugh(void)
{
register	ObjNode *newObj;

	if ((gFrames-gLastClownLaughTime) < GAME_FPS)		// see if been enough time
		return;

	if (gThisNodePtr->OwnerToMessageNode != nil)		// see if already has a message
		return;
	if (gMyNodePtr->OwnerToMessageNode != nil)			// not if Mike is talking
		return;
	if (MyRandomLong()&b11111111)						// random
		return;

				/* MAKE MESSAGE SPRITE */

	newObj = MakeNewShape(GroupNum_Clown,ObjType_Clown,SUB_LAUGH,
				gThisNodePtr->X.Int,gThisNodePtr->Y.Int,gThisNodePtr->Z,MoveMessage,
				PLAYFIELD_RELATIVE);

	if (newObj == nil)
		return;

	newObj->MessageTimer = GAME_FPS*3/2;				// set message timer
	newObj->TileMaskFlag = false;						// wont be tile masked
	newObj->MessageToOwnerNode = gThisNodePtr;			// point to owner
	gThisNodePtr->OwnerToMessageNode = newObj;			// point to message

	PlaySound(gSoundNum_ClownLaugh);

	gLastClownLaughTime = gFrames;						// remember when it occurred
}

