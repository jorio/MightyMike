/****************************/
/*    	WEAPON MANAGER      */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/****************************/
/*    EXTERNALS             */
/****************************/
#include "myglobals.h"
#include "objecttypes.h"
#include "window.h"
#include "myguy.h"
#include "playfield.h"
#include "object.h"
#include "infobar.h"
#include "io.h"
#include "miscanims.h"
#include "sound2.h"
#include "shape.h"
#include "misc.h"
#include "weapon.h"
#include "bonus.h"
#include "collision.h"
#include "input.h"
#include "externs.h"

/****************************/
/*    CONSTANTS             */
/****************************/

#define	SUCTIONCUP_POWER	1					// bullet does .. damage to enemy
#define	CAKE_POWER			4					// grenade does .. damage to enemy
#define	OOZIE_POWER			1					// oozie does..
#define	RBAND_POWER			1					// ricochet does..
#define	TOOTHPASTE_POWER 	1					// toothpaste does..

#define	SUCTIONCUP_DURATION	(GAME_FPS/4)
#define	OOZIE_DURATION	(GAME_FPS/3)
#define	RBAND_DURATION (GAME_FPS*2)
#define	TOOTHPASTE_DURATION (GAME_FPS/4)

#define	MAX_WEAPON_AMMO		500

/**********************/
/*     VARIABLES      */
/**********************/

WeaponType	gMyWeapons[MAX_WEAPONS];
Byte		gNumWeaponsIHave,gCurrentWeaponIndex;

Byte		gCurrentWeaponType;
Byte		gNumBullets;
long		gOozieTick = 0;


				/* WEAPON SHOOT JUMPTABLE */

static	Boolean(*gWeaponShootTable[])(void) =
				{
					ShootSuctionCup,
					ShootCake,
					ShootOozie,
					ShootRBand,
					ShootToothpaste,
					ShootTracer,
					ShootPixieDust,
					ThrowRock,
					ShootHeatSeek,
					ShootElephantGun,
					ThrowPie,
					ShootDoubleShot,
					ShootTripleShot,
					ShootFlamethrower,
					ShootRocketGun
				};

static	short	gWeaponQuantities[] =
				{
					50,					//SuctionCup
					20,					//Cake
					100,				//Oozie
					30,					//RBand
					100,				//Toothpaste
					50,					//Tracer
					40,					//PixieDust
					20,					//Rock
					40,					//HeatSeek
					30,					//ElephantGun
					20,					//Pie
					40,					//DoubleShot
					50,					//TripleShot
					100,				//Flamethrower
					20					//RocketGun
				};

Byte	gBonusWeaponStartScenes[] =
				{
					0,					//SuctionCup
					SCENE_CANDY,		//Cake
					SCENE_BARGAIN,		//Oozie
					SCENE_CLOWN,		//RBand
					SCENE_CANDY,		//Toothpaste
					SCENE_BARGAIN,		//Tracer
					SCENE_FAIRY,		//PixieDust
					SCENE_JURASSIC,		//Rock
					SCENE_FAIRY,		//HeatSeek
					SCENE_JURASSIC,		//ElephantGun
					SCENE_CLOWN,		//Pie
					0,					//DoubleShot
					2,					//TripleShot
					SCENE_JURASSIC,		//Flamethrower
					SCENE_CLOWN			//RocketGun
				};


/******************* INIT WEAPONS LIST **********************/
//
// Init main master weapon inventory list
//

void InitWeaponsList(void)
{
				/* INIT DEFAULT WEAPON */

	gCurrentWeaponIndex = 0;
	gCurrentWeaponType = WEAPON_TYPE_SUCTIONCUP;

	gMyWeapons[0].type = gCurrentWeaponType;
	gMyWeapons[0].life = MAX_WEAPON_AMMO;
	gNumWeaponsIHave = 1;
}



/***************** INIT BULLETS **************/
//
// Init firing & bullet info
//

void InitBullets(void)
{
	gNumBullets = 0;
	gLastRocketTime = gLastPixieTime = 0;
}

/*********** ADD WEAPON POWERUP ******************/
//
// Add playfield item
//

Boolean AddWeaponPowerup(ObjectEntryType *itemPtr)
{
ObjNode		*newObj;


	newObj = MakeNewShape(GroupNum_WeaponPOWs,ObjType_WeaponPOWs,itemPtr->parm[0],itemPtr->x,itemPtr->y,
						50,MovePOW,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	newObj->CType = CTYPE_BONUS|CTYPE_WEAPONPOW;
	newObj->CBits = CBITS_TOUCHABLE;

	newObj->TopOff = -30;							// set box
	newObj->BottomOff = 0;
	newObj->LeftOff = -14;
	newObj->RightOff = 14;
	CalcObjectBox2(newObj);

	newObj->Kind = itemPtr->parm[0];				// remember item type

	if (itemPtr->parm[1])							// see if temporary
		newObj->Special1 = GAME_FPS*6;				// set POW timer
	else
		newObj->Special1 = 0xf0000L;

	return(true);									// was added
}



/**************** GET A WEAPON *******************/
//
// Add weapon into my inventory
//

void GetAWeapon(short weaponType)
{
Byte	i;

	PlaySound(SOUND_GETWEAPON);

	if (!(MyRandomLong()&b111))
		MakeMikeMessage(MESSAGE_NUM_NICEGUY);

	if (gNumWeaponsIHave >= MAX_WEAPONS)							// see if can add anything
		DoFatalAlert("Weapon inventory Overload!  Call Brian!");


				/* SEE IF ADD TO LIFE */

	for (i=0; i < gNumWeaponsIHave; i++)
	{
		if (gMyWeapons[i].type == weaponType)
		{
			gMyWeapons[i].life += gWeaponQuantities[weaponType];		// add to quantity
			if (gMyWeapons[i].life > MAX_WEAPON_AMMO)								// check max
				gMyWeapons[i].life = MAX_WEAPON_AMMO;
			goto exit;
		}
	}

				/* ADD ENTIRELY NEW ITEM */

	gMyWeapons[gNumWeaponsIHave].type = weaponType;
	gMyWeapons[gNumWeaponsIHave].life = gWeaponQuantities[weaponType];
	i = gNumWeaponsIHave++;


exit:
//		gCurrentWeaponIndex = i;
//		gCurrentWeaponType = gMyWeapons[i].type;
//		if (refreshFlag)
			ShowWeaponIcon();
}




/*************** CHECK FIRE WEAPON ******************/
//
// See if should fire the weapon
//

void CheckFireWeapon(void)
{
Boolean		shotFlag;


				/* CHECK WEAPON SELECTION */

	if (GetNewNeedState(kNeed_NextWeapon))
		SelectNextWeapon();


	if (gCurrentWeaponType != NO_WEAPON)
		shotFlag = gWeaponShootTable[gCurrentWeaponType]();	// call shoot routine
	else
		shotFlag = false;


				/* DECREASE WEAPON LIFE */

	if (shotFlag)													// only do it if I shot something
	{
		if (--gMyWeapons[gCurrentWeaponIndex].life != 0)			// decrease life & see if exhausted
			ShowWeaponLife();
		else														// weapon is empty
			RemoveCurrentWeaponFromInventory();
	}
}


/***************** SELECT NEXT WEAPON *****************/

void SelectNextWeapon(void)
{

	if (gNumWeaponsIHave < 2)							// see if either 0 or 1 weapon
		return;

	if (++gCurrentWeaponIndex >= gNumWeaponsIHave)		// see if wrap to front
		gCurrentWeaponIndex = 0;

	gCurrentWeaponType = gMyWeapons[gCurrentWeaponIndex].type;
	ShowWeaponIcon();
	PlaySound(SOUND_SELECTCHIME);

}


/********** REMOVE CURRENT WEAPON FROM INVENTORY *************/

void RemoveCurrentWeaponFromInventory(void)
{
Byte	i;

	if (gCurrentWeaponType == WEAPON_TYPE_SUCTIONCUP)		// always have suction cup gun
	{
		gMyWeapons[gCurrentWeaponIndex].life = 1;
		return;
	}

				/* SHIFT ALL UP */

	for (i=gCurrentWeaponIndex; i < (gNumWeaponsIHave-1); i++)
	{
		gMyWeapons[i].type = gMyWeapons[i+1].type;
		gMyWeapons[i].life = gMyWeapons[i+1].life;
	}
	gNumWeaponsIHave--;											// dec # items in inventory

				/* RESET TO DEFAULT */

	if (gNumWeaponsIHave > 0)
	{
		gCurrentWeaponIndex = 0;
		gCurrentWeaponType = gMyWeapons[gCurrentWeaponIndex].type;
	}
	else
	{
		gCurrentWeaponType = NO_WEAPON;							// all out
	}
	ShowWeaponIcon();
}


/**************** WEAPON HIT ENEMY *****************/
//
// This is called by Enemy collision detection
// when a weapon hits an enemy.  This routine only
// determines what to do with the weapon when this occurs.
//

void WeaponHitEnemy(ObjNode *weaponNode)
{
	switch(weaponNode->Type)
	{
		case	ObjType_Flamethrower:						// flamethrower does nothing
		case	ObjType_RocketGun:							// rocketgun does nothing
		case	ObjType_Cake:								// grenade does nothing
				break;

		case	ObjType_Pie:								// pie explodes
				ExplodePie(weaponNode);
				break;

		default:
				DeleteWeapon(weaponNode);						// delete the weapon
	}
}


/******************** DELETE WEAPON *****************/
//
// Takes care of deleting the passed weapon node.
//

void DeleteWeapon(ObjNode *weaponNode)
{
	DeleteObject(weaponNode);
	gNumBullets--;
}


/*************** CALC WEAPON START COORDS ********************/
//
// returns true if start coords are in solid object
//

short		gWeaponStartOffZ[8] = {-10,-10,10,10,10,10,10,-10};			// shooting
short		gWeaponStartOffY[8] = {-5,-2,1,15,9,15,1,-2};
short		gWeaponStartOffX[8] = {0,10,25,15,-10,-10,-25,-10};

short		gWeaponStartOffZt[8] = {-10,-10,10,10,10,10,10,-10};		// throwing
short		gWeaponStartOffYt[8] = {-5,-2,-3,2,2,2,-4,-2};
short		gWeaponStartOffXt[8] = {-6,10,15,10,3,-10,-10,-10};


Boolean	CalcWeaponStartCoords(long dx, long dy, short *x, short *y, short *z, Byte kind)
{
short		h,v;									// h & v are used for initial collision check
unsigned short	bits;

				/* GET OFFSETS & SET COORDS */

	if (kind == WS_SHOOT)
	{
		*z = gMyNodePtr->Z+gWeaponStartOffZ[gMyDirection];
		*y = gMyY+gWeaponStartOffY[gMyDirection];
		*x = gMyX+gWeaponStartOffX[gMyDirection];
	}
	else
	if (kind == WS_THROW)
	{
		*z = gMyNodePtr->Z+gWeaponStartOffZt[gMyDirection];
		*y = gMyY+gWeaponStartOffYt[gMyDirection];
		*x = gMyX+gWeaponStartOffXt[gMyDirection];
	}

				/* SEE IF MATERIALIZE IN ROCK */

	if (dy < 0)
	{
		v = *y-8;
	}
	else
	if (dy > 0)
	{
		v = *y+8;
	}
	else
	{
		v = *y;
	}

	if (dx < 0)
	{
		h = *x-8;
	}
	else
	if (dx > 0)
	{
		h = *x+8;
	}
	else
	{
		h = *x;
	}

	bits = GetMapTileAttribs(h,v);				// see if materialize in rock
	if (bits & b1111)							// check if solid at all
	{
		if (bits & TILE_ATTRIB_BULLETGOESTHRU)	// see if go thru anyway
			return(false);
		else
			return(true);
	}
	else
			/* SEE IF MATERIALIZE IN MISC OBJECT */

	if (DoPointCollision(h,v,CTYPE_MISC))
	{
 		return(true);
	}
	else
		return(false);
}


/*************** MOVE BASIC BULLET ********************/
//
// This routine just does basic move for standard/generic bullet
//

void MoveBasicBullet(void)
{
	if (--gThisNodePtr->Health < 0)					// see if disintegrates
	{
		DeleteWeapon(gThisNodePtr);
		return;
	}

	GetObjectInfo();

	gX.L += gDX;									// move it
	gY.L += gDY;

	if (TestCoordinateRange())						// see if out of range
	{
		DeleteWeapon(gThisNodePtr);
		return;
	}

				/* DO COLLISION DETECT */

	if (!(GetMapTileAttribs(gX.Int,gY.Int)&TILE_ATTRIB_BULLETGOESTHRU))
	{

		if (DoPointCollision(gX.Int,gY.Int,CTYPE_BGROUND|CTYPE_MISC))
		{
			DeleteObject(gThisNodePtr);
			return;
		}
	}

	CalcObjectBox();
	UpdateObject();
}

/*************** MOVE BASIC RICO ********************/
//
// This routine just does basic move for standard/generic ricochet bullet
//

void MoveBasicRico(void)
{
Byte	sides,check;
unsigned short	attribs;


	if (--gThisNodePtr->Health < 0)					// see if disintegrates
	{
		DeleteWeapon(gThisNodePtr);
		return;
	}

	GetObjectInfo();

	gX.L += gDX;									// move it
	gY.L += gDY;

				/* DO COLLISION DETECT */

	if (!((attribs = GetMapTileAttribs(gX.Int,gY.Int))&TILE_ATTRIB_BULLETGOESTHRU))
	{
					/* HANDLE TILE COLLISION */

		check = 0;
		if (attribs & TILE_ATTRIB_ALLSOLID)
		{
			if (gDX < 0)						// see if bounce DX
			{
				if (!(GetMapTileAttribs(gX.Int+TILE_SIZE,gY.Int) & TILE_ATTRIB_LEFTSOLID))
					gDX = -gDX;
				else
					check++;
			}
			else
			if (gDX > 0)
			{
				if (!(GetMapTileAttribs(gX.Int-TILE_SIZE,gY.Int) & TILE_ATTRIB_RIGHTSOLID))
					gDX = -gDX;
				else
					check++;
			}
			else
				check++;

			if (gDY < 0)						// see if bounce DY
			{
				if (!(GetMapTileAttribs(gX.Int,gY.Int+TILE_SIZE) & TILE_ATTRIB_BOTTOMSOLID))
					gDY = -gDY;
				else
					check++;
			}
			else
			if (gDY > 0)
			{
				if (!(GetMapTileAttribs(gX.Int,gY.Int+TILE_SIZE) & TILE_ATTRIB_TOPSOLID))
					gDY = -gDY;
				else
					check++;
			}
			else
				check++;

			if (check >= 2)						// if checked 2 times, then direct bounce back
			{									// this is to correct for the "filler tile" problem on diagonal surfaces (when it skips thru 2 solids on a diagonal)
				gDY = -gDY;
				gDX = -gDX;
			}
		}
		else
		{

							/* HANDLE SPRITE COLLISON */
			gSumDX = gDX;
			gSumDY = gDY;
			CalcObjectBox();
			sides = HandleCollisions(CTYPE_MISC);

			if (sides & SIDE_BITS_TOP)						// see if bounce off top
			{
				gDY = -gDY;
			}
			else
			if (sides & SIDE_BITS_BOTTOM)					// see if bounce off bottom
			{
				gDY = -gDY;
			}
			else
			if (sides & SIDE_BITS_LEFT)						// see if bounce off left
			{
				gDX = -gDX;
			}
			else
			if (sides & SIDE_BITS_RIGHT)					// see if bounce off right
			{
				gDX = -gDX;
			}
		}
	}


				/* UPDATE OBJECT */

	CalcObjectBox();
	UpdateObject();
}




/*=========================== SUCTIONCUP ===============================================*/

#define	SUCTIONCUP_SPEED	0x120000L

const long		gSuctionCupDeltasX[] = {0,SUCTIONCUP_SPEED,SUCTIONCUP_SPEED,SUCTIONCUP_SPEED,
							0,-SUCTIONCUP_SPEED,-SUCTIONCUP_SPEED,-SUCTIONCUP_SPEED};
const long		gSuctionCupDeltasY[] = {-SUCTIONCUP_SPEED,-SUCTIONCUP_SPEED,0,SUCTIONCUP_SPEED,
							SUCTIONCUP_SPEED,SUCTIONCUP_SPEED,0,-SUCTIONCUP_SPEED};

/**************** SHOOT SUCTIONCUP ************************/
//
// SuctionCups are low-powered, auto-firing, guns.
//
//

Boolean ShootSuctionCup(void)
{
ObjNode *newNode;
long	dx,dy;
short		z,y,x;

				/* SEE IF READY TO SHOOT */

	if (!GetNewNeedState(kNeed_Attack))					// see if fire button pressed
		return false;

			/* SEE WHICH WAY TO MAKE IT GO */

	dx = gSuctionCupDeltasX[gMyDirection];
	dy = gSuctionCupDeltasY[gMyDirection];

	if (CalcWeaponStartCoords(dx,dy,&x,&y,&z,WS_SHOOT))	// calc start coord & see if in wall
		return(false);


					/* MAKE NEW OBJECT */

	newNode = MakeNewShape(GroupNum_SuctionCup,ObjType_SuctionCup,gMyDirection,x,y,z,
						MoveBasicBullet,PLAYFIELD_RELATIVE);
	if (newNode == nil)
		return(false);

	newNode->CType = CTYPE_MYBULLET;
	newNode->CBits = CBITS_TOUCHABLE;

	newNode->TopOff = -20;										// set collision box
	newNode->BottomOff = 2;
	newNode->LeftOff = -8;
	newNode->RightOff = 8;

	newNode->DX = dx;											// set deltas
	newNode->DY = dy;

	newNode->Health = SUCTIONCUP_DURATION;

	newNode->WeaponPower = SUCTIONCUP_POWER;					// set weapon's power

	gNumBullets++;

	PlaySound(SOUND_SUCKPOP);

	DrawMyGun();												// make sure I'm doing correct anim
	return(true);
}


/*=========================== Cake ===============================================*/

#define	CAKE_SPEED	0xD0000L

const long		gCakeDeltasX[] = {0,CAKE_SPEED,CAKE_SPEED,CAKE_SPEED,
							0,-CAKE_SPEED,-CAKE_SPEED,-CAKE_SPEED};
const long		gCakeDeltasY[] = {-CAKE_SPEED,-CAKE_SPEED,0,CAKE_SPEED,
							CAKE_SPEED,CAKE_SPEED,0,-CAKE_SPEED};


/**************** SHOOT Cake ************************/

Boolean ShootCake(void)
{
ObjNode *newNode;
long	dx,dy;
short		z,y,x;


				/* SEE IF READY TO SHOOT */

	if (!GetNewNeedState(kNeed_Attack))					// see if fire button pressed
		return false;


			/* SEE WHICH WAY TO MAKE IT GO */

	dx = gCakeDeltasX[gMyDirection];
	dy = gCakeDeltasY[gMyDirection];


			/* CALC COORDINATES */

	if (CalcWeaponStartCoords(dx,dy,&x,&y,&z,WS_THROW))	// calc start coord & see if in wall
		return(false);

					/* MAKE NEW OBJECT */

	newNode = MakeNewShape(GroupNum_Cake,ObjType_Cake,0,x,y,z,MoveCake,PLAYFIELD_RELATIVE);
	if (newNode == nil)
		return(false);

	newNode->CBits = CBITS_TOUCHABLE;
	newNode->TopOff = -60;					// set collision box (not activated yet)
	newNode->BottomOff = 60;
	newNode->LeftOff = -60;
	newNode->RightOff = 60;

	newNode->DX = dx;
	newNode->DY = dy;
	newNode->WeaponPower = CAKE_POWER;		// set weapon's power
	gNumBullets++;


				/* MAKE SHADOW */

	newNode->ShadowIndex = MakeShadow(newNode,SHADOWSIZE_TINY); 	// allocate shadow & remember ptr to it

	newNode->YOffset.Int = -30;
	newNode->DZ = -0x80000L;					// start bouncing up

	PlaySound(SOUND_POP);
	StartMyThrow();
	return(true);
}


/*************** MOVE CAKE ********************/
//
// gThisNodePtr = ptr to current node
//

void MoveCake(void)
{
	if (gThisNodePtr->SubType == 1)						// if exploding, then dont move
		return;

	GetObjectInfo();

	gX.L += gDX;										// move it
	gY.L += gDY;

	if (TestCoordinateRange())							// see if out of range
	{
		DeleteWeapon(gThisNodePtr);
		return;
	}

				/* DO COLLISION DETECT */

	if (!(GetMapTileAttribs(gX.Int,gY.Int)&TILE_ATTRIB_BULLETGOESTHRU))
	{
		DoPointCollision(gX.Int,gY.Int,CTYPE_BGROUND|CTYPE_MISC);
	}



				/* BOUNCE IT */

	gThisNodePtr->DZ += 0x22000L;								// add gravity
	gThisNodePtr->YOffset.L += gThisNodePtr->DZ;				// move it
	if ((gThisNodePtr->YOffset.Int > -1) || gNumCollisions)		// see if landed or hit something solid
	{
		gNumBullets--;										// dec count (auto deletes itself later)
		SwitchAnim(gThisNodePtr,1);								// BLOW IT UP!
		gThisNodePtr->CType = CTYPE_MYBULLET;					// activate collision
		StopObjectMovement(gThisNodePtr);						// prevent movement extrapolation
	}

	CalcObjectBox();
	UpdateObject();
}


/*=========================== Oozie ===============================================*/

#define	OOZIE_SPEED	0x140000L

const long		gOozieDeltasX[] = {0,OOZIE_SPEED,OOZIE_SPEED,OOZIE_SPEED,
							0,-OOZIE_SPEED,-OOZIE_SPEED,-OOZIE_SPEED};
const long		gOozieDeltasY[] = {-OOZIE_SPEED,-OOZIE_SPEED,0,OOZIE_SPEED,
							OOZIE_SPEED,OOZIE_SPEED,0,-OOZIE_SPEED};

/**************** SHOOT OOZIE ************************/
//
// Oozies are low-powered, auto-firing, guns.
//
//

Boolean ShootOozie(void)
{
ObjNode *newNode;
long	dx,dy;
short		z,y,x;


				/* SEE IF READY TO SHOOT */

	if (!GetNeedState(kNeed_Attack))						// see if fire button pressed
		return(false);

	if (gFrames & b1)								// see if good interval
		return(false);

			/* SEE WHICH WAY TO MAKE IT GO */

	dx = gOozieDeltasX[gMyDirection];
	dy = gOozieDeltasY[gMyDirection];


			/* CALC COORDINATES */

	if (CalcWeaponStartCoords(dx,dy,&x,&y,&z,WS_SHOOT))	// calc start coord & see if in wall
		return(false);

					/* MAKE NEW OBJECT */

	newNode = MakeNewShape(GroupNum_Oozie,ObjType_Oozie,0,x,y,z,
							MoveBasicBullet,PLAYFIELD_RELATIVE);
	if (newNode == nil)
		return(false);

	newNode->CType = CTYPE_MYBULLET;
	newNode->CBits = CBITS_TOUCHABLE;

	newNode->TopOff = -16;										// set collision box
	newNode->BottomOff = 0;
	newNode->LeftOff = -5;
	newNode->RightOff = 5;

	newNode->Health = OOZIE_DURATION;

	newNode->DX = dx;
	newNode->DY = dy;

	newNode->WeaponPower = OOZIE_POWER;							// set weapon's power

	gNumBullets++;

	if (++gOozieTick & 1)					// sound only every other
		PlaySound(SOUND_MACHINEGUN);

	DrawMyGun();												// make sure I'm doing correct anim

	return(true);
}



/*=========================== RBand ===============================================*/

#define	RBand_SPEED	0x140000L

const long		gRBandDeltasX[] = {0,RBand_SPEED,RBand_SPEED,RBand_SPEED,
							0,-RBand_SPEED,-RBand_SPEED,-RBand_SPEED};
const long		gRBandDeltasY[] = {-RBand_SPEED,-RBand_SPEED,0,RBand_SPEED,
							RBand_SPEED,RBand_SPEED,0,-RBand_SPEED};

/**************** SHOOT RBand ************************/

Boolean ShootRBand(void)
{
ObjNode *newNode;
long	dx,dy;
short		z,y,x;


				/* SEE IF READY TO SHOOT */

	if (!GetNewNeedState(kNeed_Attack))					// see if fire button pressed
		return false;

	
			/* SEE WHICH WAY TO MAKE IT GO */

	dx = gRBandDeltasX[gMyDirection];
	dy = gRBandDeltasY[gMyDirection];


			/* CALC COORDINATES */

	if (CalcWeaponStartCoords(dx,dy,&x,&y,&z,WS_SHOOT))	// calc start coord & see if in wall
		return(false);


					/* MAKE NEW OBJECT */

	newNode = MakeNewShape(GroupNum_RBand,ObjType_RBand,0,x,y,z,MoveBasicRico,PLAYFIELD_RELATIVE);
	if (newNode == nil)
		return(false);

	newNode->CType = CTYPE_MYBULLET;
	newNode->CBits = CBITS_TOUCHABLE;

	newNode->TopOff = -16;					// set collision box
	newNode->BottomOff = 0;
	newNode->LeftOff = -5;
	newNode->RightOff = 5;

	newNode->Health = RBAND_DURATION;

	newNode->DX = dx;
	newNode->DY = dy;

	newNode->WeaponPower = RBAND_POWER;		// set weapon's power

	gNumBullets++;

	PlaySound(SOUND_RUBBERGUN);

	DrawMyGun();												// make sure I'm doing correct anim
	return(true);
}



/*=========================== TOOTHPASTE ===============================================*/

#define	Toothpaste_SPEED		0x120000L
#define ToothpasteFallDelta		Special1

const long		gToothpasteDeltasX[8] = {0,Toothpaste_SPEED,Toothpaste_SPEED,Toothpaste_SPEED,
							0,-Toothpaste_SPEED,-Toothpaste_SPEED,-Toothpaste_SPEED};
const long		gToothpasteDeltasY[8] = {-Toothpaste_SPEED,-Toothpaste_SPEED,0,Toothpaste_SPEED,
							Toothpaste_SPEED,Toothpaste_SPEED,0,-Toothpaste_SPEED};

const Byte		gToothpasteAnims[8] = {1,2,0,4,1,2,0,4};


/**************** SHOOT TOOTHPASTE ************************/
//
// Toothpastes are low-powered, auto-firing, guns.
//
//

Boolean ShootToothpaste(void)
{
ObjNode *newNode;
long	dx,dy;
short		z,y,x;
Byte	animNum;


				/* SEE IF READY TO SHOOT */

	if (!GetNeedState(kNeed_Attack))						// see if fire button pressed
		return(false);

			/* SEE WHICH WAY TO MAKE IT GO */

	dx = gToothpasteDeltasX[gMyDirection];
	dy = gToothpasteDeltasY[gMyDirection];

	animNum = gToothpasteAnims[gMyDirection];			// get correct anim

			/* CALC COORDINATES */

	if (CalcWeaponStartCoords(dx,dy,&x,&y,&z,WS_SHOOT))	// calc start coord & see if in wall
		return(false);

	if (gMyDirection == AIM_DOWN)						// special tweak if down
		y += 30;

	y += MyRandomLong()&b11-2;								// scatter a bit
	x += MyRandomLong()&b11-2;

					/* MAKE NEW OBJECT */

	newNode = MakeNewShape(GroupNum_Toothpaste,ObjType_Toothpaste,animNum,x,y,z,MoveToothpaste,PLAYFIELD_RELATIVE);
	if (newNode == nil)
		return(false);

	newNode->YOffset.Int = -39;

	newNode->CType = CTYPE_MYBULLET;
	newNode->CBits = CBITS_TOUCHABLE;

	newNode->TopOff = -16;					// set collision box
	newNode->BottomOff = 0;
	newNode->LeftOff = -9;
	newNode->RightOff = 9;

	newNode->Health = TOOTHPASTE_DURATION;

	newNode->DX = dx;
	newNode->DY = dy;
	newNode->ToothpasteFallDelta = 0;

	newNode->WeaponPower = TOOTHPASTE_POWER;		// set weapon's power

	gNumBullets++;

	DrawMyGun();												// make sure I'm doing correct anim
	return(true);
}


/*************** MOVE TOOTHPASTE ********************/
//
// gThisNodePtr = ptr to current node
//

void MoveToothpaste(void)
{
	if (--gThisNodePtr->Health < 0)					// see if disintegrates
	{
		gNumBullets--;
		SwitchAnim(gThisNodePtr,3);					// splat anim
		gThisNodePtr->MoveCall = nil;
		gThisNodePtr->CType = 0;
		gThisNodePtr->AnimSpeed = (MyRandomLong()&b1111111111)+0x80;
		StopObjectMovement(gThisNodePtr);			// prevent movement extrapolation
		return;
	}

	GetObjectInfo();

	gX.L += gDX;									// move it
	gY.L += gDY;

	gThisNodePtr->ToothpasteFallDelta += 0x3000;
	gThisNodePtr->YOffset.L += gThisNodePtr->ToothpasteFallDelta;

	if (TestCoordinateRange())						// see if out of range
	{
		DeleteWeapon(gThisNodePtr);
		return;
	}

				/* DO COLLISION DETECT */

	if (!(GetMapTileAttribs(gX.Int,gY.Int)&TILE_ATTRIB_BULLETGOESTHRU))
	{

		if (DoPointCollision(gX.Int,gY.Int,CTYPE_BGROUND|CTYPE_MISC))
		{
			DeleteObject(gThisNodePtr);
			return;
		}
	}

	CalcObjectBox();
	UpdateObject();
}

