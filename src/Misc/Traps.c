/****************************/
/*    	TRAPS               */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/****************************/
/*    EXTERNALS             */
/****************************/
#include "myglobals.h"
#include "window.h"
#include "enemy.h"
#include "enemy2.h"
#include "enemy3.h"
#include "enemy4.h"
#include "object.h"
#include "traps.h"
#include "shape.h"
#include "playfield.h"
#include "misc.h"
#include "myguy.h"
#include "objecttypes.h"
#include "miscanims.h"
#include "sound2.h"
#include "externs.h"

/****************************/
/*    CONSTANTS             */
/****************************/


#define		PLANT_TOP			-16
#define		PLANT_SIDE			20
#define		PLANT_KILL_DISTX	60
#define		PLANT_KILL_DISTY	40


#define		MAX_MUFFIT_SPIDERS	3

enum{
	MUFFIT_SUB_SIT,
	MUFFIT_SUB_SCARE
};


/**********************/
/*     VARIABLES      */
/**********************/

#define	SproingFinishedFlag	Flag0
#define	CandyMPlatSpeed	Special1
#define	HydrantDirection Special1
#define WaterFallDelta		Special2

/********************** ADD APPEAR ZONE *********************/

Boolean AddAppearZone(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	newObj = MakeNewObject(BG_GENRE,itemPtr->x,itemPtr->y,NEAREST_Z,MoveAppearZone);

	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;			// remember where this came from

	return(true);											// was added
}


/****************** MOVE APPEAR ZONE ******************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveAppearZone(void)
{
ObjectEntryType	item;

	if (TrackItem())									// see if out of range
	{
		DeleteObject(gThisNodePtr);
		return;
	}

	if (gEnemyFreezeTimer)					// see if frozen
		return;


	if (!(MyRandomLong()&(0b11111)))								// see if create enemy
	{

		item.x = gThisNodePtr->X.Int;
		item.y = gThisNodePtr->Y.Int;
		item.parm[0] = 0;

		switch(gSceneNum)
		{
			case	SCENE_JURASSIC:
					if (AddEnemy_Caveman(&item))
						gMostRecentlyAddedNode->ItemIndex = nil;	// trick it
					break;

			case	SCENE_CANDY:
					if (AddEnemy_Mint(&item))
						gMostRecentlyAddedNode->ItemIndex = nil;	// trick it
					break;

			case	SCENE_CLOWN:
					if (AddEnemy_FlowerClown(&item))
						gMostRecentlyAddedNode->ItemIndex = nil;	// trick it
					break;

			case	SCENE_FAIRY:


	  				break;
		}
	}
}


//============================================================================================

/*************** ADD MAN EATING PLANT ******************/

Boolean AddManEatingPlant(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	newObj = MakeNewShape(GroupNum_ManEatingPlant,ObjType_ManEatingPlant,0,
			itemPtr->x,itemPtr->y,50,MoveManEatingPlant,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CType = CTYPE_MISC;						// set collision info
	newObj->CBits = CBITS_ALLSOLID;

	newObj->TopOff = PLANT_TOP;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -PLANT_SIDE;
	newObj->RightOff = PLANT_SIDE;
	CalcObjectBox2(newObj);

	newObj->Flag0 = false;							// clear the action flag
	newObj->Flag1 = false;							// clear the POD flag

	return(true);									// was added
}

/******************** MOVE MAN EATING PLANT ******************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveManEatingPlant(void)
{
ObjNode	*newObj;


	if (TrackItem())									// see if out of range
	{
		DeleteObject(gThisNodePtr);
		return;
	}

	if (gEnemyFreezeTimer)					// see if frozen
		return;


	if (gThisNodePtr->SubType == 0)						// see if BLOOMING
	{
		if (gThisNodePtr->Flag0)						// see if ready to spike
		{
			gThisNodePtr->Flag0 = false;
			SwitchAnim(gThisNodePtr,1);					// make spike
			gThisNodePtr->CType = CTYPE_ENEMYC;			// make harmful
		}
	}
	else												// else SPIKING
	{
		if ((Absolute(gThisNodePtr->X.Int - gMyX) < PLANT_KILL_DISTX) &&	// see if hurt me
			(Absolute(gThisNodePtr->Y.Int - gMyY) < PLANT_KILL_DISTY))
		{
			IGotHurt();
		}

		if (gThisNodePtr->Flag0)						// see if done
		{
			gThisNodePtr->Flag0 = false;
			SwitchAnim(gThisNodePtr,0);					// make bloom
			gThisNodePtr->CType = CTYPE_MISC;
		}
	}

				/* SEE IF SHOOT POD */

	if (gThisNodePtr->Flag1)
	{
		gThisNodePtr->Flag1 = false;

		newObj = MakeNewShape(GroupNum_ManEatingPlant,ObjType_ManEatingPlant,2,
							gThisNodePtr->X.Int,gThisNodePtr->Y.Int+1,gThisNodePtr->Z,
							MovePlantPod,PLAYFIELD_RELATIVE);
		if (newObj != nil)
		{
			newObj->CType = CTYPE_ENEMYB;				// set collision info
			newObj->CBits = CBITS_TOUCHABLE;
			newObj->TopOff = -8;						// set box
			newObj->BottomOff = 0;
			newObj->LeftOff = -8;
			newObj->RightOff = 8;
			CalcObjectBox2(newObj);

			InitYOffset(newObj, -42);
			newObj->DZ = -0x90000L;						// start bouncing up

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


/******************* MOVE PLANT POD **********************/

void MovePlantPod(void)
{
register	ObjNode		*theNode;

	theNode = gThisNodePtr;

	theNode->YOffset.L += (theNode->DZ += 0x0d000L);		// gravity & move it
	if (theNode->YOffset.Int > -1)							// see if landed
	{
		DeleteObject(theNode);
		return;
	}

	theNode->X.L += theNode->DX;							// move x/y
	theNode->Y.L += theNode->DY;

	CalcObjectBox2(theNode);
}


//============================================================================================

/*************** ADD JACKINTHEBOX ******************/

Boolean AddJackInTheBox(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	newObj = MakeNewShape(GroupNum_JackInTheBox,ObjType_JackInTheBox,1,
			itemPtr->x,itemPtr->y,50,MoveJackInTheBox,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CType = 0;								// set collision info
	newObj->CBits = CBITS_TOUCHABLE;

	newObj->TopOff = -16;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -10;
	newObj->RightOff = 10;
	CalcObjectBox2(newObj);

	newObj->DrawFlag = false;						// start as invisible

	newObj->SproingFinishedFlag = false;

	return(true);									// was added
}

/******************** MOVE JACKINTHEBOX ******************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveJackInTheBox(void)
{
	if (TrackItem())									// see if out of range
	{
		DeleteObject(gThisNodePtr);
		return;
	}

	if (gEnemyFreezeTimer)								// see if frozen
		return;

	if (gThisNodePtr->SubType == 0)						// see if SPROINGING
	{
		if (gThisNodePtr->SproingFinishedFlag)
		{
			SwitchAnim(gThisNodePtr,1);					// all done, go back to normal
			gThisNodePtr->CType = 0;
			gThisNodePtr->DrawFlag = false;
			gThisNodePtr->SproingFinishedFlag = false;
		}
	}
	else												// else SEE IF READY TO SPROING
	{
		if (!(MyRandomLong()&0b111111))
		{
			gThisNodePtr->CType = CTYPE_ENEMYC;			// make harmful
			gThisNodePtr->DrawFlag = true;
			SwitchAnim(gThisNodePtr,0);
			PlaySound(gSoundNum_JackInTheBox);
		}
	}
}



//============================================================================================

/*************** ADD CandyMPlatform ******************/

Boolean AddCandyMPlatform(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	newObj = MakeNewShape(GroupNum_CandyMPlatform,ObjType_CandyMPlatform,0,
			itemPtr->x,itemPtr->y,FARTHEST_Z,MoveCandyMPlatform,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CType = CTYPE_MPLATFORM;				// set collision info
	newObj->CBits = 0;

	newObj->TopOff = -40;							// set box
	newObj->BottomOff = 40;
	newObj->LeftOff = -40;
	newObj->RightOff = 40;
	CalcObjectBox2(newObj);

	newObj->CandyMPlatSpeed = (long)itemPtr->parm[0]<<16;		// set speed

	return(true);									// was added
}

/******************** MOVE CandyMPlatform ******************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveCandyMPlatform(void)
{
	if (TrackItem())									// see if out of range
	{
		DeleteObject(gThisNodePtr);
		return;
	}

	GetObjectInfo();

	MoveOnPath(gThisNodePtr->CandyMPlatSpeed,false);
	CalcObjectBox();
	UpdateObject();
}


//============================================================================================

/*************** ADD STAR ******************/

Boolean AddStar(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	newObj = MakeNewShape(GroupNum_Star,ObjType_Star,0,
			itemPtr->x,itemPtr->y,FARTHEST_Z,SimpleObjectMove,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CType = CTYPE_ENEMYB;					// set collision info
	newObj->CBits = CBITS_ALLSOLID;

	newObj->TopOff = -10;							// set box
	newObj->BottomOff = 10;
	newObj->LeftOff = -10;
	newObj->RightOff = 10;
	CalcObjectBox2(newObj);

	return(true);									// was added
}


//============================================================================================

/*************** ADD GumBall ******************/

Boolean AddGumBall(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	newObj = MakeNewShape(GroupNum_GumBall,ObjType_GumBall,0,
			itemPtr->x,itemPtr->y,100,MoveGumBall,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CType = CTYPE_ENEMYC;					// set collision info
	newObj->CBits = CBITS_TOUCHABLE;

	newObj->TopOff = -40;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -15;
	newObj->RightOff = 15;
	CalcObjectBox2(newObj);

	return(true);									// was added
}


/******************** MOVE GumBall ******************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveGumBall(void)
{
	if (TrackItem())									// see if out of range
	{
		DeleteObject(gThisNodePtr);
		return;
	}

	GetObjectInfo();

	MoveOnPath(0x50000L,false);
	CalcObjectBox();
	UpdateObject();
}



//========================================================================================


/*************** ADD MUFFIT ******************/

Boolean AddMuffit(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	newObj = MakeNewShape(GroupNum_Muffit,ObjType_Muffit,MUFFIT_SUB_SIT,
			itemPtr->x,itemPtr->y,50,MoveMuffit,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CType = CTYPE_MISC;						// set collision info
	newObj->CBits = CBITS_ALLSOLID;

	newObj->TopOff = -25;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -25;
	newObj->RightOff = 25;
	CalcObjectBox2(newObj);

	return(true);									// was added
}


/********************** MOVE MUFFIT **********************/

void MoveMuffit(void)
{
ObjectEntryType	item;
short	i;

	if (TrackItem())									// see if gone
	{
		DeleteObject(gThisNodePtr);
		return;
	}

	if (gThisNodePtr->X.Int < gScrollX)					// as long as visible, then potential for spider drop
		return;
	if (gThisNodePtr->X.Int > (gScrollX+PF_WINDOW_WIDTH))
		return;
	if (gThisNodePtr->Y.Int < gScrollY)
		return;
	if (gThisNodePtr->Y.Int > (gScrollY+PF_WINDOW_HEIGHT))
		return;

	if ((MyRandomLong()&0b1111111) == 0)					// see if spider add
	{
		for (i = 0; i < MAX_MUFFIT_SPIDERS; i++)
		{
			item.x = gThisNodePtr->X.Int+RandomRange(0,15)-7;
			item.y = gThisNodePtr->Y.Int-RandomRange(0,10);
			if (AddEnemy_Spider(&item))					// add spider & fix
				gMostRecentlyAddedNode->ItemIndex = nil;
		}

		if (gThisNodePtr->SubType != MUFFIT_SUB_SCARE)				// see if scare her
		{
			SwitchAnim(gThisNodePtr,MUFFIT_SUB_SCARE);
			PlaySound(gSoundNum_Shriek);
		}
	}
}


/*====================================================================================*/

/*********** MAKE POISON APPLE ***************/
//
// parm[0] = subType 0 or 1
//
// Note: uses normal Health sprites
//

Boolean AddPoisonApple(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	newObj = MakeNewShape(GroupNum_FairyHealth,ObjType_FairyHealth,itemPtr->parm[0],
						itemPtr->x,itemPtr->y,50,SimpleObjectMove,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CType = CTYPE_ENEMYB;					// set collision info
	newObj->CBits = CBITS_TOUCHABLE;

	newObj->TopOff = -20;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -15;
	newObj->RightOff = 15;
	CalcObjectBox2(newObj);

	return(true);									// was added
}

/*===============================================================================*/

/**************** ADD HYDRANT ************************/

Boolean AddHydrant(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	newObj = MakeNewObject(BG_GENRE,itemPtr->x,itemPtr->y,NEAREST_Z,MoveHydrantBase);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;			// remember where this came from

	newObj->HydrantDirection = itemPtr->parm[0];

	return(true);							// was added
}


/*************** MOVE HYDRANT BASE *****************/

void MoveHydrantBase(void)
{
ObjNode *newNode;
long	dx;
short	x,y;
static	long hydrantDX[2] = {-0x90000L,0x90000L};

	if (TrackItem())									// see if gone
	{
		DeleteObject(gThisNodePtr);
		return;
	}

	if (NumObjects >= (MAX_OBJECTS-50))					// don't use up all the sprites!
		return;


			/* SEE WHICH WAY TO MAKE IT GO */

	dx = hydrantDX[gThisNodePtr->HydrantDirection];
	x = gThisNodePtr->X.Int + (MyRandomLong()&0b11);
	y = gThisNodePtr->Y.Int + 38;

					/* MAKE NEW OBJECT */

	newNode = MakeNewShape(GroupNum_FireHydrant,ObjType_FireHydrant,0,x,y,50,
							MoveHydrantWater,PLAYFIELD_RELATIVE);
	if (newNode == nil)
		return;

	InitYOffset(newNode, -39);

	newNode->CType = CTYPE_ENEMYC;
	newNode->CBits = CBITS_TOUCHABLE;

	newNode->TopOff = -16;					// set collision box
	newNode->BottomOff = 0;
	newNode->LeftOff = -9;
	newNode->RightOff = 9;

	newNode->DX = dx;
	newNode->WaterFallDelta = 0;

	return;
}


/*************** MOVE HYDRANT WATER ********************/
//
// gThisNodePtr = ptr to current node
//

void MoveHydrantWater(void)
{
	GetObjectInfo();

	if (!gThisNodePtr->SubType)							// if moving
	{
		gX.L += gDX;									// move it
		gY.L += gDY;

		gThisNodePtr->WaterFallDelta += 0x2000;
		if (((gThisNodePtr->YOffset.L += gThisNodePtr->WaterFallDelta) >= 0) ||
			(TestCoordinateRange()))
		{
			SwitchAnim(gThisNodePtr,1);
			StopObjectMovement(gThisNodePtr);			// prevent movement extrapolation
		}

		CalcObjectBox();
		UpdateObject();
	}
}








