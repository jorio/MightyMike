/****************************/
/*    	ENEMY_CAVEMAN       */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/****************************/
/*    EXTERNALS             */
/****************************/
#include "myglobals.h"
#include "windows.h"
#include "shape.h"
#include "playfield.h"
#include "enemy.h"
#include "object.h"
#include "misc.h"
#include "miscanims.h"
#include "sound2.h"
#include "objecttypes.h"
#include "collision.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY,gFrames;
extern	union_gX;
extern	union_gY;
extern	CollisionRec	gCollisionList[];
extern	long				gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	long				gMyX,gMyY;
extern	short				gNumEnemies,gSoundNum_UngaBunga;
extern	ObjNode			*gMyNodePtr;
extern	short			gEnemyFreezeTimer;

/****************************/
/*    CONSTANTS             */
/****************************/

#define	CAVEMAN_MAX_SPEED	0x20000L
#define	CAVEMAN_ACCEL		0x2000L
#define	CAVEMAN_HEALTH		2
#define	CAVEMAN_WORTH		1
#define CAVEMAN_DAMAGE_THRESHOLD	1

#define STONE_WHEEL_SPEED	0x68000L

#define	BONE_MIN_DIST		60
#define	BONE_MAX_DIST		250
#define	BONE_SPEED			0x70000L

enum
{
	SUB_WALK_RIGHT,
	SUB_WALK_LEFT,
	SUB_ROLL_RIGHT,
	SUB_ROLL_LEFT,
	SUB_THROW_RIGHT,
	SUB_THROW_LEFT,
	SUB_UNGA
};

#define	UNGA_DELAY	(GAME_FPS*4)

/**********************/
/*     VARIABLES      */
/**********************/

#define	RollFlag	Flag0
#define AimFlag		Flag1
#define ThrowFlag 	Flag0

long	gLastUngaTime = 0;

/************************ ADD ENEMY: CAVEMAN ********************/

Boolean AddEnemy_Caveman(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;
Byte		animNum;


	switch(itemPtr->parm[0])						// see which kind of caveman to create
	{
		case 0:
						/***********************/
						/* ADD WALKING CAVEMAN */
						/***********************/

			if (gNumEnemies >= MAX_ENEMIES)			// check # enemies
				return(false);

			newObj = MakeNewShape(GroupNum_Caveman,ObjType_Caveman,SUB_WALK_RIGHT,itemPtr->x,
								itemPtr->y,50,MoveCaveman_Walker,PLAYFIELD_RELATIVE);
			if (newObj == nil)
				return(false);

			CalcEnemyScatterOffset(newObj);

			newObj->DY = 0x10000L;			// start going down (for cave entrances)
			break;

		case 1:
					/*****************************/
					/* ADD BONE THROWING CAVEMAN */
					/*****************************/

			if (gNumEnemies >= MAX_ENEMIES)			// check # enemies
				return(false);

			newObj = MakeNewShape(GroupNum_Caveman,ObjType_Caveman,SUB_WALK_RIGHT,itemPtr->x,
								itemPtr->y,50,MoveCaveman_Thrower,PLAYFIELD_RELATIVE);
			if (newObj == nil)
				return(false);

			CalcEnemyScatterOffset(newObj);
			newObj->ThrowFlag = false;

			break;

		case 2:
					/*****************************/
					/* ADD WHEEL ROLLING CAVEMAN */
					/*****************************/

			if (itemPtr->parm[1])					// see which way to aim
				animNum = SUB_ROLL_LEFT;
			else
				animNum = SUB_ROLL_RIGHT;

			newObj = MakeNewShape(GroupNum_Caveman,ObjType_Caveman,animNum,itemPtr->x,
								itemPtr->y,50,MoveCaveman_Roller,PLAYFIELD_RELATIVE);
			if (newObj == nil)
				return(false);

			newObj->RollFlag = false;
			newObj->AimFlag = itemPtr->parm[1];

			break;


		default:									// put this here just in case get bad input parameter
			return(false);
	}


				/* SET STANDARD STUFF */

	newObj->ItemIndex = itemPtr;				// remember where this came from
	newObj->CType = CTYPE_ENEMYA;				// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = CAVEMAN_HEALTH;			// set health
	newObj->TopOff = -22;						// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -14;
	newObj->RightOff = 14;
	CalcObjectBox2(newObj);
	newObj->Worth = CAVEMAN_WORTH;				// set worth
	newObj->InjuryThreshold = CAVEMAN_DAMAGE_THRESHOLD;


	gNumEnemies++;

	return(true);								// was added
}


/********************* MOVE CAVEMAN: WALKER *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveCaveman_Walker(void)
{
	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	if (TrackEnemy())									// see if out of range
		return;

	DoUngaBunga();

	GetObjectInfo();

	DoCavemanMove();

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	UpdateCaveman();
}


/********************* MOVE CAVEMAN: THROWER *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveCaveman_Thrower(void)
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

	DoCavemanMove();

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	distX = Absolute(gX.Int - gMyX);					// see if at range
	if ((distX > BONE_MIN_DIST) && (distX < BONE_MAX_DIST))
	{
		distY = Absolute(gY.Int - gMyY);
		if ((distY > BONE_MIN_DIST) && (distY < BONE_MAX_DIST))
		{
			if (!(MyRandomLong()&b111111))
			{
				if (gThisNodePtr->SubType == 0)
					SwitchAnim(gThisNodePtr,SUB_THROW_RIGHT);
				else
					SwitchAnim(gThisNodePtr,SUB_THROW_LEFT);
				gDY = gDX = 0;
				gThisNodePtr->MoveCall = MoveCaveman_Thrower2;
				UpdateEnemy();
				return;
			}
		}
	}


				/* SEE IF START TO THROW */

	UpdateCaveman();
}


/********************* MOVE CAVEMAN: THROWER2 *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//
// This is called WHILE it is throwing

void MoveCaveman_Thrower2(void)
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
		ThrowABone();
	}


	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))	// returns true if died
		return;

	if (gThisNodePtr->SubType < SUB_THROW_RIGHT)
		gThisNodePtr->MoveCall = MoveCaveman_Thrower;

	UpdateEnemy();
}



/********************* MOVE CAVEMAN: ROLLER *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveCaveman_Roller(void)
{
	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	if (TrackEnemy())									// see if out of range
		return;

	GetObjectInfo();

				/* DO ENEMY COLLISION */

	if (DoEnemyCollisionDetect(ENEMY_NO_BG_COLLISION))	// returns true if died
		return;

	if (gThisNodePtr->RollFlag)							// see if roll a wheel
	{
		gThisNodePtr->RollFlag = false;
		RollWheel();
	}
}


/**************** UPDATE CAVEMAN *******************/

void UpdateCaveman(void)
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

/************** DO CAVEMAN MOVE ******************/

void DoCavemanMove(void)
{

				/* ACCEL TOWARD ME */

	if (gX.Int < (gMyX+gThisNodePtr->EnemyTargetXOff))
		gDX += CAVEMAN_ACCEL;
	else
	if (gX.Int > (gMyX+gThisNodePtr->EnemyTargetXOff))
		gDX -= CAVEMAN_ACCEL;

	if (gY.Int > (gMyY+gThisNodePtr->EnemyTargetYOff))
		gDY -= CAVEMAN_ACCEL;
	else
	if (gY.Int < (gMyY+gThisNodePtr->EnemyTargetYOff))
		gDY += CAVEMAN_ACCEL;

				/* CHECK MAX DELTAS */

	if (gDX > CAVEMAN_MAX_SPEED)
		gDX -= CAVEMAN_ACCEL;
	else
	if (gDX < -CAVEMAN_MAX_SPEED)
		gDX +=  CAVEMAN_ACCEL;

	if (gDY > CAVEMAN_MAX_SPEED)
		gDY -=  CAVEMAN_ACCEL;
	else
	if (gDY < -CAVEMAN_MAX_SPEED)
		gDY +=  CAVEMAN_ACCEL;

					/* MOVE IT */
	gX.L += gDX;
	gY.L += gDY;

}


/*================================= WHEEL ============================================*/

/**************** ROLL WHEEL ********************/
//
// gThisNodePtr = ptr to caveman doing the rolling
//

void RollWheel(void)
{
register	ObjNode		*newObj;
register	Byte	animNum;
short			x;
long		dx;

	if (gThisNodePtr->AimFlag)
	{
		animNum = 1;							// aim left
		x = gThisNodePtr->X.Int - 50;
		dx = -STONE_WHEEL_SPEED;
	}
	else
	{
		animNum = 0;							// aim right
		x = gThisNodePtr->X.Int + 50;
		dx = STONE_WHEEL_SPEED;
	}


	newObj = MakeNewShape(GroupNum_StoneWheel,ObjType_StoneWheel,animNum,x,
						gThisNodePtr->Y.Int,gThisNodePtr->Z,MoveStoneWheel,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return;

	newObj->DX = dx+(RandomRange(0,10000)<<3-40000L);

	newObj->CType = CTYPE_ENEMYC;				// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->TopOff = -14;						// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -20;
	newObj->RightOff = 20;
	CalcObjectBox2(newObj);
}


/************** MOVE STONE WHEEL ******************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveStoneWheel(void)
{
	if (TrackItem())							// see if out of range
	{
		DeleteObject(gThisNodePtr);
		return;
	}

	GetObjectInfo();

	gX.L += gDX;								// move it

	gSumDX = gDX;								// see if hit wall
	gSumDY = 0;
	CalcObjectBox();
	if (HandleCollisions(CTYPE_BGROUND))
	{
		DeleteObject(gThisNodePtr);
		return;
	}

	UpdateObject();
}

/*=========================================== BONE ================================================*/

/*********** THROW A BONE ***************/
//
//

void ThrowABone(void)
{
register	ObjNode		*newObj;
short		x,fudgeX;
long		dx;
unsigned short	bits;

				/* GET INFO */

	if (gThisNodePtr->SubType == SUB_THROW_RIGHT)
	{
		x = gThisNodePtr->X.Int + 30;
		dx = BONE_SPEED;
	}
	else
	{
		x = gThisNodePtr->X.Int - 30;
		dx = -BONE_SPEED;
	}

			/* SEE IF MATERIALIZE INSIDE SOLID OBJECT */

	if (DoPointCollision(x,gThisNodePtr->Y.Int,CTYPE_MISC))			// check sprites
		return;

	bits = GetMapTileAttribs(x,gThisNodePtr->Y.Int);
	if (bits & b1111)												// check if solid at all
		if (!(bits & TILE_ATTRIB_BULLETGOESTHRU))						// see if go thru anyway
			return;


				/* CREATE BONE */

	newObj = MakeNewShape(GroupNum_Bone,ObjType_Bone,0,x,
						gThisNodePtr->Y.Int,gThisNodePtr->Z,MoveBone,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return;

	fudgeX = (MyRandomLong()&0x7f) - 0x40;		// fudge factor for inaccurate throws - make it easier
	newObj->DX = dx + ((MyRandomLong()&0x7f) - 0x40);
	newObj->DY = (long)(gMyY - gY.Int + fudgeX) * 3000L;

	newObj->CType = CTYPE_ENEMYB;				// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->TopOff = -25;						// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -15;
	newObj->RightOff = 15;
	CalcObjectBox2(newObj);

	newObj->YOffset.Int = -40;


			/* MAKE SHADOW */

	newObj->ShadowIndex = MakeShadow(newObj,SHADOWSIZE_SMALL);	// allocate shadow

}


/****************** MOVE BONE *********************/
//
// gThisNodePtr = ptr to caveman doing the rolling
//

void MoveBone(void)
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
	if (DoPointCollision(gX.Int,gY.Int,CTYPE_MISC) ||
		(GetMapTileAttribs(gX.Int,gY.Int)&ALL_SOLID_SIDES))
	{
		DeleteObject(gThisNodePtr);
		return;
	}

	UpdateObject();
}


/**************** DO UNGA BUNGA *****************/
//
//
//

void DoUngaBunga(void)
{
register	ObjNode *newObj;

	if ((gFrames-gLastUngaTime) < GAME_FPS)			// see if been enough time
		return;

	if (gThisNodePtr->OwnerToMessageNode != nil)		// see if already has a message
		return;
	if (gMyNodePtr->OwnerToMessageNode != nil)			// not if Mike is talking
		return;
	if (MyRandomLong()&b1111111111)						// random
		return;

				/* MAKE MESSAGE SPRITE */

	newObj = MakeNewShape(GroupNum_Caveman,ObjType_Caveman,SUB_UNGA,
				gThisNodePtr->X.Int,gThisNodePtr->Y.Int,gThisNodePtr->Z,MoveMessage,
				PLAYFIELD_RELATIVE);

	if (newObj == nil)
		return;

	newObj->YOffset.Int = -65;
	newObj->MessageTimer = GAME_FPS*3/2;				// set message timer
	newObj->TileMaskFlag = false;						// wont be tile masked
	newObj->MessageToOwnerNode = gThisNodePtr;			// point to owner
	gThisNodePtr->OwnerToMessageNode = newObj;			// point to message

	PlaySound(gSoundNum_UngaBunga);

	gLastUngaTime = gFrames;							// remember when it occurred
}




