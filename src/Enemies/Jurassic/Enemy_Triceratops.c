/****************************/
/*    	ENEMY_TRICERATOPS   */
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
#include "shape.h"
#include "object.h"
#include "misc.h"
#include "objecttypes.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY;
extern	MikeFixed		gX;
extern	MikeFixed		gY;
extern	CollisionRec	gCollisionList[];
extern	long				gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	long				gMyX,gMyY;
extern	short				gNumEnemies;
extern	short			gEnemyFreezeTimer;


/****************************/
/*    CONSTANTS             */
/****************************/

#define	TRICERATOPS_HEALTH		7
#define TRI_WORTH				6
#define	TRI_DAMAGE_THRESHOLD	3

#define	CHARGING_RANGE_X		300
#define	CHARGING_RANGE_Y		90
#define	TRI_CHARGE_SPEED		0x70000L

/**********************/
/*     VARIABLES      */
/**********************/

#define TriceratopsAim		Special1		// 0 = right, 1 = left
#define	TriceratopsDist		Special2


/************************ ADD ENEMY: TRICERATOPS ********************/

Boolean AddEnemy_Triceratops(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;
short			animNum;

//	if (gNumEnemies >= MAX_ENEMIES)					// check # enemies
//		return(false);


	animNum = itemPtr->parm[0];						// see which way to aim him

	newObj = MakeNewShape(GroupNum_Triceratops,ObjType_Triceratops,animNum,
			itemPtr->x,itemPtr->y,50,MoveTriceratops,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CType = CTYPE_ENEMYA;					// set collision info
	newObj->CBits = CBITS_TOUCHABLE;
	newObj->Health = TRICERATOPS_HEALTH;			// set health

	newObj->TopOff = -22;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -30;
	newObj->RightOff = 30;
	CalcObjectBox2(newObj);

	newObj->Worth = TRI_WORTH;						// set worth
	newObj->InjuryThreshold = TRI_DAMAGE_THRESHOLD;

	newObj->BaseX = itemPtr->x;						// remember starting X
	newObj->TriceratopsAim = animNum;				// set aim flag

	newObj->TriceratopsDist = itemPtr->parm[1]*TILE_SIZE;	// # pixels to move it

	gNumEnemies++;

	return(true);									// was added
}


/********************* MOVE TRICERATOPS *********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveTriceratops(void)
{
static	void(*moveTable[])(void) =
				{
					MoveTri_Waiting,			// wait right
					MoveTri_Waiting,			// wait left
					MoveTri_Charging,			// charge right
					MoveTri_Charging			// charge left
				};

	if (gEnemyFreezeTimer)					// see if frozen
	{
		MoveFrozenEnemy();
		return;
	}

	GetObjectInfo();
	moveTable[gThisNodePtr->SubType]();
}


/**************** MOVE TRI: WAITING ********************/
//
// Waiting to charge at me.
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveTri_Waiting(void)
{
short		distX,distY;


	if (TrackEnemy())									// see if out of range
		return;

	distX = gMyX-gX.Int;								// calc dist to me
	distY = Absolute(gMyY-gY.Int);


					/* SEE IF IN RANGE TO CHARGE */

	if (distY < CHARGING_RANGE_Y)
	{
		if (gThisNodePtr->TriceratopsAim)
		{
			if ((distX < 0) && (distX > -CHARGING_RANGE_X))	// check left
				goto start_charge;
		}
		else
			if ((distX > 0) && (distX < CHARGING_RANGE_X))	// check right
				goto start_charge;
	}
	goto no_charge;


					/* START TO CHARGE */
start_charge:
	SwitchAnim(gThisNodePtr,gThisNodePtr->SubType+2);		// do correct anim



no_charge:
				/* DO ENEMY COLLISION */

	if (DoEnemyCollisionDetect(FULL_ENEMY_COLLISION))		// returns true if died
		return;


					/* UPDATE */

	UpdateEnemy();
}


/**************** MOVE TRI: CHARGING ********************/
//
// Waiting to charge at me.
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveTri_Charging(void)
{
	if (TrackEnemy())									// see if out of range
		return;

	if (gThisNodePtr->TriceratopsAim)					// set deltas
	{
		gDX = -TRI_CHARGE_SPEED;
	}
	else
	{
		gDX = TRI_CHARGE_SPEED;
	}

	gX.L += gDX;										// do the move

	if (Absolute(gThisNodePtr->BaseX - gX.Int) > gThisNodePtr->TriceratopsDist )	// see if @ end of charge
	{
		DeleteEnemy(gThisNodePtr);						// delete when done
		return;
	}

					/* UPDATE */

	UpdateEnemy();
}





