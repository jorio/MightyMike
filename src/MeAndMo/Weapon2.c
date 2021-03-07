/****************************/
/*     	WEAPON2             */
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
#include "miscanims.h"
#include "sound2.h"
#include "misc.h"
#include "weapon.h"
#include "shape.h"
#include "io.h"
#include "collision.h"
#include "input.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY;
extern	MikeFixed		gX;
extern	MikeFixed		gY;
extern	CollisionRec	gCollisionList[];
extern	short			gNumCollisions;
extern	long				gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	short			gMyDirection;
extern	long				gMyX,gMyY;
extern	ObjNode			*gMyNodePtr;
extern	long			gFrames;
extern	Byte			gNumBullets;


/****************************/
/*    CONSTANTS             */
/****************************/

#define	ROCK_POWER	2					// does .. damage to enemy

#define	TRACER_DURATION		(GAME_FPS)
#define	TRACER_POWER 		4


#define	FLAMETHROWER_DURATION 	(GAME_FPS/4)
#define	FLAMETHROWER_POWER 		6

#define	ELEPHANTGUN_DURATION	(GAME_FPS/2)
#define	ELEPHANTGUN_POWER	30

#define	PIE_DURATION		(GAME_FPS)
#define	PIE_POWER			4

#define	DOUBLESHOT_DURATION		(GAME_FPS)
#define	DOUBLESHOT_POWER			3

#define	TRIPLESHOT_DURATION		(GAME_FPS)
#define	TRIPLESHOT_POWER			4

#define	ROCKETGUN_DURATION		(GAME_FPS*3)
#define	ROCKETGUN_POWER			7
#define	ROCKET_REFIRE_TIME		(GAME_FPS/2)				// speed can shoot rocket

#define	HEATSEEK_DURATION		(GAME_FPS*6)
#define	HEATSEEK_POWER			3

#define	PIXIEDUST_DURATION		(GAME_FPS*2)
#define	PIXIEDUST_POWER			7
#define	PIXIE_REFIRE_TIME		(GAME_FPS/2)

/**********************/
/*     VARIABLES      */
/**********************/

long	gLastRocketTime	= 0;
long	gLastPixieTime	= 0;

#define	HeatSeekTarget	Ptr1

/*=========================== Rock ===============================================*/

#define	ROCK_SPEED	0xD0000L

const long		gRockDeltasX[] = {0,ROCK_SPEED,ROCK_SPEED,ROCK_SPEED,
							0,-ROCK_SPEED,-ROCK_SPEED,-ROCK_SPEED};
const long		gRockDeltasY[] = {-ROCK_SPEED,-ROCK_SPEED,0,ROCK_SPEED,
							ROCK_SPEED,ROCK_SPEED,0,-ROCK_SPEED};


/**************** THROW Rock ************************/

Boolean ThrowRock(void)
{
ObjNode *newNode;
long	dx,dy;
short		z,y,x;


				/* SEE IF READY TO SHOOT */

	if (!GetNewNeedState(kNeed_Attack))					// see if fire button pressed
		return false;


			/* SEE WHICH WAY TO MAKE IT GO */

	dx = gRockDeltasX[gMyDirection];
	dy = gRockDeltasY[gMyDirection];


			/* CALC COORDINATES */

	if (CalcWeaponStartCoords(dx,dy,&x,&y,&z,WS_THROW))	// calc start coord & see if in wall
		return(false);

					/* MAKE NEW OBJECT */

	newNode = MakeNewShape(GroupNum_Rock,ObjType_Rock,0,x,y,z,MoveRock,PLAYFIELD_RELATIVE);
	if (newNode == nil)
		return(false);

	newNode->CBits = CBITS_TOUCHABLE;
	newNode->TopOff = -60;					// set collision box (not activated yet)
	newNode->BottomOff = 60;
	newNode->LeftOff = -60;
	newNode->RightOff = 60;

	newNode->DX = dx;
	newNode->DY = dy;
	newNode->WeaponPower = ROCK_POWER;		// set weapon's power
	gNumBullets++;


				/* MAKE SHADOW */

	newNode->ShadowIndex = MakeShadow(newNode,SHADOWSIZE_SMALL); 	// allocate shadow & remember ptr to it

	newNode->YOffset.Int = -30;
	newNode->DZ = -0x80000L;					// start bouncing up

	PlaySound(SOUND_POP);
	StartMyThrow();
	return(true);
}


/*************** MOVE ROCK ********************/
//
// gThisNodePtr = ptr to current node
//

void MoveRock(void)
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
		DoPointCollision(gX.Int,gY.Int,CTYPE_BGROUND|CTYPE_MISC);


				/* MAKE IT FALL */

	gThisNodePtr->DZ += 0x22000L;								// add gravity
	gThisNodePtr->YOffset.L += gThisNodePtr->DZ;				// move it
	if ((gThisNodePtr->YOffset.Int > -1) || gNumCollisions)		// see if landed or hit something solid
	{
		gNumBullets--;										// dec count (auto deletes itself later)
		SwitchAnim(gThisNodePtr,1);								// BLOW IT UP!
		gThisNodePtr->CType = CTYPE_MYBULLET;					// activate collision
	}

	CalcObjectBox();
	UpdateObject();
}

/*=========================== Tracer ===============================================*/

#define	Tracer_SPEED	0x150000L

const long		gTracerDeltasX[] = {0,Tracer_SPEED,Tracer_SPEED,Tracer_SPEED,
							0,-Tracer_SPEED,-Tracer_SPEED,-Tracer_SPEED};
const long		gTracerDeltasY[] = {-Tracer_SPEED,-Tracer_SPEED,0,Tracer_SPEED,
							Tracer_SPEED,Tracer_SPEED,0,-Tracer_SPEED};

/**************** SHOOT Tracer ************************/

Boolean ShootTracer(void)
{
ObjNode *newNode;
long	dx,dy;
short		z,y,x;


				/* SEE IF READY TO SHOOT */

	if (!GetNeedState(kNeed_Attack))						// see if fire button pressed
		return(false);

	if (gFrames & b11)									// see if good interval
		return(false);

			/* SEE WHICH WAY TO MAKE IT GO */

	dx = gTracerDeltasX[gMyDirection];
	dy = gTracerDeltasY[gMyDirection];


			/* CALC COORDINATES */

	if (CalcWeaponStartCoords(dx,dy,&x,&y,&z,WS_SHOOT))	// calc start coord & see if in wall
		return(false);


					/* MAKE NEW OBJECT */

	newNode = MakeNewShape(GroupNum_Tracer,ObjType_Tracer,RandomRange(0,2),x,y,z,MoveBasicRico,PLAYFIELD_RELATIVE);
	if (newNode == nil)
		return(false);

	newNode->CType = CTYPE_MYBULLET;
	newNode->CBits = CBITS_TOUCHABLE;

	newNode->TopOff = -30;						// set collision box
	newNode->BottomOff = 0;
	newNode->LeftOff = -9;
	newNode->RightOff = 9;

	newNode->DX = dx;
	newNode->DY = dy;

	newNode->WeaponPower = TRACER_POWER;		// set weapon's power
	newNode->Health = TRACER_DURATION;

	gNumBullets++;

	PlaySound(SOUND_TRACERSHOT);

	DrawMyGun();								// make sure I'm doing correct anim
	return(true);
}



/*=========================== FLAMETHROWER ===============================================*/

#define	Flamethrower_SPEED		0x110000L
#define FlamethrowerFallDelta	Special1

const long		gFlamethrowerDeltasX[8] = {0,Flamethrower_SPEED,Flamethrower_SPEED,Flamethrower_SPEED,
							0,-Flamethrower_SPEED,-Flamethrower_SPEED,-Flamethrower_SPEED};
const long		gFlamethrowerDeltasY[8] = {-Flamethrower_SPEED,-Flamethrower_SPEED,0,Flamethrower_SPEED,
							Flamethrower_SPEED,Flamethrower_SPEED,0,-Flamethrower_SPEED};


/**************** SHOOT FLAMETHROWER ************************/

Boolean ShootFlamethrower(void)
{
ObjNode *newNode;
long	dx,dy;
short		z,y,x;


				/* SEE IF READY TO SHOOT */

	if (!GetNeedState(kNeed_Attack))						// see if fire button pressed
		return(false);

			/* SEE WHICH WAY TO MAKE IT GO */

	dx = gFlamethrowerDeltasX[gMyDirection];
	dy = gFlamethrowerDeltasY[gMyDirection];

			/* CALC COORDINATES */

	if (CalcWeaponStartCoords(dx,dy,&x,&y,&z,WS_SHOOT))	// calc start coord & see if in wall
		return(false);

	if (gMyDirection == AIM_DOWN)						// special tweak if down
		y += 30;

	y += MyRandomLong()&b11-2;								// scatter a bit
	x += MyRandomLong()&b11-2;

					/* MAKE NEW OBJECT */

	newNode = MakeNewShape(GroupNum_Flamethrower,ObjType_Flamethrower,0,x,y,z,MoveFlamethrower,PLAYFIELD_RELATIVE);
	if (newNode == nil)
		return(false);

	newNode->YOffset.Int = -32;

	newNode->CType = CTYPE_MYBULLET;
	newNode->CBits = CBITS_TOUCHABLE;

	newNode->TopOff = -16;					// set collision box
	newNode->BottomOff = 0;
	newNode->LeftOff = -9;
	newNode->RightOff = 9;

	newNode->Health = FLAMETHROWER_DURATION;

	newNode->DX = dx;
	newNode->DY = dy;
	newNode->FlamethrowerFallDelta = 0;

	newNode->WeaponPower = FLAMETHROWER_POWER;		// set weapon's power

	newNode->AnimSpeed = RandomRange(0x80,0x300);	// set random anim speed

	gNumBullets++;

	DrawMyGun();												// make sure I'm doing correct anim
	return(true);
}


/*************** MOVE FLAMETHROWER ********************/
//
// gThisNodePtr = ptr to current node
//

void MoveFlamethrower(void)
{
	if (--gThisNodePtr->Health < 0)					// see if disintegrates
	{
		gNumBullets--;
//		SwitchAnim(gThisNodePtr,3);					// splat anim
		DeleteObject(gThisNodePtr);	//------
//		gThisNodePtr->MoveCall = nil;
//		gThisNodePtr->CType = 0;
//		gThisNodePtr->AnimSpeed = (MyRandomLong()&b1111111111)+0x80;
		return;
	}

	GetObjectInfo();

	gX.L += gDX;									// move it
	gY.L += gDY;

	gThisNodePtr->FlamethrowerFallDelta += 0x3000;
	gThisNodePtr->YOffset.L += gThisNodePtr->FlamethrowerFallDelta;

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


/*=========================== ELEPHANTGUN ===============================================*/

#define	ELEPHANTGUN_SPEED	0x120000L

const long		gElephantGunDeltasX[] = {0,ELEPHANTGUN_SPEED,ELEPHANTGUN_SPEED,ELEPHANTGUN_SPEED,
							0,-ELEPHANTGUN_SPEED,-ELEPHANTGUN_SPEED,-ELEPHANTGUN_SPEED};
const long		gElephantGunDeltasY[] = {-ELEPHANTGUN_SPEED,-ELEPHANTGUN_SPEED,0,ELEPHANTGUN_SPEED,
							ELEPHANTGUN_SPEED,ELEPHANTGUN_SPEED,0,-ELEPHANTGUN_SPEED};

/**************** SHOOT ELEPHANTGUN ************************/
//
// ACTUALLY THIS IS THE SHOTGUN
//
//

Boolean ShootElephantGun(void)
{
ObjNode *newNode;
long	dx,dy;
short		z,y,x;

				/* SEE IF READY TO SHOOT */

	if (!GetNewNeedState(kNeed_Attack))					// see if fire button pressed
		return false;


			/* SEE WHICH WAY TO MAKE IT GO */

	dx = gElephantGunDeltasX[gMyDirection];
	dy = gElephantGunDeltasY[gMyDirection];

	if (CalcWeaponStartCoords(dx,dy,&x,&y,&z,WS_SHOOT))	// calc start coord & see if in wall
		return(false);


					/* MAKE NEW OBJECT */

	newNode = MakeNewShape(GroupNum_ElephantGun,ObjType_ElephantGun,0,x,y,z,
						MoveBasicBullet,PLAYFIELD_RELATIVE);
	if (newNode == nil)
		return(false);

	newNode->CType = CTYPE_MYBULLET;
	newNode->CBits = CBITS_TOUCHABLE;

	newNode->TopOff = -16;										// set collision box
	newNode->BottomOff = 0;
	newNode->LeftOff = -5;
	newNode->RightOff = 5;

	newNode->DX = dx;											// set deltas
	newNode->DY = dy;

	newNode->Health = ELEPHANTGUN_DURATION;
	newNode->WeaponPower = ELEPHANTGUN_POWER;					// set weapon's power

	newNode->YOffset.Int = -30;
	newNode->ShadowIndex = MakeShadow(newNode,SHADOWSIZE_TINY);	// allocate shadow & remember ptr to it

	gNumBullets++;

	PlaySound(SOUND_POP);

	DrawMyGun();												// make sure I'm doing correct anim
	return(true);
}


/*=========================== Pie ===============================================*/

#define	PIE_SPEED	0xf0000L

const long		gPieDeltasX[] = {0,PIE_SPEED,PIE_SPEED,PIE_SPEED,
							0,-PIE_SPEED,-PIE_SPEED,-PIE_SPEED};
const long		gPieDeltasY[] = {-PIE_SPEED,-PIE_SPEED,0,PIE_SPEED,
							PIE_SPEED,PIE_SPEED,0,-PIE_SPEED};


/**************** THROW Pie ************************/

Boolean ThrowPie(void)
{
ObjNode *newNode;
long	dx,dy;
short		z,y,x;


				/* SEE IF READY TO SHOOT */

	if (!GetNewNeedState(kNeed_Attack))					// see if fire button pressed
		return false;


			/* SEE WHICH WAY TO MAKE IT GO */

	dx = gPieDeltasX[gMyDirection];
	dy = gPieDeltasY[gMyDirection];


			/* CALC COORDINATES */

	if (CalcWeaponStartCoords(dx,dy,&x,&y,&z,WS_THROW))	// calc start coord & see if in wall
		return(false);

					/* MAKE NEW OBJECT */

	newNode = MakeNewShape(GroupNum_Pie,ObjType_Pie,gMyDirection,x,y,z,MovePie,PLAYFIELD_RELATIVE);
	if (newNode == nil)
		return(false);

	newNode->CType = CTYPE_MYBULLET;		// activate collision
	newNode->CBits = CBITS_TOUCHABLE;
	newNode->TopOff = -20;					// set collision box (not activated yet)
	newNode->BottomOff = 0;
	newNode->LeftOff = -15;
	newNode->RightOff = 15;

	newNode->DX = dx;
	newNode->DY = dy;
	newNode->WeaponPower = PIE_POWER;		// set weapon's power
	gNumBullets++;


				/* MAKE SHADOW */

	newNode->ShadowIndex = MakeShadow(newNode,SHADOWSIZE_SMALL); 	// allocate shadow & remember ptr to it

	newNode->YOffset.Int = -38;

	PlaySound(SOUND_POP);
	StartMyThrow();
	return(true);
}


/*************** MOVE PIE ********************/
//
// gThisNodePtr = ptr to current node
//

void MovePie(void)
{
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
		if (DoPointCollision(gX.Int,gY.Int,CTYPE_BGROUND|CTYPE_MISC))
		{
			ExplodePie(gThisNodePtr);
		}
	}

	CalcObjectBox();
	UpdateObject();
}

/***************** EXPLODE PIE *******************/

void ExplodePie(ObjNode *theNode)
{
	gNumBullets--;										// dec count (auto deletes itself later)
	SwitchAnim(theNode,8);								// BLOW IT UP!
	theNode->MoveCall = nil;							// stop from moving
	theNode->CType = 0;									// no longer harmful

	PlaySound(SOUND_PIESQUISH);
}



/*=========================== DOUBLESHOT ===============================================*/

#define	DOUBLESHOT_SPEED	0x120000L

const long		gDoubleShotDeltasX[] = {0,DOUBLESHOT_SPEED,DOUBLESHOT_SPEED,DOUBLESHOT_SPEED,
							0,-DOUBLESHOT_SPEED,-DOUBLESHOT_SPEED,-DOUBLESHOT_SPEED};
const long		gDoubleShotDeltasY[] = {-DOUBLESHOT_SPEED,-DOUBLESHOT_SPEED,0,DOUBLESHOT_SPEED,
							DOUBLESHOT_SPEED,DOUBLESHOT_SPEED,0,-DOUBLESHOT_SPEED};

/**************** SHOOT DOUBLESHOT ************************/
//
// DoubleShots are low-powered, auto-firing, guns.
//
//

Boolean ShootDoubleShot(void)
{
ObjNode *newNode;
long	dx,dy;
short		z,y,x;
Byte 	sub;

				/* SEE IF READY TO SHOOT */

	if (!GetNewNeedState(kNeed_Attack))					// see if fire button pressed
		return false;

			/* SEE WHICH WAY TO MAKE IT GO */

	dx = gDoubleShotDeltasX[gMyDirection];
	dy = gDoubleShotDeltasY[gMyDirection];

	if (CalcWeaponStartCoords(dx,dy,&x,&y,&z,WS_SHOOT))	// calc start coord & see if in wall
		return(false);


				/* SEE WHICH ANIM TO USE */

	if (dx == 0)
		sub = 0;
	else
	if (dy == 0)
		sub = 2;
	else
	if (dx < 0)
	{
		if (dy < 0)
			sub = 3;
		else
			sub = 1;
	}
	else
	{
		if (dy < 0)
			sub = 1;
		else
			sub = 3;
	}


					/* MAKE NEW OBJECT */

	newNode = MakeNewShape(GroupNum_DoubleShot,ObjType_DoubleShot,sub,x,y,z,
						MoveBasicBullet,PLAYFIELD_RELATIVE);
	if (newNode == nil)
		return(false);

	newNode->CType = CTYPE_MYBULLET;
	newNode->CBits = CBITS_TOUCHABLE;

	newNode->TopOff = -20;										// set collision box
	newNode->BottomOff = 0;
	newNode->LeftOff = -10;
	newNode->RightOff = 10;

	newNode->DX = dx;											// set deltas
	newNode->DY = dy;

	newNode->Health = DOUBLESHOT_DURATION;
	newNode->WeaponPower = DOUBLESHOT_POWER;					// set weapon's power

	newNode->YOffset.Int = -39;

	gNumBullets++;

	PlaySound(SOUND_RIFLESHOT);

	DrawMyGun();												// make sure I'm doing correct anim
	return(true);
}

/*=========================== TRIPLESHOT ===============================================*/

#define	TRIPLESHOT_SPEED	0x130000L

const long		gTripleShotDeltasX[] = {0,TRIPLESHOT_SPEED,TRIPLESHOT_SPEED,TRIPLESHOT_SPEED,
							0,-TRIPLESHOT_SPEED,-TRIPLESHOT_SPEED,-TRIPLESHOT_SPEED};
const long		gTripleShotDeltasY[] = {-TRIPLESHOT_SPEED,-TRIPLESHOT_SPEED,0,TRIPLESHOT_SPEED,
							TRIPLESHOT_SPEED,TRIPLESHOT_SPEED,0,-TRIPLESHOT_SPEED};


/**************** SHOOT TRIPLESHOT ************************/
//
// TripleShots are low-powered, auto-firing, guns.
//
//

Boolean ShootTripleShot(void)
{
ObjNode *newNode;
long	dx,dy;
short	z,y,x;
Byte	sub;
static	unsigned long lastShotFrame = 0;

				/* SEE IF READY TO SHOOT */

	if (!GetNeedState(kNeed_Attack))						// see if fire button pressed
		return(false);

	if ((gFrames - lastShotFrame) < 4)					// see if good interval
//	if (gFrames & b11)
		return(false);

	lastShotFrame = gFrames;							// remember when last shot was fired

			/* SEE WHICH WAY TO MAKE IT GO */

	dx = gTripleShotDeltasX[gMyDirection];
	dy = gTripleShotDeltasY[gMyDirection];

	if (CalcWeaponStartCoords(dx,dy,&x,&y,&z,WS_SHOOT))	// calc start coord & see if in wall
		return(false);

				/* SEE WHICH ANIM TO USE */

	if (dx == 0)
		sub = 0;
	else
	if (dy == 0)
		sub = 2;
	else
	if (dx < 0)
	{
		if (dy < 0)
			sub = 3;
		else
			sub = 1;
	}
	else
	{
		if (dy < 0)
			sub = 1;
		else
			sub = 3;
	}

					/* MAKE NEW OBJECT */

	newNode = MakeNewShape(GroupNum_TripleShot,ObjType_TripleShot,sub,x,y,z,
						MoveBasicBullet,PLAYFIELD_RELATIVE);
	if (newNode == nil)
		return(false);

	newNode->CType = CTYPE_MYBULLET;
	newNode->CBits = CBITS_TOUCHABLE;

	newNode->TopOff = -32;										// set collision box
	newNode->BottomOff = 0;
	newNode->LeftOff = -17;
	newNode->RightOff = 17;

	newNode->DX = dx;											// set deltas
	newNode->DY = dy;

	newNode->Health = TRIPLESHOT_DURATION;
	newNode->WeaponPower = TRIPLESHOT_POWER;					// set weapon's power

	newNode->YOffset.Int = -39;

	gNumBullets++;

	PlaySound(SOUND_MACHINEGUN);

	DrawMyGun();												// make sure I'm doing correct anim
	return(true);
}





/*=========================== ROCKETGUN ===============================================*/

#define	ROCKETGUN_SPEED	0x100000L

const long		gRocketGunDeltasX[] = {0,ROCKETGUN_SPEED,ROCKETGUN_SPEED,ROCKETGUN_SPEED,
							0,-ROCKETGUN_SPEED,-ROCKETGUN_SPEED,-ROCKETGUN_SPEED};
const long		gRocketGunDeltasY[] = {-ROCKETGUN_SPEED,-ROCKETGUN_SPEED,0,ROCKETGUN_SPEED,
							ROCKETGUN_SPEED,ROCKETGUN_SPEED,0,-ROCKETGUN_SPEED};

/**************** SHOOT ROCKETGUN ************************/

Boolean ShootRocketGun(void)
{
ObjNode *newNode;
long	dx,dy;
short		z,y,x;

				/* SEE IF READY TO SHOOT */

	if (!GetNeedState(kNeed_Attack))						// see if fire button pressed
	{
		return(false);
	}

	if ((gFrames - gLastRocketTime) < ROCKET_REFIRE_TIME)	// see if can shoot now
		return(false);

	gLastRocketTime = gFrames;

			/* SEE WHICH WAY TO MAKE IT GO */

	dx = gRocketGunDeltasX[gMyDirection];
	dy = gRocketGunDeltasY[gMyDirection];

	if (CalcWeaponStartCoords(dx,dy,&x,&y,&z,WS_SHOOT))	// calc start coord & see if in wall
		return(false);


					/* MAKE NEW OBJECT */

	newNode = MakeNewShape(GroupNum_RocketGun,ObjType_RocketGun,gMyDirection,x,y,z,
						MoveRocketGun,PLAYFIELD_RELATIVE);
	if (newNode == nil)
		return(false);

	newNode->CType = CTYPE_MYBULLET;
	newNode->CBits = CBITS_TOUCHABLE;

	newNode->TopOff = -16;										// set collision box
	newNode->BottomOff = 0;
	newNode->LeftOff = -5;
	newNode->RightOff = 5;

	newNode->DX = dx;											// set deltas
	newNode->DY = dy;

	newNode->Health = ROCKETGUN_DURATION;
	newNode->WeaponPower = ROCKETGUN_POWER;						// set weapon's power

	gNumBullets++;

	PlaySound(SOUND_MISSLELAUNCH);

	DrawMyGun();												// make sure I'm doing correct anim
	return(true);
}

/*************** MOVE ROCKET GUN ********************/

void MoveRocketGun(void)
{
register	ObjNode *newObj;

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

				/* SEE IF PUT SMOKE TRAIL */

	if (!(MyRandomLong()&b1))
	{
		newObj = MakeNewShape(GroupNum_RocketGun,ObjType_RocketGun,8,gX.Int,gY.Int,
					gThisNodePtr->Z,nil,PLAYFIELD_RELATIVE);
		if (newObj != nil)
		{
			newObj->AnimSpeed += MyRandomLong()&0xff;		// random anim speed
		}
	}

	CalcObjectBox();
	UpdateObject();
}


/*=========================== HEATSEEK ===============================================*/

#define	HEATSEEK_SPEED	0x110000L

const long		gHeatSeekDeltasX[] = {0,HEATSEEK_SPEED,HEATSEEK_SPEED,HEATSEEK_SPEED,
							0,-HEATSEEK_SPEED,-HEATSEEK_SPEED,-HEATSEEK_SPEED};
const long		gHeatSeekDeltasY[] = {-HEATSEEK_SPEED,-HEATSEEK_SPEED,0,HEATSEEK_SPEED,
							HEATSEEK_SPEED,HEATSEEK_SPEED,0,-HEATSEEK_SPEED};


/**************** SHOOT HEATSEEK ************************/

Boolean ShootHeatSeek(void)
{
ObjNode *newNode;
long	dx,dy;
short		z,y,x;

				/* SEE IF READY TO SHOOT */

	if (!GetNewNeedState(kNeed_Attack))					// see if fire button pressed
		return false;


			/* SEE WHICH WAY TO MAKE IT GO */

	dx = gHeatSeekDeltasX[gMyDirection];
	dy = gHeatSeekDeltasY[gMyDirection];

	if (CalcWeaponStartCoords(dx,dy,&x,&y,&z,WS_SHOOT))	// calc start coord & see if in wall
		return(false);


					/* MAKE NEW OBJECT */

	newNode = MakeNewShape(GroupNum_HeatSeek,ObjType_HeatSeek,0,x,y,z,
						MoveHeatSeek,PLAYFIELD_RELATIVE);
	if (newNode == nil)
		return(false);

	newNode->CType = CTYPE_MYBULLET;
	newNode->CBits = CBITS_TOUCHABLE;

	newNode->TopOff = -16;										// set collision box
	newNode->BottomOff = 0;
	newNode->LeftOff = -5;
	newNode->RightOff = 5;

	newNode->DX = dx;											// set deltas
	newNode->DY = dy;

	newNode->Health = HEATSEEK_DURATION;
	newNode->WeaponPower = HEATSEEK_POWER;						// set weapon's power

	FindHeatSeekTarget(newNode);

	PlaySound(SOUND_HEATSEEK);

	gNumBullets++;

	DrawMyGun();												// make sure I'm doing correct anim
	return(true);
}

/*************** FIND HEAT SEEK TARGET *****************/

void FindHeatSeekTarget(ObjNode *theNode)
{
ObjNode		*thisNodePtr;
ObjNode		*targetNode = nil;
short		dist,bestDist;
short		x,y;

	x = theNode->X.Int;
	y = theNode->Y.Int;

						/* SCAN FOR CLOSEST ENEMY */

	bestDist = 0x7fff;
	thisNodePtr = FirstNodePtr;
	do
	{
		if (thisNodePtr->CType & CTYPE_ENEMYA)
		{
			dist = (Absolute(thisNodePtr->X.Int - x) + Absolute(thisNodePtr->Y.Int - y))/2;

			if (dist < bestDist)
			{
				bestDist = dist;
				targetNode = thisNodePtr;
			}
		}

		thisNodePtr = thisNodePtr->NextNode;		// next node
	}
	while (thisNodePtr != nil);

					/* REMEMBER WHERE TO GO */

	if (bestDist != 0x7fff)
	{
		theNode->HeatSeekTarget = targetNode;
	}
	else
	{
		theNode->HeatSeekTarget = 0;				// no enemy found
	}
}


/*************** MOVE HEATSEEK ********************/

void MoveHeatSeek(void)
{
short	targetX,targetY;

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

				/* UPDATE HEAT SEEKER AIM */

	if (gThisNodePtr->HeatSeekTarget == 0)			// see if need to find new target
	{
		FindHeatSeekTarget(gThisNodePtr);
	}
	else
	{
		if (!(gThisNodePtr->HeatSeekTarget->CType & CTYPE_ENEMYA))		// see if target is no longer an enemy
		{
			FindHeatSeekTarget(gThisNodePtr);
			if (gThisNodePtr->HeatSeekTarget == 0)
				goto update;
		}

		targetX = ((ObjNode *)(gThisNodePtr->HeatSeekTarget))->X.Int;	// move towards target
		targetY = ((ObjNode *)(gThisNodePtr->HeatSeekTarget))->Y.Int;

		if (targetX < gX.Int)
			gDX -= 0x13000L;
		else
			gDX += 0x13000L;

		if (targetY < gY.Int)
			gDY -= 0x13000L;
		else
			gDY += 0x13000L;
	}

update:
	CalcObjectBox();
	UpdateObject();
}


/*=========================== PIXIE ===============================================*/

#define	PIXIEDUST_SPEED	0x0d0000L

const long		gPixieGunDeltasX[] = {0,PIXIEDUST_SPEED,PIXIEDUST_SPEED,PIXIEDUST_SPEED,
							0,-PIXIEDUST_SPEED,-PIXIEDUST_SPEED,-PIXIEDUST_SPEED};
const long		gPixieGunDeltasY[] = {-PIXIEDUST_SPEED,-PIXIEDUST_SPEED,0,PIXIEDUST_SPEED,
							PIXIEDUST_SPEED,PIXIEDUST_SPEED,0,-PIXIEDUST_SPEED};


/**************** SHOOT PIXIEDUST ************************/

Boolean ShootPixieDust(void)
{
ObjNode *newNode;
long	dx,dy;
short		z,y,x;

				/* SEE IF READY TO SHOOT */

	if (!GetNeedState(kNeed_Attack))						// see if fire button pressed
		return(false);

	if ((gFrames - gLastPixieTime) < PIXIE_REFIRE_TIME)	// see if can shoot now
		return(false);

	gLastPixieTime = gFrames;

			/* SEE WHICH WAY TO MAKE IT GO */

	dx = gPixieGunDeltasX[gMyDirection];
	dy = gPixieGunDeltasY[gMyDirection];

	if (CalcWeaponStartCoords(dx,dy,&x,&y,&z,WS_SHOOT))	// calc start coord & see if in wall
		return(false);


					/* MAKE NEW OBJECT */

	newNode = MakeNewShape(GroupNum_PixieDust,ObjType_PixieDust,0,x,y,z,
						MovePixieDust,PLAYFIELD_RELATIVE);
	if (newNode == nil)
		return(false);

	newNode->CType = CTYPE_MYBULLET;
	newNode->CBits = CBITS_TOUCHABLE;

	newNode->TopOff = -32;										// set collision box
	newNode->BottomOff = 0;
	newNode->LeftOff = -8;
	newNode->RightOff = 8;

	newNode->DX = dx;											// set deltas
	newNode->DY = dy;

	newNode->Health = PIXIEDUST_DURATION;
	newNode->WeaponPower = PIXIEDUST_POWER;						// set weapon's power

	newNode->YOffset.Int = -20;

	gNumBullets++;

	PlaySound(SOUND_PIXIEDUST);

	DrawMyGun();												// make sure I'm doing correct anim
	return(true);
}


/*************** MOVE PIXIE DUST ********************/

void MovePixieDust(void)
{
register	ObjNode *newObj;

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

				/* SEE IF PUT SMOKE TRAIL */

	if (!(MyRandomLong()&b1))
	{
		newObj = MakeNewShape(GroupNum_PixieDust,ObjType_PixieDust,1,gX.Int,gY.Int,
					gThisNodePtr->Z,nil,PLAYFIELD_RELATIVE);
		if (newObj != nil)
		{
			newObj->AnimSpeed += MyRandomLong()&0x1ff;		// random anim speed
			newObj->YOffset.Int = gThisNodePtr->YOffset.Int;		// same dist off ground
		}
	}

	CalcObjectBox();
	UpdateObject();
}









