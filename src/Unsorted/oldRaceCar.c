/****************************/
/*      RACECAR             */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/****************************/
/*    EXTERNALS             */
/****************************/
#include "myglobals.h"
#include "racecar.h"
#include "sound2.h"
#include "objecttypes.h"
#include "shape.h"
#include "misc.h"
#include "object.h"
#include "playfield.h"
#include "enemy.h"
#include "weapon.h"
#include "store.h"
#include "io.h"
#include "infobar.h"
#include "tileanim.h"
#include "picture.h"
#include "myguy.h"
#include "collision.h"

extern	union_gX;
extern	union_gY;
extern	short			gMyX,gMyY,gMyInitX,gMyInitY;
extern	ObjNode		*gMyNodePtr;
extern	Byte		gMyMode;
extern	short		gNumCollisions;
extern	long		gMySpeed,gFrames;
extern	Boolean		gFinishedArea,gTeleportingFlag;
extern	long			gDX,gDY,gSumDX,gSumDY;
extern	ObjNode			*gThisNodePtr,*FirstNodePtr;
extern	long			gMySumDX,gMySumDY,gMyDX,gMyDY;
extern	short				gScrollX,gScrollY;
extern	short		gMyDirection;

/****************************/
/*    CONSTANTS             */
/****************************/

#define	MAX_CAR_SPEED		0x140000L

#define	MAX_CARS			40

enum
{
	FLINE_NONE,
	FLINE_PRIMARY,
	FLINE_SECONDARY
};

#define	LAPS_PER_RACE		3

#define LAPNUM_X		250
#define LAPNUM_Y		130

/**********************/
/*     VARIABLES      */
/**********************/

#define	CarRotation		Special1
#define	CarMaxSpeed		Special2

#define	LapNum			Special3

#define	FinishLineInfo		Flag1
#define	PrevFLInfo	Flag2
				// 0 = not on line
				// 1 = on primary
				// 2 = on secondary

#define	CarNum		Flag3

Boolean			gRaceOverFlag;

Byte			gMyCarDirection;

long			gRotSinTable[16] = {
									0,						// 0
									0x61f7L,				// 22
									0xb504L,				// 45
									0xec83L,				// 67
									0x10000L,				// 90
									0xec83L,				// 112
									0xb504L,				// 135
									0x61f7L,				// 157
									0,						// 180
									-0x61f7L,				// 202
									-0xb504L,				// 225
									-0xec83L,				// 247
									-0x10000L,				// 270
									-0xec83L,				// 292
									-0xb504L,				// 315
									-0x61f7L				// 337
								};

long			gRotCosTable[16] = {
									-0x10000L,				// 270
									-0xec83L,				// 292
									-0xb504L,				// 315
									-0x61f7L,				// 337
									0,						// 0
									0x61f7L,				// 22
									0xb504L,				// 45
									0xec83L,				// 67
									0x10000L,				// 90
									0xec83L,				// 112
									0xb504L,				// 135
									0x61f7L,				// 157
									0,						// 180
									-0x61f7L,				// 202
									-0xb504L,				// 225
									-0xec83L				// 247
								};


Rect	gFinishLine_Primary,gFinishLine_Secondary;

short	gNumCars;								// # enemy cars on track

Byte	gCarLapList[MAX_CARS];					// for each car, the # of laps completed


/******************* SEE IF DO RACE **********************/

void SeeIfDoRace(void)
{
	return;		//---------------


	InitRaceTrack();
	PlayRaceTrack();

	RaceComesToEnd();

	FadeOutGameCLUT();
	StopMusic();
}


/****************** INIT RACETRACK ******************/

void InitRaceTrack(void)
{
	gRaceOverFlag = FALSE;
	gNumCars = 0;

	StopMusic();
	ZapAllAddedSounds();

	ClearGlobalFlags();
	InitObjectManager();

	LoadPlayfield("\p:data:maps:RaceTrack.map");
	LoadTileSet("\p:data:maps:RaceTrack.tileset");
	LoadShapeTable("\p:data:shapes:RaceTrack.shapes",GROUP_AREA_SPECIFIC,DONT_GET_PALETTE);
	FadeOutGameCLUT();														// fade out old screen
	LoadBackground_Direct("\p:data:images:border.image",GET_PALETTE);		// load border pict & USE PALETTE

	InitMyCar();
	InitEnemies();
	InitBullets();
	BuildItemList();											// creates playfield item list (used to be called in InitPlayfield)
	InitPlayfield();											// must init playfield *after* InitMe!
	EraseStore();												// (sets up interlace fill)

	ShowScore();
	ShowCoins();
	ShowWeaponIcon();
	DrawObjects();
	DisplayPlayfield();
	EraseObjects();

	PlaySong(SONG_ID_RACE);

	FadeInGameCLUT();											// fade in new screen
}


/******************* PLAY RACETRACK *****************/

void PlayRaceTrack(void)
{
Boolean	finish = FALSE;

	do
	{
		RegulateSpeed(GAME_SPEED);

		ReadKeyboard();
		MoveObjects();
		ScrollPlayfield();							// do playfield updating
		UpdateTileAnimation();
		DrawObjects();
		DisplayPlayfield();
		UpdateInfoBar();
		EraseObjects();

		if (GetKeyState(KEY_Q))						// see if key quit
			CleanQuit();

		if (GetKeyState(KEY_P))						// see if pause
		{
			while (GetKeyState2(KEY_P));
			while (!GetKeyState2(KEY_P));
			while (GetKeyState2(KEY_P));
		}

		DoSoundMaintenance();						// (must be after readkeyboard)

		if (GetKeyState(KEY_ESC))					// see if abort game
			gRaceOverFlag = TRUE;

		if (GetKeyState(KEY_E))						//------- see if skip to next level
			finish = TRUE;


	} while((!gRaceOverFlag) && (!finish));
}

/****************** RACE COMES TO AN END ********************/
//
// Several seconds of slowdown to finish race
//

void RaceComesToEnd(void)
{
short	i;

	for (i=0; i < (GAME_FPS*2); i++)
	{
		RegulateSpeed(GAME_SPEED);

		ReadKeyboard();
		MoveObjects();
		ScrollPlayfield();							// do playfield updating
		UpdateTileAnimation();
		DrawObjects();
		DisplayPlayfield();
		UpdateInfoBar();
		EraseObjects();

		DoSoundMaintenance();						// (must be after readkeyboard)
	}
}


/************************ INIT MY CAR ********************/

void InitMyCar(void)
{
	FindMyInitCoords();								// see where to put me

	gMyCarDirection = 4;
	gTeleportingFlag = FALSE;

				/* GET NEW OBJECT FOR ME */

	gMyX = gMyInitX;
	gMyY = gMyInitY;

	gMyNodePtr = MakeNewShape(GroupNum_MyCar,ObjType_MyCar,gMyCarDirection,
					gMyX,gMyY,50,MoveMyCar,PLAYFIELD_RELATIVE);

	if (gMyNodePtr == nil)
		DoFatalAlert("\pCouldnt init MyCar!");

	gMyNodePtr->CType = CTYPE_MYGUY;
	gMyNodePtr->CBits = CBITS_ALLSOLID;

	gMyNodePtr->TopOff = -14;					// set box
	gMyNodePtr->BottomOff = 14;
	gMyNodePtr->LeftOff = -14;
	gMyNodePtr->RightOff = 14;

	gMyNodePtr->TileMaskFlag = FALSE;			// doesnt use tile masks

	gMyNodePtr->FinishLineInfo = FLINE_NONE;	// not on finish line
	gMyNodePtr->PrevFLInfo = FLINE_NONE;
	gMyNodePtr->LapNum = 0;						// 0 laps

	PutLapsSignal(0);
}


/***************** MOVE MY CAR ****************/

void MoveMyCar(void)
{
long		accX,accY;
Byte		sides;
Byte		prevLap;

	prevLap = gThisNodePtr->LapNum;					// remember lap #

	GetObjectInfo();

	if (gFrames & 1)
	{
		if (GetKeyState(KEY_K4))					// check rotation
			gMyCarDirection--;
		if (GetKeyState(KEY_K6))
			gMyCarDirection++;
		gMyCarDirection  &= 0xf;
		if (gThisNodePtr->SubType != gMyCarDirection)	// verify animation
			SwitchAnim(gThisNodePtr,gMyCarDirection);
	}

				/* HANDLE ACCELERATION */

	if (GetKeyState(KEY_SPACE) && (!gRaceOverFlag))
	{
		accX = gRotSinTable[gMyCarDirection];		// get acceleration
		accY = gRotCosTable[gMyCarDirection];
	}
	else
	{
		accX = accY = 0;
		gDX = gDX*9/10;								// even distribution friction
		gDY = gDY*9/10;
	}

	gDX += accX;
	gDY += accY;

	if (GetMapTileAttribs(gX.Int,gY.Int) & TILE_ATTRIB_FRICTION)		// see if on friction tile
	{
		gDX = gDX*6/7;
		gDY = gDY*6/7;
	}


	switch(gMyCarDirection)							// alignment friction
	{
		case	0:
		case	8:
				gDX = gDX*4/5;
				break;

		case	1:
		case	15:
				gDX = gDX*9/10;
				break;

		case	3:
		case	13:
				gDY = gDY*9/10;
				break;

		case	4:
		case	12:
				gDY = gDY*4/5;
				break;
	}

	if (gDX > MAX_CAR_SPEED)						// check max speed
		gDX = MAX_CAR_SPEED;
	else
	if (gDX < -MAX_CAR_SPEED)
		gDX = -MAX_CAR_SPEED;

	if (gDY > MAX_CAR_SPEED)
		gDY = MAX_CAR_SPEED;
	else
	if (gDY < -MAX_CAR_SPEED)
		gDY = -MAX_CAR_SPEED;

	gX.L += gDX;											// move it
	gY.L += gDY;

					/* DO COLLISION DETECT */

	CalcObjectBox();
	gSumDX = gDX;
	gSumDY = gDY;
	sides = HandleCollisions(CTYPE_BGROUND);
	if (gNumCollisions > 0)									// if hit, then stop
	{
		if ((sides & SIDE_BITS_TOP) || (sides & SIDE_BITS_BOTTOM))
			gDY = -gDY/2;

		if ((sides & SIDE_BITS_LEFT) || (sides & SIDE_BITS_RIGHT))
			gDX = -gDX/2;
	}

	CheckForFinishLine();									// update finish line check
	if (gThisNodePtr->LapNum >= LAPS_PER_RACE)				// see if finished race
		gRaceOverFlag = TRUE;
	else
	if (gThisNodePtr->LapNum != prevLap)					// see if need lap signal
	{
		PutLapsSignal(gThisNodePtr->LapNum);
	}

	UpdateMyCar();
}



/***************** UPDATE MY CAR *****************/

void UpdateMyCar(void)
{

	gMyX = gX.Int;
	gMyY = gY.Int;

	gMySumDX = gSumDX;
	gMySumDY = gSumDY;
	gMyDX = gDX;
	gMyDY = gDY;

	gMyDirection = gMyCarDirection>>1;						// convert direction for scolling window

	DoMyScreenScroll();

	CalcObjectBox();
	UpdateObject();
}


//===============================================================================================

/************************ ADD BAD CAR ********************/

Boolean AddBadCar(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;


	newObj = MakeNewShape(GroupNum_BadCar,ObjType_BadCar,2,
					itemPtr->x,itemPtr->y,40,MoveBadCar,PLAYFIELD_RELATIVE);

	if (newObj == nil)
		return(FALSE);

	newObj->CType = CTYPE_ENEMYA;
	newObj->CBits = CBITS_ALLSOLID;

	newObj->TopOff = -14;					// set box
	newObj->BottomOff = 14;
	newObj->LeftOff = -14;
	newObj->RightOff = 14;

	newObj->CarRotation = 2;				// aim right

	newObj->CarMaxSpeed = (long)RandomRange(0x000a,0x0017)<<16; //gCarSpeeds[itemPtr->parm[1]];		// set speed

	newObj->TileMaskFlag = FALSE;						// doesnt use tile masks

	newObj->FinishLineInfo = FLINE_NONE;	// not on finish line
	newObj->PrevFLInfo = FLINE_NONE;
	newObj->LapNum = 0;						// 0 laps

	newObj->CarNum = gNumCars;				// set global car #

	gCarLapList[gNumCars++] = 0;			// init global lap counter

	return(TRUE);
}


/*************** MOVE BAD CAR *******************/

void MoveBadCar(void)
{
long		accX,accY,maxSpeed;
Byte		sides,rot;

	GetObjectInfo();

	switch(GetAlternateTileInfo(gX.Int, gY.Int))			// see which way to go
	{
		case	ALT_TILE_DIR_UP:
				gThisNodePtr->CarRotation = 0;
				gDX = gDX*9/10;
				break;

		case	ALT_TILE_DIR_UP_RIGHT:
				gThisNodePtr->CarRotation = 1;
				break;

		case	ALT_TILE_DIR_RIGHT:
				gThisNodePtr->CarRotation = 2;
				gDY = gDY*9/10;
				break;

		case	ALT_TILE_DIR_DOWN_RIGHT:
				gThisNodePtr->CarRotation = 3;
				break;

		case	ALT_TILE_DIR_DOWN:
				gThisNodePtr->CarRotation = 4;
				gDX = gDX*9/10;
				break;

		case	ALT_TILE_DIR_DOWN_LEFT:
				gThisNodePtr->CarRotation = 5;
				break;

		case	ALT_TILE_DIR_LEFT:
				gThisNodePtr->CarRotation = 6;
				gDY = gDY*9/10;
				break;

		case	ALT_TILE_DIR_LEFT_UP:
				gThisNodePtr->CarRotation = 7;
				break;

	}

	rot = gThisNodePtr->CarRotation;

	if (gThisNodePtr->SubType != rot)						// verify animation
		SwitchAnim(gThisNodePtr,rot);

				/* HANDLE ACCELERATION */

	if (!gRaceOverFlag)
	{
		accX = gRotSinTable[rot<<1];				// get acceleration
		accY = gRotCosTable[rot<<1];
	}
	else
	{
		accX = accY = 0;
		gDY = gDY*9/10;
	}

	gDX += accX;
	gDY += accY;

	if (GetMapTileAttribs(gX.Int,gY.Int) & TILE_ATTRIB_FRICTION)		// see if on friction tile
	{
		gDX = gDX*4/5;
		gDY = gDY*4/5;
	}

	maxSpeed = gThisNodePtr->CarMaxSpeed;

	if (gDX > maxSpeed)						// check max speed
		gDX = maxSpeed;
	else
	if (gDX < -maxSpeed)
		gDX = -maxSpeed;

	if (gDY > maxSpeed)
		gDY = maxSpeed;
	else
	if (gDY < -maxSpeed)
		gDY = -maxSpeed;

	gX.L += gDX;											// move it
	gY.L += gDY;

					/* DO COLLISION DETECT */

	CalcObjectBox();
	gSumDX = gDX;
	gSumDY = gDY;
	sides = HandleCollisions(CTYPE_BGROUND|CTYPE_MYGUY|CTYPE_ENEMYA);
	if (gNumCollisions > 0)									// if hit, then stop
	{
		if ((sides & SIDE_BITS_TOP) || (sides & SIDE_BITS_BOTTOM))
			gDY = -gDY/8;

		if ((sides & SIDE_BITS_LEFT) || (sides & SIDE_BITS_RIGHT))
			gDX = -gDX/8;
	}

	CheckForFinishLine();										// update finish line check
	gCarLapList[gThisNodePtr->CarNum] = gThisNodePtr->LapNum;	// update global lap counter

				/* UPDATE */

	CalcObjectBox();
	UpdateObject();
}


//===============================================================================================

/************************ ADD FINISH LINE ********************/
//
// Calculate the double collision areas for the finish line, the primary & secondary
//

Boolean AddFinishLine(ObjectEntryType *itemPtr)
{
					/* CALC BOX FOR PRIMARY LINE */

	gFinishLine_Primary.top = itemPtr->y;
	gFinishLine_Primary.bottom = itemPtr->y+(itemPtr->parm[0]*TILE_SIZE);
	gFinishLine_Primary.left = itemPtr->x;
	gFinishLine_Primary.right = itemPtr->x+TILE_SIZE;

					/* CALC BOX FOR SECONDARY LINE */

	gFinishLine_Secondary.top = itemPtr->y;
	gFinishLine_Secondary.bottom = itemPtr->y+(itemPtr->parm[0]*TILE_SIZE);
	gFinishLine_Secondary.left = gFinishLine_Primary.right;
	gFinishLine_Secondary.right = gFinishLine_Secondary.left+TILE_SIZE;

	return(TRUE);
}



/******************* CHECK FOR FINISH LINE ***********************/
//
// Uses gThisNodePtr,gX/Y
//

void CheckForFinishLine(void)
{
						/* SEE IF HIT PRIMARY */

	if	((gX.Int >= gFinishLine_Primary.left) && (gX.Int <= gFinishLine_Primary.right) &&
		(gY.Int >= gFinishLine_Primary.top) && (gY.Int <= gFinishLine_Primary.bottom))
	{
		if (gThisNodePtr->FinishLineInfo == FLINE_NONE)				// if came from nowhere then check prev
		{
			if (gThisNodePtr->PrevFLInfo == FLINE_SECONDARY)		// if prev was 2ndary, then we lapped
			{
				gThisNodePtr->LapNum++;								// inc lap counter
			}
		}
		gThisNodePtr->PrevFLInfo = gThisNodePtr->FinishLineInfo;
		gThisNodePtr->FinishLineInfo = FLINE_PRIMARY;
	}

						/* SEE IF HIT SECONDARY */
	else
	if	((gX.Int >= gFinishLine_Secondary.left) && (gX.Int <= gFinishLine_Secondary.right) &&
		(gY.Int >= gFinishLine_Secondary.top) && (gY.Int <= gFinishLine_Secondary.bottom))
	{
		gThisNodePtr->PrevFLInfo = FLINE_SECONDARY;
		gThisNodePtr->FinishLineInfo = FLINE_SECONDARY;
	}

					/* NOT ON FINISH LINE */
	else
		gThisNodePtr->FinishLineInfo = FLINE_NONE;
}



/****************** PUT LAPS SIGNAL ******************/

void PutLapsSignal(short lapNum)
{
register	ObjNode			*newObj;

	lapNum = LAPS_PER_RACE-lapNum-1;			// convert to anim #

	newObj = MakeNewShape(GroupNum_Laps,ObjType_Laps,lapNum,
			LAPNUM_X+gScrollX,LAPNUM_Y+gScrollY,NEAREST_Z,MoveLapSignal,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return;

	newObj->TileMaskFlag = FALSE;						// doesnt use tile masks
	newObj->Special1 = GAME_FPS*3;
}


/*************** MOVE LAPS SIGNAL ****************/

void MoveLapSignal(void)
{
	gThisNodePtr->X.Int = gScrollX+LAPNUM_X;		// keep aligned with screen
	gThisNodePtr->Y.Int = gScrollY+LAPNUM_Y;

	if ((gFrames&b11) == 0)									// make flash
		gThisNodePtr->DrawFlag = !gThisNodePtr->DrawFlag;

	if (gThisNodePtr->Special1-- < 0)						// see if done
		DeleteObject(gThisNodePtr);
}



