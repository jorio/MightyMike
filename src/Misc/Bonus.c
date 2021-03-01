/****************************/
/*    	BONUS STUFF         */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/****************************/
/*    EXTERNALS             */
/****************************/
#include "myglobals.h"
//#include <fixmath.h>
#include "window.h"
#include "enemy.h"
#include "object.h"
#include "bonus.h"
#include "shape.h"
#include "playfield.h"
#include "miscanims.h"
#include "misc.h"
#include "myguy.h"
#include "myguy2.h"
#include "objecttypes.h"
#include "infobar.h"
#include "store.h"
#include "io.h"
#include "weapon.h"
#include "sound2.h"
#include "collision.h"
#include "input.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY;
extern	MikeFixed		gX;
extern	MikeFixed		gY;
extern	CollisionRec	gCollisionList[];
extern	short			gNumCollisions;
extern	long			gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	long			gMyNormalMaxSpeed,NumObjects;
extern	long			gMyX,gMyY;
extern	short			gNumEnemies;
extern	ObjNode			*gMostRecentlyAddedNode;
extern	unsigned char	gInterlaceMode;
extern	ObjectEntryType *gMasterItemList;
extern	short				gNumItems;
extern	ObjNode			*gMyNodePtr;
extern	long			gScrollX,gScrollY;
extern	Byte			gCurrentWeaponType,gSceneNum;
extern	char  			gMMUMode;
extern	Ptr				*gPFLookUpTable;
extern	short			gEnemyFreezeTimer,gSpeedyTimer;
extern	ObjNode			*gMyNodePtr;
extern	Byte			gNumBullets;
extern	Byte			gBonusWeaponStartScenes[];
extern	Boolean			gPPCFullScreenFlag,gRadarKeyFlag;
extern	Byte			gAreaNum;
extern	long			gDifficultySetting;


/****************************/
/*    CONSTANTS             */
/****************************/

#define	CoinTimer			Special3

#define	COIN_TIME			(GAME_FPS*4)				// life of coin on screen

#define BUNNY_BOUNCE_FACTOR	0xC0000L
#define	BUNNY_MAP_ID		3							// map item # for bunny
#define	BUNNY_HINT_DELAY	(GAME_FPS*20)
#define	BUNNY_HINT_MARGIN	50


struct HealthPOWType
{
	Byte			group,type,anim;
};
typedef struct HealthPOWType HealthPOWType;

#define	RADAR_CENTER_X		234
#define	RADAR_CENTER_Y		240
#define	RADAR_RANGE			20

#define	RADAR_CENTER_Xf		318
#define	RADAR_CENTER_Yf		214

#define	SHIELD_DURATION		(GAME_FPS*10)

/**********************/
/*     VARIABLES      */
/**********************/

Byte	gBunnyMessageNum = 0;
short		gNumBunnies;

short	gShieldTimer;

static	HealthPOWType	gHealthPOW[] = {
							GroupNum_JurassicHealth,ObjType_JurassicHealth,0,		// meat
							GroupNum_JurassicHealth,ObjType_JurassicHealth,1,		// berries
							GroupNum_ClownHealth,ObjType_ClownHealth,0,				// burger
							GroupNum_ClownHealth,ObjType_ClownHealth,1				// hotdog
							};


#define	NukeTimer		Special1
#define	NukeDoneFlag	Flag0

ObjNode	*gShieldNodePtr;

Byte	gBunnyCounts[5][3];


/******************** MAKE COINS ***********************/

void MakeCoins(short x, short y, short z, short quantity)
{
register	ObjNode		*newObj;
short		i;

	for (i=0; i < quantity; i++)
	{
					/* MAKE COIN OBJECT */

		newObj = MakeNewShape(GroupNum_Coin,ObjType_Coin,0,x,y,z,MoveCoin,PLAYFIELD_RELATIVE);
		if (newObj == nil)
			return;

		newObj->CType = CTYPE_BONUS;
		newObj->CBits = CBITS_TOUCHABLE;
		newObj->CoinTimer = COIN_TIME+(MyRandomLong()&b11111);		// set life of coin

		newObj->TopOff = -16;									// set box
		newObj->BottomOff = 0;
		newObj->LeftOff = -8;
		newObj->RightOff = 8;
		CalcObjectBox2(newObj);

		newObj->DX = MyRandomShort()<<1-0x10000L;					// random vector
		newObj->DY = MyRandomShort()<<1-0x10000L;

				/* MAKE COIN'S SHADOW */

		newObj->ShadowIndex = MakeShadow(newObj,SHADOWSIZE_TINY);		// allocate shadow

		newObj->YOffset.Int = -15;
		newObj->DZ = -0xA0000L+(MyRandomShort()<<2);					// start bouncing up
	}
}


/**************** MOVE COIN ********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveCoin(void)
{
register	ObjNode		*theNode;

	theNode = gThisNodePtr;

	GetObjectInfo();

	if (--theNode->CoinTimer <= 0)							// see if timed out
	{
		DeleteObject(theNode);									// delete coin & shadow
		return;
	}

	gX.L += gDX;											// move x/y
	gY.L += gDY;

				/* BOUNCE COIN AROUND */

	theNode->DZ += 0x12000L;								// add gravity

	theNode->YOffset.L += theNode->DZ;						// move it

	if (theNode->YOffset.Int > -1)							// see if bounce
	{
		theNode->YOffset.Int = -1;
		theNode->DZ = -theNode->DZ;
	}

				/* DO COLLISION */

	gSumDX = gDX;
	gSumDY = gDY;
	CalcObjectBox();
	HandleCollisions(CTYPE_BGROUND);

	UpdateObject();
}


//====================== FUZZY BUNNY =================================================================

/******************** ADD BUNNY ***********************/

Boolean AddBunny(ObjectEntryType *itemPtr)
{
ObjNode		*newObj;


	newObj = MakeNewShape(GroupNum_Bunny,ObjType_Bunny,0,itemPtr->x,itemPtr->y,
						50,MoveBunny,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CType = CTYPE_BONUS;
	newObj->CBits = CBITS_TOUCHABLE;

	newObj->TopOff = -30;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -14;
	newObj->RightOff = 14;
	CalcObjectBox2(newObj);

	newObj->YOffset.Int = 0;
	newObj->DZ = -(BUNNY_BOUNCE_FACTOR);				// start bouncing up


			/* MAKE SHADOW */

	newObj->ShadowIndex = MakeShadow(newObj,SHADOWSIZE_MEDIUM);	// allocate shadow


	return(true);									// was added
}


/******************** MOVE BUNNY ***********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveBunny(void)
{
register ObjNode	*theNode;

	theNode = gThisNodePtr;

	if (TrackItem())									// see if out of range
	{
		DeleteObject(theNode);
		return;
	}


	GetObjectInfo();


				/* BOUNCE */

	theNode->DZ += 0x14000L;							// add gravity

	theNode->YOffset.L += theNode->DZ;					// move it

	if (theNode->YOffset.Int > 0)						// see if bounce
	{
		theNode->YOffset.Int = 0;						// rebound
		theNode->DZ = -(BUNNY_BOUNCE_FACTOR);
		SwitchAnim(theNode,0);							// make just anim again
	}

				/* CHECK FOR MESSAGES */

	if (gMyNodePtr->OwnerToMessageNode == nil)						// see if already talking
	{
		if ((Absolute(gX.Int - gMyX) < 300) && (Absolute(gY.Int - gMyY) < 250))	// see if in range
		{
			if (!(MyRandomLong()&(b1111111)))
			{
				MakeMikeMessage(gBunnyMessageNum);
				if (++gBunnyMessageNum > 1)					// cycle thru messages
					gBunnyMessageNum = 0;
			}
		}
	}
}


/*************** DELETE BUNNY ********************/

void DeleteBunny(ObjNode *theNode)
{
	theNode->ItemIndex->type |= ITEM_MEMORY;		// set memory bits to indicate it is really gone
	theNode->ItemIndex = nil;						// wont be comin back
	DeleteObject(theNode);
}


/***************** COUNT BUNNIES ******************/

void CountBunnies(void)
{
short		i;

	gNumBunnies = 0;
	for (i=0; i < gNumItems; i++)					// scan all map items for BUNNY_MAP_ID
	{
		if (gMasterItemList[i].type == BUNNY_MAP_ID)
			gNumBunnies++;
	}

	gBunnyCounts[gSceneNum][gAreaNum] = gNumBunnies;	// remember count for each level
}


/**************** DEC BUNNY COUNT *****************/
//
// Gets called when a bunny is "removed".
// It then checks if we're ready for liftoff
//

void DecBunnyCount(void)
{
	if (--gNumBunnies <= 0)									// dec & see if that was the last bunny
	{
		gNumBunnies = 0;
		DisposeFrog();										// undo frog if needed
		DisposeSpaceShip();									// or ship
		StartMyLiftoff();
	}
	ShowNumBunnies();										// update counter
}

/********************** DISPLAY BUNNY RADAR *****************************/

void DisplayBunnyRadar(void)
{
int16_t		width,height;
long		numToRead;
short		fRefNum,vRefNum;
short		xDist,yDist;

						/***********************/
						/* LOAD THE RADAR PICT */
						/***********************/

	GetVol(nil,&vRefNum);									// get default volume
	OpenMikeFile(":data:images:radarmap.image",&fRefNum,"Cant open Radar Image!");
	SetFPos(fRefNum,fsFromStart,256*sizeof(RGBColor)+8);	// skip palette & pack header

	PlaySound(SOUND_RADAR);

	numToRead = 2;
	FSRead(fRefNum,&numToRead,&width);						// read width
	numToRead = 2;
	FSRead(fRefNum,&numToRead,&height);						// read height

	Byteswap16SignedRW(&width);
	Byteswap16SignedRW(&height);

	Ptr linePtr = AllocPtr(width*4);					// alloc memory to hold 4 lines of data

				/* WRITE TO BUFFER */

	Ptr destPtr = gPFLookUpTable[0];

	for (int i = 0; i < (height/4); i++)
	{
		numToRead = width*4;								// read 4 lines of data
		FSRead(fRefNum,&numToRead,linePtr);

		BlockMove(linePtr, destPtr, width*4);				// copy
		destPtr += width*4;
	}

	FSClose(fRefNum);
	DisposePtr((Ptr)linePtr);

						/************************************/
						/* SHOW THE RADAR PICT & DRAW BLIPS */
						/************************************/

	DisplayStoreBuffer();
	PresentIndexedFramebuffer();

	for (int i=0; i < gNumItems; i++)
	{
		if ((gMasterItemList[i].type & (ITEM_MEMORY|ITEM_NUM)) == BUNNY_MAP_ID)		// if memory bits set, then was deleted
		{
			xDist = (gMasterItemList[i].x - gMyX)/RADAR_RANGE;
			yDist = (gMasterItemList[i].y - gMyY)/RADAR_RANGE;

			if ((Absolute(xDist) < 180) && (Absolute(yDist) < 172))				// draw if on radar screen
			{
				if (gPPCFullScreenFlag)
					DrawFrameToScreen(RADAR_CENTER_Xf+xDist,RADAR_CENTER_Yf+yDist,
							GroupNum_RadarBlip,ObjType_RadarBlip,0);
				else
					DrawFrameToScreen(RADAR_CENTER_X+xDist,RADAR_CENTER_Y+yDist,
							GroupNum_RadarBlip,ObjType_RadarBlip,0);
			}
		}
	}

	while(!CheckNewKeyDown(KEY_SPACE,kKey_Radar,&gRadarKeyFlag))		// wait for spacebar
	{
		PresentIndexedFramebuffer();
		Wait4(10);														// don't cook CPU too much
	}

	EraseStore();

}


//====================== HEALTH POWERUPS =================================================================

/******************** ADD HEALTH POW ***********************/

Boolean AddHealthPOW(ObjectEntryType *itemPtr)
{
ObjNode		*newObj;
Byte	group,type;

	switch(gSceneNum)
	{

		case	SCENE_JURASSIC:
				group = GroupNum_JurassicHealth;
				type = ObjType_JurassicHealth;
				break;

		case	SCENE_CLOWN:
				group = GroupNum_ClownHealth;
				type = ObjType_ClownHealth;
				break;

		case	SCENE_CANDY:
				group = GroupNum_CandyHealth;
				type = ObjType_CandyHealth;
				break;

		case	SCENE_FAIRY:
				group = GroupNum_FairyHealth;
				type = ObjType_FairyHealth;
				break;

		case	SCENE_BARGAIN:
				group = GroupNum_BargainHealth;
				type = ObjType_BargainHealth;
				break;

		default:
				DoFatalAlert("No food defined for this Scene.");
	}

	newObj = MakeNewShape(group,type,itemPtr->parm[0],itemPtr->x,itemPtr->y,
						50,SimpleObjectMove,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CType = CTYPE_BONUS|CTYPE_HEALTH;
	newObj->CBits = CBITS_TOUCHABLE;

	newObj->TopOff = -30;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -14;
	newObj->RightOff = 14;
	CalcObjectBox2(newObj);

	return(true);									// was added
}


//====================== KEYS ========================================================

/******************** ADD KEY ***********************/

Boolean AddKey(ObjectEntryType *itemPtr)
{
ObjNode		*newObj;
short	group,type;

	if (gDifficultySetting == DIFFICULTY_EASY)				// no doors/keys in easy mode
		return(false);

	switch(gSceneNum)
	{
		case	SCENE_CLOWN:
				group = GroupNum_ClownKeys;
				type = ObjType_ClownKeys;
				break;

		case	SCENE_JURASSIC:
				group = GroupNum_JurassicKeys;
				type = ObjType_JurassicKeys;
				break;

		case	SCENE_CANDY:
				group = GroupNum_CandyKeys;
				type = ObjType_CandyKeys;
				break;

		case	SCENE_FAIRY:
				group = GroupNum_FairyKeys;
				type = ObjType_FairyKeys;
				break;

		case	SCENE_BARGAIN:
				group = GroupNum_BargainKeys;
				type = ObjType_BargainKeys;
				break;
	}


	newObj = MakeNewShape(group,type,itemPtr->parm[0],itemPtr->x,itemPtr->y,
						50,MoveKey,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CType = CTYPE_BONUS|CTYPE_KEY;
	newObj->CBits = CBITS_TOUCHABLE;

	newObj->TopOff = -20;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -10;
	newObj->RightOff = 10;
	CalcObjectBox2(newObj);

	return(true);									// was added
}


/******************** MOVE KEY ***********************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveKey(void)
{
	if (TrackItem())									// see if out of range
	{
		DeleteObject(gThisNodePtr);
		return;
	}
}


/****************** PUT BONUS POW **********************/
//
// Called when an enemy is killed.  Checks to see if we
// want to put a random powerup down with the coins.
//

void PutBonusPOW(short x, short y)
{
static ObjectEntryType	item;

	if (!(MyRandomLong()&b1))
	{
		if (MyRandomLong()&1)
		{
					/* PUT MISC */

			item.x = x;
			item.y = y;
			item.parm[0] = RandomRange(0,4);			// note: #5 is free dude - they don't randomly appear!
			item.parm[1] = true;
			if (AddMiscPowerup(&item))
				gMostRecentlyAddedNode->ItemIndex = nil;	// trick it
		}
		else
		{
				/* PUT WEAPON */
			do
			{
				item.parm[0] = RandomRange(0,14);
			} while (gBonusWeaponStartScenes[item.parm[0]] > gSceneNum);	// make sure this weapon can appear during this scene

			item.x = x;
			item.y = y;
			item.parm[1] = true;
			if (AddWeaponPowerup(&item))
				gMostRecentlyAddedNode->ItemIndex = nil;	// trick it
		}
	}
}


/*********** ADD MISC POWERUP ******************/
//
// Add playfield item
//

Boolean AddMiscPowerup(ObjectEntryType *itemPtr)
{
ObjNode		*newObj;


	newObj = MakeNewShape(GroupNum_MiscPOWs,ObjType_MiscPOWs,itemPtr->parm[0],itemPtr->x,itemPtr->y,
						50,MovePOW,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CType = CTYPE_BONUS|CTYPE_MISCPOW;
	newObj->CBits = CBITS_TOUCHABLE;

	newObj->TopOff = -20;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -10;
	newObj->RightOff = 10;
	CalcObjectBox2(newObj);

	newObj->Kind = itemPtr->parm[0];				// remember item type

	if (itemPtr->parm[1])							// see if temporary
		newObj->Special1 = GAME_FPS*6;				// set POW timer
	else
		newObj->Special1 = 0xf0000L;

	return(true);									// was added
}

/**************** GET MISC POW *******************/
//
// Add weapon into my inventory
//

void GetMiscPOW(short powType)
{
	PlaySound(SOUND_GETPOW);

	switch(powType)
	{
		case	MISC_POW_TYPE_NUKE:						// nuke kill
				StartNuke();
				break;

		case	MISC_POW_TYPE_FREEZE:
				gEnemyFreezeTimer = GAME_FPS*7;			// freeze enemies
				break;

		case	MISC_POW_TYPE_SHIELD:					// get shield
				StartShield();
				break;

		case	MISC_POW_TYPE_RINGSHOT:					// fire ring of death
				FireRingShot();
				break;

		case	MISC_POW_TYPE_SPEED:					// speedy shoes
				gMyNormalMaxSpeed = 0x130000L;
				gSpeedyTimer = GAME_FPS*15;
				break;

		case	MISC_POW_TYPE_FREEDUDE:					// free dude
				GetFreeDude();
				break;
	}
}


/**************** MOVE POW ********************/

void MovePOW(void)
{
	if (TrackItem())
		DeleteObject(gThisNodePtr);
	else
	{
		if (--gThisNodePtr->Special1 < (GAME_FPS*2))			// see if blinkie or timeout
		{
			if (gThisNodePtr->Special1 <= 0)				// timeout?
			{
				DeleteObject(gThisNodePtr);
				return;
			}
			else
			{
				gThisNodePtr->DrawFlag = gThisNodePtr->Special1&1;	// make blink
			}
		}
	}
}


/***************** START SHIELD *******************/

void StartShield(void)
{
	if	(!gShieldTimer)				// see if already have shield
	{
		if ((gShieldNodePtr = MakeNewShape(GroupNum_MyShieldEffect,ObjType_MyShieldEffect,0,
				gMyNodePtr->X.Int,gMyNodePtr->Y.Int,NEAREST_Z,MoveShield,PLAYFIELD_RELATIVE)) == nil)
			return;
	}
	else
	{
		SwitchAnim(gShieldNodePtr,0);				// make sure its normal if already had one
	}

	gShieldTimer = SHIELD_DURATION;		// set duration
}


/*************** MOVE SHIELD ********************/

void MoveShield(void)
{
	gThisNodePtr->X.Int = gMyNodePtr->X.Int;
	gThisNodePtr->Y.Int = gMyNodePtr->Y.Int;

	if (--gShieldTimer == 0)								// see if timeup
		DeleteObject(gShieldNodePtr);
	else
	if (gShieldTimer == (GAME_FPS*2))						// see if about out
		SwitchAnim(gThisNodePtr,1);

}


/******************** FIRE RING SHOT ********************/

void FireRingShot(void)
{
register ObjNode	*newNode;
short	i;
static	float	sinTbl[16] =	{0,0.38268,0.7071,0.92387,
								1,0.92387,0.7071,0.38268,
								0,-0.38268,-0.7071,-0.92387,
								-1,-0.92387,-0.7071,-0.38268};

static	float	cosTbl[16] =	{-1,-0.92387,-0.7071,-0.38268,
								0,0.38268,0.7071,0.92387,
								1,0.92387,0.7071,0.38268,
								0,-0.38268,-0.7071,-0.92387};


	for (i=0; i < 16; i++)
	{
		if (NumObjects >= (MAX_OBJECTS-30))					// don't use up all the sprites!
			return;

		newNode = MakeNewShape(GroupNum_Flamethrower,ObjType_Flamethrower,0,
								gMyX,gMyY,100,MoveFireRing,PLAYFIELD_RELATIVE);
		if (newNode == nil)
			return;

		newNode->CType = CTYPE_MYBULLET;
		newNode->CBits = CBITS_TOUCHABLE;

		newNode->TopOff = -10;						// set collision box
		newNode->BottomOff = 10;
		newNode->LeftOff = -10;
		newNode->RightOff = 10;

		newNode->DX = (float)sinTbl[i]*(float)0x60000L;
		newNode->DY = (float)cosTbl[i]*(float)0x60000L;

		newNode->WeaponPower = 10;					// set weapon's power

		newNode->TileMaskFlag = false;				// ignore prioritized tiles

		gNumBullets++;

	}
}


/*************** MOVE FIRE RING ********************/
//
// gThisNodePtr = ptr to current node
//

void MoveFireRing(void)
{

	if (TrackItem())
	{
		DeleteWeapon(gThisNodePtr);
		return;
	}

	GetObjectInfo();

	gX.L += gDX;									// move it
	gY.L += gDY;

				/* UPDATE OBJECT */

	CalcObjectBox();
	UpdateObject();
}


/******************* START NUKE ********************/

void StartNuke(void)
{
register	ObjNode		*newObj;

	newObj = MakeNewObject(BG_GENRE,gMyX,gMyY,NEAREST_Z,MoveNukeObject);		// make nuke object process
	if (newObj == nil)
		return;

	StartShakeyScreen(newObj->NukeTimer = GAME_FPS*4);			// set duration & shake

	MakeMikeMessage(MESSAGE_NUM_FIREHOLE);
}


/******************** MOVE NUKE OBJECT **********************/
//
// Controls the main Nuke generator
//

void MoveNukeObject(void)
{
short	x,y;
ObjNode	*newNode;

	if (TrackItem())									// see if out of range
	{
		DeleteObject(gThisNodePtr);
		return;
	}

				/* MAKE A NUKE */

	if (NumObjects >= (MAX_OBJECTS-40))					// don't use up all the sprites!
		return;

	x = gThisNodePtr->X.Int + (RandomRange(0,600)-300);
	y = gThisNodePtr->Y.Int + (RandomRange(0,600)-300);


	newNode = MakeNewShape(GroupNum_Nuke,ObjType_Nuke,0,x,y,100,MoveNuke,PLAYFIELD_RELATIVE);
	if (newNode == nil)
		goto update;

	newNode->CType = CTYPE_MYBULLET;
	newNode->CBits = CBITS_TOUCHABLE;

	newNode->TopOff = -40;						// set collision box
	newNode->BottomOff = 0;
	newNode->LeftOff = -25;
	newNode->RightOff = 25;

	newNode->WeaponPower = 10;					// set weapon's power
	newNode->NukeDoneFlag = false;

	gNumBullets++;

	if (!(gThisNodePtr->NukeTimer & b11))		// see if continue nuke
		PlaySound(SOUND_NUKE);

update:
	if (--gThisNodePtr->NukeTimer <= 0)					// see if nuked out
		DeleteObject(gThisNodePtr);
}


/******************** MOVE NUKE *************************/
//
// Move the small individual nukelet
//

void MoveNuke(void)
{
	CalcObjectBox2(gThisNodePtr);

	if (gThisNodePtr->NukeDoneFlag)				// just see if done
		DeleteWeapon(gThisNodePtr);
}


/******************** ADD SHIP POW ***********************/

Boolean AddShipPOW(ObjectEntryType *itemPtr)
{
ObjNode		*newObj;

	newObj = MakeNewShape(GroupNum_SpaceShip,ObjType_SpaceShip,8,itemPtr->x,itemPtr->y,
						50,SimpleObjectMove,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CType = CTYPE_BONUS;
	newObj->CBits = CBITS_TOUCHABLE;

	newObj->TopOff = -40;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -20;
	newObj->RightOff = 20;
	CalcObjectBox2(newObj);

	return(true);									// was added
}

