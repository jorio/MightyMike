/****************************/
/*      MIKE - MAIN         */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/****************************/
/*    EXTERNALS             */
/****************************/
#include "myglobals.h"
#include "objecttypes.h"
#include "window.h"
#include "picture.h"
#include "playfield.h"
#include "myguy.h"
#include "object.h"
#include "enemy.h"
#include "infobar.h"
#include "cinema.h"
#include "misc.h"
#include "bonus.h"
#include "sound2.h"
#include "miscanims.h"
#include "weapon.h"
#include "shape.h"
#include "io.h"
#include "main.h"
#include "input.h"
#include "version.h"
#include "externs.h"
#include <SDL.h>
#include <string.h>

/****************************/
/*    CONSTANTS             */
/****************************/



/**********************/
/*     VARIABLES      */
/**********************/

PrefsType	gGamePrefs;

Boolean		gAbortGameFlag,gWinFlag,gFinishedArea;

long		gFrames=0;				// # frames tick counter
Byte		gSceneNum,gAreaNum;

Byte		gPlayerMode = ONE_PLAYER;
Byte		gCurrentPlayer;

static	PlayerSaveType	gPlayerSaveData[2];

static	const char*		p1name = ":MightyMike:PeteP1Swap.data";
static	const char* 	p2name = ":MightyMike:PeteP2Swap.data";

static	Str255		gSaveName = ":MightyMike:PowerPeteSavedGameData0";
static	Str255		gSaveName2x = ":MightyMike:PowerPeteSavedGameData2x00";
short		gLoadOldGameNum;
Boolean		gLoadOldGameFlag;


long		someLong;

Byte		gStartingScene = 0, gStartingArea = 0;
Byte		gDifficultySetting = DIFFICULTY_NORMAL;

Boolean		gIsASavedGame[2];

short		gMainAppRezFile;

Boolean		gScreenScrollFlag = true;

MikeFixed	gTweenFrameFactor			= { .L = 0x00000000 };
MikeFixed	gOneMinusTweenFrameFactor	= { .L = 0x00010000 };

static const uint32_t	kDebugTextUpdateInterval = 50;
static uint32_t			gDebugTextFrameAccumulator = 0;
static uint32_t			gDebugTextLastUpdatedAt = 0;
static char				gDebugTextBuffer[1024];

/*****************/
/* TOOLBOX INIT  */
/*****************/

void ToolBoxInit(void)
{
	TODO_REWRITE_THIS_MINOR();
#if 0
 	MaxApplZone();
 	MoreMasters();	MoreMasters();	MoreMasters();	MoreMasters();
 	MoreMasters();	MoreMasters();	MoreMasters();	MoreMasters();
 	MoreMasters();	MoreMasters();	MoreMasters();	MoreMasters();

	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents ( everyEvent, REMOVE_ALL_EVENTS);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();
#endif

	gMainAppRezFile = CurResFile();

}


/****************** INIT GAME ******************/

void InitGame(void)
{
	gFrames = 0;
	gStartingArea = 0;								// assume area 0 unless loading old game (see below)

	CleanMemory();

	gCurrentPlayer = ONE_PLAYER;					// start with player 1
	gDifficultySetting = gGamePrefs.difficulty;		// by default, use difficulty from prefs
	InitWeaponsList();
	InitScore();
	InitCoins();
	InitFreeLives();
	InitHealth();
	InitKeys();

				/* INIT STUFF FOR 2 PLAYER GAME */

	gCurrentPlayer = TWO_PLAYER;					// check load player 2
	if (gLoadOldGameFlag)							// try to get saved game stuff to start with
		LoadGame(gLoadOldGameNum);
	gPlayerSaveData[1].beenSavedData = false;
	gPlayerSaveData[1].newAreaFlag = false;
	gPlayerSaveData[1].scene = gStartingScene;
	gPlayerSaveData[1].area = gStartingArea;
	gPlayerSaveData[1].donePlayingFlag = false;

	gCurrentPlayer = ONE_PLAYER;					// check load player 1
	if (gLoadOldGameFlag)							// try to get saved game stuff to start with
		LoadGame(gLoadOldGameNum);
	gPlayerSaveData[0].beenSavedData = false;		// no save data exists
	gPlayerSaveData[0].newAreaFlag = false;
	gPlayerSaveData[0].scene = gStartingScene;
	gPlayerSaveData[0].area = gStartingArea;
	gPlayerSaveData[0].donePlayingFlag = false;



	gWinFlag = false;
	gAbortGameFlag = false;
	gAbortDemoFlag = false;

				/* LOAD ART WHICH IS ALWAYS WITH US */

	if (gGamePrefs.pfSize != PFSIZE_SMALL)
		LoadShapeTable(":data:shapes:infobar2.shapes",GROUP_INFOBAR,DONT_GET_PALETTE);
	else
		LoadShapeTable(":data:shapes:infobar.shapes",GROUP_INFOBAR,DONT_GET_PALETTE);

	LoadShapeTable(":data:shapes:weapon.shapes",GROUP_WEAPONS,DONT_GET_PALETTE);
	LoadShapeTable(":data:shapes:main.shapes",GROUP_MAIN,DONT_GET_PALETTE);
}



/******************** INIT AREA *********************/

void InitArea(void)
{
	FadeOutGameCLUT();

	OptimizeMemory();

	InitThermometer();											// prepare thermometer dial
	PlayAreaMusic();											// start area song
	FillThermometer(10);
	LoadAreaArt();												// load art
	LoadAreaSound();											// load sound
	FillThermometer(100);

	gFinishedArea = false;

	ClearGlobalFlags();
	InitObjectManager();

	BuildItemList();											// creates playfield item list (used to be called in InitPlayfield)
	InitMe();
	InitEnemies();
	InitBullets();
	InitKeys();
	CountBunnies();
	LoadCurrentPlayer(false);									// load current player's info (for 2 player mode)
	InitPlayfield();											// must init playfield *after* InitMe!
	EraseStore();												// (sets up interlace fill)
	PutPlayerSignal(true);										// put p1 or p2 message

	EraseCLUT();
	LoadBorderImage();

	ShowScore();
	ShowCoins();
	ShowKeys();
	ShowNumBunnies();
	ShowWeaponIcon();
	ShowHealth();
	ShowLives();
	DrawObjects();
	DisplayPlayfield();
	EraseObjects();

	FadeInGameCLUT();											// fade in new screen
}


/*************** LOAD AREA ART ****************/
//
// Load the necessary Screen, Maps, Tiles, and Sprites for this area.
//

void LoadAreaArt(void)
{
	MaxMem(&someLong);														// clean up again
	CompactMem(maxSize);

	switch(gSceneNum)
	{
		case	SCENE_JURASSIC:
				LoadTileSet(":data:maps:Jurassic.tileset");
				FillThermometer(20);
				LoadShapeTable(":data:shapes:jurassic1.shapes",GROUP_AREA_SPECIFIC,DONT_GET_PALETTE);
				FillThermometer(40);
				LoadShapeTable(":data:shapes:jurassic2.shapes",GROUP_AREA_SPECIFIC2,DONT_GET_PALETTE);
				FillThermometer(60);
				switch(gAreaNum)
				{
					case 0:
						LoadPlayfield(":data:maps:Jurassic.map-1");
						break;
					case 1:
						LoadPlayfield(":data:maps:Jurassic.map-2");
						break;
					case 2:
						LoadPlayfield(":data:maps:Jurassic.map-3");
						break;
				}
				FillThermometer(80);
				break;

		case	SCENE_CANDY:
				LoadTileSet(":data:maps:Candy.tileset");
				FillThermometer(20);
				LoadShapeTable(":data:shapes:Candy1.shapes",GROUP_AREA_SPECIFIC,DONT_GET_PALETTE);
				FillThermometer(40);
				LoadShapeTable(":data:shapes:Candy2.shapes",GROUP_AREA_SPECIFIC2,DONT_GET_PALETTE);
				FillThermometer(60);
				switch(gAreaNum)
				{
					case 0:
						LoadPlayfield(":data:maps:Candy.map-1");
						break;
					case 1:
						LoadPlayfield(":data:maps:Candy.map-2");
						break;
					case 2:
						LoadPlayfield(":data:maps:Candy.map-3");
						break;
				}
				FillThermometer(80);
				break;

		case	SCENE_CLOWN:
				LoadTileSet(":data:maps:Clown.tileset");
				FillThermometer(20);
				LoadShapeTable(":data:shapes:clown1.shapes",GROUP_AREA_SPECIFIC,DONT_GET_PALETTE);
				FillThermometer(40);
				LoadShapeTable(":data:shapes:clown2.shapes",GROUP_AREA_SPECIFIC2,DONT_GET_PALETTE);
				FillThermometer(60);
				switch(gAreaNum)
				{
					case 0:
						LoadPlayfield(":data:maps:Clown.map-1");
						break;
					case 1:
						LoadPlayfield(":data:maps:Clown.map-2");
						break;
					case 2:
						LoadPlayfield(":data:maps:Clown.map-3");
						break;
				}
				FillThermometer(80);
				break;

		case	SCENE_FAIRY:
				LoadTileSet(":data:maps:fairy.tileset");
				FillThermometer(20);
				LoadShapeTable(":data:shapes:fairy2.shapes",GROUP_AREA_SPECIFIC2,DONT_GET_PALETTE);
				FillThermometer(40);
				LoadShapeTable(":data:shapes:fairy1.shapes",GROUP_AREA_SPECIFIC,DONT_GET_PALETTE);
				FillThermometer(60);
				switch(gAreaNum)
				{
					case 0:
						LoadPlayfield(":data:maps:fairy.map-1");
						break;
					case 1:
						LoadPlayfield(":data:maps:fairy.map-2");
						break;
					case 2:
						LoadPlayfield(":data:maps:fairy.map-3");
						break;
				}
				FillThermometer(80);
  				break;

		case	SCENE_BARGAIN:
				LoadTileSet(":data:maps:bargain.tileset");
				FillThermometer(20);
				LoadShapeTable(":data:shapes:bargain1.shapes",GROUP_AREA_SPECIFIC,DONT_GET_PALETTE);
				FillThermometer(40);
				LoadShapeTable(":data:shapes:bargain2.shapes",GROUP_AREA_SPECIFIC2,DONT_GET_PALETTE);
				FillThermometer(60);
				switch(gAreaNum)
				{
					case 0:
						LoadPlayfield(":data:maps:bargain.map-1");
						break;
					case 1:
						LoadPlayfield(":data:maps:bargain.map-2");
						break;
					case 2:
						LoadPlayfield(":data:maps:bargain.map-3");
						break;
				}
				FillThermometer(80);
  				break;
	}
}



/******************* PLAY AREA *****************/
//
// Main loop for playing an area of a department of the game.
//

void PlayArea(void)
{
long	r;

	r = MyRandomLong();

	do
	{

		if (!gGamePrefs.uncappedFramerate)
		{
			gTweenFrameFactor.L			= 0x00010000;				// reset frame interpolation (factor=1: force new coordinates)
			gOneMinusTweenFrameFactor.L	= 0x00000000;

			gFrames++;												// one more simulation frame
			
			if (gShakeyScreenCount)
				gShakeyScreenCount--;

			ReadKeyboard();
			MoveObjects();
			SortObjectsByY();										// sort 'em
			ScrollPlayfield();										// do playfield updating
			UpdateTileAnimation();
			DrawObjects();
			DisplayPlayfield();
			UpdateInfoBar();
			EraseObjects();
			PresentIndexedFramebuffer();
			RegulateSpeed(GAME_SPEED_MICROSECONDS);

			gDebugTextFrameAccumulator++;
		}
		else
		{
			gTweenFrameFactor.L			= 0x00000000;				// reset frame interpolation (factor=0: start interpolating from previous frame)
			gOneMinusTweenFrameFactor.L	= 0x00010000;

						/* SIMULATION FRAMES */
			
			uint32_t timeSinceSim = GAME_SPEED_SDL;					// force simulation to run once when we enter this function

			while (timeSinceSim >= GAME_SPEED_SDL)					// we need to run some simulation frames
			{
				gFrames++;											// one more simulation frame

				if (gShakeyScreenCount)
					gShakeyScreenCount--;

						/* ROUTINES THAT ONLY NEED TO BE RUN ONCE PER SIM TIC */
				
				ReadKeyboard();
				MoveObjects();
				SortObjectsByY();									// sort 'em
				UpdateTileAnimation();
				UpdateInfoBar();

				timeSinceSim -= GAME_SPEED_SDL;						// catch up
			}

						/* GRAPHICS FRAMES */
			
			const uint32_t timeAtEndOfSim = SDL_GetTicks();

			while (timeSinceSim < GAME_SPEED_SDL)					// render as many graphics frames as we can until it's time to run the simulation again
			{
				GAME_ASSERT(gTweenFrameFactor.L >= 0 && gTweenFrameFactor.L <= 0x10000);
				
				ScrollPlayfield();									// also tweens camera position
				DrawObjects();
				DisplayPlayfield();
				EraseObjects();
				PresentIndexedFramebuffer();

				gDebugTextFrameAccumulator++;

				timeSinceSim = SDL_GetTicks() - timeAtEndOfSim;

				// Update tween factor at end of loop
				// (meaning tween factor at 1st iteration is 0, i.e. 1st graphics frame = end of previous simulation frame)
				gTweenFrameFactor.L			= 0x10000 * timeSinceSim / GAME_SPEED_SDL;
				gOneMinusTweenFrameFactor.L	= 0x10000 - gTweenFrameFactor.L;
			}
		}
		



					/* DEBUG INFO */

		//if (gGamePrefs.debugInfoInTitleBar)
		{
			uint32_t ticksNow = SDL_GetTicks();
			uint32_t ticksElapsed = ticksNow - gDebugTextLastUpdatedAt;
			if (ticksElapsed >= kDebugTextUpdateInterval)
			{
				float fps = 1000 * gDebugTextFrameAccumulator / (float)ticksElapsed;
				snprintf(
					gDebugTextBuffer, sizeof(gDebugTextBuffer),
					"Mighty Mike %s - fps:%d - x:%d y:%d",
					PROJECT_VERSION,
					(int)round(fps),
					gMyX,
					gMyY
				);
				SDL_SetWindowTitle(gSDLWindow, gDebugTextBuffer);
				gDebugTextFrameAccumulator = 0;
				gDebugTextLastUpdatedAt = ticksNow;
			}
		}

//		if (GetKeyState(kKey_Pause))			    // see if pause
//			ShowPaused();

		DoSoundMaintenance(true);						// (must be after readkeyboard)

		if (GetNewNeedState(kNeed_Radar))				// see if show radar
			DisplayBunnyRadar();

		if (GetNewNeedState(kNeed_UIPause))				// see if abort game
			gAbortGameFlag = AskIfQuit();

		if (GetSDLKeyState(SDL_SCANCODE_PERIOD) && GetSDLKeyState(SDL_SCANCODE_N))	// see if skip to next level
		{
			gNumBunnies = 1;
			DecBunnyCount();
		}

		if (gNumBunnies <= 0)					// special hack to fix reported bug!?!?
			DecBunnyCount();

#if _DEBUG
		if (GetNewSDLKeyState(SDL_SCANCODE_F8))
		{
			DumpIndexedTGA(
				"playfield.tga",
				PF_BUFFER_WIDTH,
				PF_BUFFER_HEIGHT,
				*gPFBufferHandle
			);
		}
#endif

		if (GetNewSDLKeyState(SDL_SCANCODE_F9))
			gScreenScrollFlag = !gScreenScrollFlag;

	} while((!gGlobFlag_MeDoneDead) && (!gAbortGameFlag) &&
			(!gFinishedArea) && (!gAbortDemoFlag));
}



/********************** SWITCH PLAYER **********************/

void SwitchPlayer(void)
{

	gCurrentPlayer ^= 1;						// switch

				/* REMEMBER WHERE THE OTHER PLAYER WAS */

	gSceneNum = gPlayerSaveData[gCurrentPlayer].scene;
	gAreaNum = gPlayerSaveData[gCurrentPlayer].area;
}


/***************** SAVE CURRENT PLAYER **********************/

void SaveCurrentPlayer(void)
{
Str255		saveName;
OSErr		iErr;
short		fRefNum;
long		numBytes;
FSSpec		mySpec;

					/* GET CORRECT SAVE FILENAME */

	if (gCurrentPlayer == ONE_PLAYER)
		BlockMove(p1name,saveName,p1name[0]+1);						 // copy pathname
	else
		BlockMove(p2name,saveName,p2name[0]+1);						 // copy pathname

				/* GET FSSPEC & SEE IF DELETE OLD FILE */

	iErr = FSMakeFSSpec(gPrefsFolderVRefNum, gPrefsFolderDirID,
						saveName,&mySpec);
	if(iErr != fnfErr)
		iErr = FSpDelete(&mySpec);


					/*  CREATE THE FILE */

	iErr = FSpCreate(&mySpec,'MMik','sAve',-1);
	if (iErr != noErr)
		DoFatalAlert("Cannot Create Player Save File.  Disk may be locked or full.");


				/*  OPEN THE FILE */

	iErr = FSpOpenDF(&mySpec,fsWrPerm,&fRefNum);
	if (iErr != noErr)
		DoFatalAlert("Cannot Open Player Save File.");

				/******************/
				/* WRITE THE DATA */
				/******************/
														// WRITE INVENTORY LIST
	numBytes = sizeof(WeaponType)*MAX_WEAPONS;
	iErr = FSWrite(fRefNum, &numBytes, (Ptr) gMyWeapons);
	if (iErr != noErr)
		DoFatalAlert("Cannot Write to Player Save File.  Disk may be locked or full.");

														// WRITE OBJECT LIST
	numBytes = sizeof(ObjNode)*MAX_OBJECTS;
	iErr = FSWrite(fRefNum, &numBytes, (Ptr) ObjectList);
	if (iErr != noErr)
		DoFatalAlert("Cannot Write to Player Save File.  Disk may be locked or full.");

														// WRITE FREE NODE STACK
	numBytes = sizeof(ObjNode *)*MAX_OBJECTS;
	iErr = FSWrite(fRefNum, &numBytes, (Ptr) &FreeNodeStack[0]);
	if (iErr != noErr)
		DoFatalAlert("Cannot Write to Player Save File.  Disk may be locked or full.");

														// WRITE MASTER ITEM LIST
	numBytes = sizeof(ObjectEntryType)*gNumItems;
	iErr = FSWrite(fRefNum, &numBytes, (Ptr) gMasterItemList);
	if (iErr != noErr)
		DoFatalAlert("Cannot Write to Player Save File.  Disk may be locked or full.");


			/*  CLOSE THE FILE */

	iErr = FSClose(fRefNum);
	if (iErr != noErr)
		DoFatalAlert("Cannot Close Player Save File.  Disk may be locked or full.");


				/* SAVE MISC SMALL STUFF */

	gPlayerSaveData[gCurrentPlayer].beenSavedData = true;			// data has been saved
	gPlayerSaveData[gCurrentPlayer].score = gScore;
	gPlayerSaveData[gCurrentPlayer].numCoins = gNumCoins;
	gPlayerSaveData[gCurrentPlayer].lives = gNumLives;
	gPlayerSaveData[gCurrentPlayer].currentWeaponType = gCurrentWeaponType;
	gPlayerSaveData[gCurrentPlayer].scene = gSceneNum;
	gPlayerSaveData[gCurrentPlayer].area = gAreaNum;
	gPlayerSaveData[gCurrentPlayer].numEnemies = gNumEnemies;
	gPlayerSaveData[gCurrentPlayer].numBunnies = gNumBunnies;
	gPlayerSaveData[gCurrentPlayer].myX = gMyX;
	gPlayerSaveData[gCurrentPlayer].myY = gMyY;
	gPlayerSaveData[gCurrentPlayer].nodeStackFront = NodeStackFront;
	gPlayerSaveData[gCurrentPlayer].numObjects = NumObjects;
	gPlayerSaveData[gCurrentPlayer].firstNodePtr = FirstNodePtr;
	gPlayerSaveData[gCurrentPlayer].myNodePtr = gMyNodePtr;
	gPlayerSaveData[gCurrentPlayer].myBlinkieTimer = gMyBlinkieTimer;
	gPlayerSaveData[gCurrentPlayer].numItemsInInventory = gNumWeaponsIHave;
	gPlayerSaveData[gCurrentPlayer].inventoryIndex_weapon = gCurrentWeaponIndex;
	gPlayerSaveData[gCurrentPlayer].keys[0] = gMyKeys[0];
	gPlayerSaveData[gCurrentPlayer].keys[1] = gMyKeys[1];
	gPlayerSaveData[gCurrentPlayer].keys[2] = gMyKeys[2];
	gPlayerSaveData[gCurrentPlayer].keys[3] = gMyKeys[3];
	gPlayerSaveData[gCurrentPlayer].keys[4] = gMyKeys[4];
	gPlayerSaveData[gCurrentPlayer].keys[5] = gMyKeys[5];
	gPlayerSaveData[gCurrentPlayer].myHealth = gMyHealth;
	gPlayerSaveData[gCurrentPlayer].myMaxHealth = gMyMaxHealth;
	gPlayerSaveData[gCurrentPlayer].lastNonDeathX = gLastNonDeathX;
	gPlayerSaveData[gCurrentPlayer].lastNonDeathY = gLastNonDeathY;
	gPlayerSaveData[gCurrentPlayer].oldItemIndex = (Ptr)gMasterItemList;
}


/***************** LOAD CURRENT PLAYER **********************/
//
// INPUT : minimal = true if only read basics & inventory list (for 2player save game crud)
//

void LoadCurrentPlayer(Boolean minimal)
{
Str255	loadName;
OSErr		iErr;
short		fRefNum;
long		numBytes;
register	ObjNode		*theNode;
Ptr			tempPtr;
ptrdiff_t	diff;
FSSpec		mySpec;

	if (gPlayerMode == ONE_PLAYER)								// only for 2 player mode!
		return;

	if (gPlayerSaveData[gCurrentPlayer].beenSavedData == false)	// see if any data exists
	{
		InitHealth();											// init some basics
		InitScore();
		InitFreeLives();
		InitCoins();
		InitWeaponsList();

				/* SEE IF RESTORING AN OLD GAME */

		if (gLoadOldGameFlag)
		{
			LoadGame(gLoadOldGameNum);
		}

		return;
	}

					/* GET CORRECT LOAD FILENAME */

	if (gCurrentPlayer == ONE_PLAYER)
		BlockMove(p1name,loadName,p1name[0]+1);						 // copy pathname
	else
		BlockMove(p2name,loadName,p2name[0]+1);						 // copy pathname

					/* GET FSSPEC */

	iErr = FSMakeFSSpec(gPrefsFolderVRefNum, gPrefsFolderDirID,loadName,&mySpec);

				/*  OPEN THE FILE */

	iErr = FSpOpenDF(&mySpec,fsRdPerm,&fRefNum);
	if (iErr != noErr)
		DoFatalAlert("Cannot Open Player Save File.");

					/*****************/
					/* READ THE DATA */
					/*****************/

															// READ INVENTORY LIST
	numBytes = sizeof(WeaponType)*MAX_WEAPONS;
	iErr = FSRead(fRefNum, &numBytes, (Ptr) gMyWeapons);
	if (iErr != noErr)
		DoFatalAlert("Cannot Read Player Save File.");

	if (!minimal)
	{
		if (!gPlayerSaveData[gCurrentPlayer].newAreaFlag)		// IF NOT NEW AREA, THEN LOAD OLD AREA INFO
		{

																// READ OBJECT LIST
			numBytes = sizeof(ObjNode)*MAX_OBJECTS;
			iErr = FSRead(fRefNum, &numBytes, (Ptr) ObjectList);
			if (iErr != noErr)
				DoFatalAlert("Error Reading from Player Save File.");

																// READ FREE NODE STACK
			numBytes = sizeof(ObjNode *)*MAX_OBJECTS;
			iErr = FSRead(fRefNum, &numBytes, (Ptr) FreeNodeStack);
			if (iErr != noErr)
				DoFatalAlert("Error Reading from Player Save File.");

																// READ MASTER ITEM LIST
			numBytes = sizeof(ObjectEntryType)*gNumItems;
			iErr = FSRead(fRefNum, &numBytes, (Ptr) gMasterItemList);
			if (iErr != noErr)
				DoFatalAlert("Error Reading from Player Save File.");


						/* LOAD MISC SMALL STUFF */

			gNumEnemies = 				gPlayerSaveData[gCurrentPlayer].numEnemies;
			gNumBunnies = 				gPlayerSaveData[gCurrentPlayer].numBunnies;
			gMyX = gMyNodePtr->X.Int =	gPlayerSaveData[gCurrentPlayer].lastNonDeathX;
			gMyY = gMyNodePtr->Y.Int =	gPlayerSaveData[gCurrentPlayer].lastNonDeathY;
			NodeStackFront = 			gPlayerSaveData[gCurrentPlayer].nodeStackFront;
			NumObjects = 				gPlayerSaveData[gCurrentPlayer].numObjects;
			FirstNodePtr = 				gPlayerSaveData[gCurrentPlayer].firstNodePtr;
			gMyNodePtr =  				gPlayerSaveData[gCurrentPlayer].myNodePtr;
		}
		else
			gPlayerSaveData[gCurrentPlayer].newAreaFlag = false;		// not new anymore
	}

				/*  CLOSE THE FILE */

	iErr = FSClose(fRefNum);
	if (iErr != noErr)
		DoFatalAlert("Cannot Close Player Save File.");


			/* LOAD STANDARD INFO (stuff that always gets restored) */

	gScore = 					gPlayerSaveData[gCurrentPlayer].score;
	gNumCoins = 				gPlayerSaveData[gCurrentPlayer].numCoins;
	gNumLives =					gPlayerSaveData[gCurrentPlayer].lives;
	gCurrentWeaponType = 		gPlayerSaveData[gCurrentPlayer].currentWeaponType;
	gMyBlinkieTimer = 			gPlayerSaveData[gCurrentPlayer].myBlinkieTimer;
	gNumWeaponsIHave = 			gPlayerSaveData[gCurrentPlayer].numItemsInInventory;
	gCurrentWeaponIndex = 		gPlayerSaveData[gCurrentPlayer].inventoryIndex_weapon;
	gMyKeys[0] = 				gPlayerSaveData[gCurrentPlayer].keys[0];
	gMyKeys[1] = 				gPlayerSaveData[gCurrentPlayer].keys[1];
	gMyKeys[2] = 				gPlayerSaveData[gCurrentPlayer].keys[2];
	gMyKeys[3] = 				gPlayerSaveData[gCurrentPlayer].keys[3];
	gMyKeys[4] = 				gPlayerSaveData[gCurrentPlayer].keys[4];
	gMyKeys[5] = 				gPlayerSaveData[gCurrentPlayer].keys[5];
	gMyHealth =					gPlayerSaveData[gCurrentPlayer].myHealth;
	gMyMaxHealth = 				gPlayerSaveData[gCurrentPlayer].myMaxHealth;
	gLastNonDeathX = 			gPlayerSaveData[gCurrentPlayer].lastNonDeathX;
	gLastNonDeathY = 			gPlayerSaveData[gCurrentPlayer].lastNonDeathY;


	if (!minimal)
	{
				/*******************************/
				/* RESET ALL OBJECTS' POINTERS */
				/*******************************/
//
// Because the sprites & things have moved around, some of the
// object node records will be invalid.  It is necessary to recalculate
// the shape header pointer, and then calc the distance it moved.  Then,
// all future shape addresses can be offset by that amount.
//

		if (FirstNodePtr == nil)									// see if there are any objects
			return;
		theNode = FirstNodePtr;
		do
		{
			if (theNode->Genre == SPRITE_GENRE)						// only adjust sprite objects
			{
				tempPtr = gSHAPE_HEADER_Ptrs[theNode->SpriteGroupNum][theNode->Type];	// get shape header ptr
				diff = tempPtr - theNode->SHAPE_HEADER_Ptr;								// calc how far it moved
				theNode->SHAPE_HEADER_Ptr = tempPtr;									// reset to new location
				theNode->AnimsList = (Ptr)(theNode->AnimsList + diff);					// adjust anim ptr by the distance
			}

					/* ADJUST ALL ITEM INDEX PTRS */

			if (theNode->ItemIndex != nil)
			{
				diff = (Ptr)gMasterItemList - gPlayerSaveData[gCurrentPlayer].oldItemIndex;	// how far did it move?
				theNode->ItemIndex = (ObjectEntryType *)((Ptr)theNode->ItemIndex + diff);
			}

			theNode = (ObjNode *)theNode->NextNode;
		}while (theNode != nil);
	}
}

/******************* TRY TO SAVE BOTH PLAYERS ************************/

void TryToSaveBothPlayers(short gameNum)
{
long	score,numCoins,numLives,currentWeaponType;
long	scene,area,numWeapons,currentWeaponIndex;
long	myHealth,myMaxHealth;
long	difficultySetting;
WeaponType	weaponList[MAX_WEAPONS];
FSSpec		mySpec;


	SaveGame(gameNum,true);			// save this guy's game	for sure


			/* IF PLAYER 0 AND OTHER PLAYER HASNT GONE YET, DONT SAVE P2 */

	if ((gCurrentPlayer == 0) && (!gPlayerSaveData[gCurrentPlayer^1].beenSavedData))
	{
		if (!gLoadOldGameFlag)			// if not playing saved game, delete p2's file
			goto delete;
		return;
	}

		/* OR IF OTHER PLAYER IS DEAD OR DONE, THEN DELETE OTHER PLAYER SAVE FILE */

	if (gPlayerSaveData[gCurrentPlayer^1].donePlayingFlag)
	{
delete:
		gSaveName2x[strlen(gSaveName2x)-2] = '0'+(gCurrentPlayer^1);		// tag player # to end of filename
		gSaveName2x[strlen(gSaveName2x)-1] = '0'+gameNum;					// tag game # to end of filename
		FSMakeFSSpec(gPrefsFolderVRefNum, gPrefsFolderDirID,gSaveName2x,&mySpec);
		FSpDelete(&mySpec);
		return;
	}

				/* KEEP CURRENT PLAYER'S VITALS */

	score = gScore;
	numCoins = gNumCoins;
	numLives = gNumLives;
	currentWeaponType = gCurrentWeaponType;
	scene = gSceneNum;
	area = gAreaNum;
	numWeapons = gNumWeaponsIHave;
	currentWeaponIndex = gCurrentWeaponIndex;
	myHealth = gMyHealth;
	myMaxHealth = gMyMaxHealth;
	difficultySetting = gDifficultySetting;
	BlockMove((Ptr)&gMyWeapons[0],(Ptr)weaponList,sizeof(WeaponType)*MAX_WEAPONS);


				/* SWITCH TO OTHER PLAYER & LOAD HIS DATA */

	SwitchPlayer();
	LoadCurrentPlayer(true);			// load minimal "other" player


					/* SAVE OTHER PLAYER'S GAME */

	SaveGame(gameNum,false);					// save him


				/* GO BACK TO CURRENT PLAYER & RESTORE DATA */

	SwitchPlayer();

	gScore = score;
	gNumCoins = numCoins;
	gNumLives = numLives;
	gCurrentWeaponType = currentWeaponType;
	gSceneNum = scene;
	gAreaNum = area;
	gNumWeaponsIHave = numWeapons;
	gCurrentWeaponIndex = currentWeaponIndex;
	gMyHealth = myHealth;
	gMyMaxHealth = myMaxHealth;
	gDifficultySetting = difficultySetting;
	BlockMove((Ptr)weaponList,(Ptr)&gMyWeapons[0],sizeof(WeaponType)*MAX_WEAPONS);
}



/***************** SAVE GAME **********************/
//
// Note, Scene & Area #'s are still set to what they were for last level, not
// level we're about to go to.
//
// INPUT : atNextFlag = true if want to save @ beginning of next level
//

void SaveGame(short	gameNum, Boolean atNextFlag)
{
OSErr		iErr;
short		fRefNum;
long		numBytes;
FSSpec		mySpec;
Byte		scene,area;
SaveGameFile	saveGame;


	if (atNextFlag)
	{
				/* CALC NEXT SCENE & AREA */

		if (gAreaNum == 2)
		{
			area = 0;
			scene = gSceneNum+1;
		}
		else
		{
			area = gAreaNum+1;
			scene = gSceneNum;
		}
	}
	else
	{
		area = gAreaNum;
		scene = gSceneNum;
	}
	
					/*************************/
					/* PREPARE DATA TO WRITE */
					/*************************/

	memset(&saveGame, 0xFF, sizeof(SaveGameFile));

	_Static_assert(sizeof(gMyWeapons) == sizeof(saveGame.myWeapons), "size mismatch: weapons on disk vs in memory");

	snprintf(saveGame.magic, sizeof(saveGame.magic), "%s", SAVEGAMEFILE_MAGIC);
	memcpy(saveGame.myWeapons, gMyWeapons, sizeof(saveGame.myWeapons));
	saveGame.score					= gScore;
	saveGame.numCoins				= gNumCoins;
	saveGame.numLives				= gNumLives;
	saveGame.currentWeaponType		= gCurrentWeaponType;
	saveGame.sceneNum				= scene;
	saveGame.areaNum				= area;
	saveGame.numWeaponsIHave		= gNumWeaponsIHave;
	saveGame.currentWeaponIndex		= gCurrentWeaponIndex;
	saveGame.myHealth				= gMyHealth;
	saveGame.myMaxHealth			= gMyMaxHealth;
	saveGame.difficultySetting		= gDifficultySetting;

				/***************************************/
				/* GET FSSPEC & SEE IF DELETE OLD FILE */
				/***************************************/

	if (gPlayerMode == ONE_PLAYER)
	{
		gSaveName[strlen(gSaveName)-1] = '0'+gameNum;					// tag game # to end of filename
		iErr = FSMakeFSSpec(gPrefsFolderVRefNum, gPrefsFolderDirID,gSaveName,&mySpec);
	}
	else
	{
		gSaveName2x[strlen(gSaveName2x)-2] = '0'+gCurrentPlayer;			// tag player # to end of filename
		gSaveName2x[strlen(gSaveName2x)-1] = '0'+gameNum;					// tag game # to end of filename
		iErr = FSMakeFSSpec(gPrefsFolderVRefNum, gPrefsFolderDirID,gSaveName2x,&mySpec);
	}
	if(iErr != fnfErr)
		iErr = FSpDelete(&mySpec);


					/*  CREATE THE FILE */

	iErr = FSpCreate(&mySpec,'MMik','sav2',-1);
	if (iErr != noErr)
	{
		DoAlert("Cannot Create Player Save File.  Disk may be locked or full.");
		return;
	}

				/*  OPEN THE FILE */

	iErr = FSpOpenDF(&mySpec,fsWrPerm,&fRefNum);
	if (iErr != noErr)
		DoFatalAlert("Cannot Open Player Save File.");

				/* WRITE THE DATA */

	numBytes = sizeof(SaveGameFile);
	iErr = FSWrite(fRefNum, &numBytes, (Ptr)&saveGame);
	if (iErr != noErr || numBytes != sizeof(SaveGameFile))
		DoAlert("Cannot write save file.");

			/*  CLOSE THE FILE */

	iErr = FSClose(fRefNum);
	if (iErr != noErr)
		DoAlert("Cannot Close Player Save File.  Disk may be locked or full.");
}



/***************** LOAD GAME **********************/

void LoadGame(short	gameNum)
{
OSErr		iErr;
short		fRefNum;
long		numBytes;
FSSpec		mySpec;
SaveGameFile	saveGame;

	InitKeys();

	gIsASavedGame[gCurrentPlayer] = false;			// assume nothing to restore

					/* GET FSSPEC */

	if (gPlayerMode == ONE_PLAYER)
	{
		gSaveName[strlen(gSaveName)-1] = '0'+gameNum;					// tag game # to end of filename
		iErr = FSMakeFSSpec(gPrefsFolderVRefNum, gPrefsFolderDirID,gSaveName,&mySpec);
	}
	else
	{
		gSaveName2x[strlen(gSaveName2x)-2] = '0'+gCurrentPlayer;			// tag player # to end of filename
		gSaveName2x[strlen(gSaveName2x)-1] = '0'+gameNum;					// tag game # to end of filename
		iErr = FSMakeFSSpec(gPrefsFolderVRefNum, gPrefsFolderDirID,gSaveName2x,&mySpec);
	}

				/*  OPEN THE FILE */

	iErr = FSpOpenDF(&mySpec,fsRdPerm,&fRefNum);
	if (iErr != noErr)										// if error, then no saved game yet
	{
		gStartingScene = gStartingArea = 0;
		return;
	}

				/* READ THE DATA */

	numBytes = sizeof(SaveGameFile);
	iErr = FSRead(fRefNum, &numBytes, (Ptr)&saveGame);
	GAME_ASSERT_MESSAGE(iErr == noErr, "Cannot Read Player Save File.");
	GAME_ASSERT(numBytes == sizeof(SaveGameFile));

				/*  CLOSE THE FILE */

	iErr = FSClose(fRefNum);
	GAME_ASSERT_MESSAGE(iErr == noErr, "Cannot Close Player Save File.");

				/* COPY TO GLOBALS */

	if (0 != strncmp(saveGame.magic, SAVEGAMEFILE_MAGIC, sizeof(saveGame.magic)))
		GAME_ASSERT_MESSAGE(false, "Save File has incorrect magic header.");
	
	_Static_assert(sizeof(gMyWeapons) == sizeof(saveGame.myWeapons), "size mismatch: weapons on disk vs in memory");
	
	memcpy(gMyWeapons, saveGame.myWeapons, sizeof(saveGame.myWeapons));
	gScore					= saveGame.score;
	gNumCoins				= saveGame.numCoins;
	gNumLives				= saveGame.numLives;
	gCurrentWeaponType		= saveGame.currentWeaponType;
	gSceneNum				= saveGame.sceneNum;
	gAreaNum				= saveGame.areaNum;
	gNumWeaponsIHave		= saveGame.numWeaponsIHave;
	gCurrentWeaponIndex		= saveGame.currentWeaponIndex;
	gMyHealth				= saveGame.myHealth;
	gMyMaxHealth			= saveGame.myMaxHealth;
	gDifficultySetting		= saveGame.difficultySetting;

	
	gIsASavedGame[gCurrentPlayer] = true;					// it exists

	if (gAreaNum >= 3)
	{
		gAreaNum = 0;
		gSceneNum++;
	}

	gStartingScene = gSceneNum;
	gStartingArea = gAreaNum;
}


/**************** DO 1 PLAYER GAME ********************/

void Do1PlayerGame(void)
{
short	maxScenes;

				/* SEE HOW MANY LEVELS TO PLAY */

	switch(gDifficultySetting)
	{
//		case	DIFFICULTY_NORMAL:
//				maxScenes = 4;
//				break;
		case	DIFFICULTY_EASY:
				maxScenes = 3;
				break;
		case	DIFFICULTY_NORMAL:
		case	DIFFICULTY_HARD:
				maxScenes = 5;
				break;
		default:
				DoFatalAlert("Unknown difficulty setting");
	}

	for (gSceneNum = gStartingScene; gSceneNum < maxScenes; gSceneNum++)	// do each Scene
	{
		for (gAreaNum = gStartingArea; gAreaNum < 3; gAreaNum++)
		{
			gStartingArea = 0;									// reset starting area, so next level will start @ 0
			DoOverheadMap();									// show the overhead map
			if (gAbortDemoFlag)
				goto game_over;

			if (gAreaNum == 0)
				DoSceneScreen();								// if 1st area, show the Scene/World intro
				if (gAbortDemoFlag)
					goto game_over;

			InitArea();											// init the area
retry_area:	PlayArea();											// PLAY IT

			if (gAbortGameFlag || gAbortDemoFlag)				// see if ABORTED (via demo or ESC key)
				goto game_over;

			if (gGlobFlag_MeDoneDead)							// see if Im DEAD
			{
							/* HANDLE DEATH */

				if (--gNumLives)								// see if got another life
				{
					ReviveMe();
					goto retry_area;							// try again
				}
				else
					goto game_over;								// out of lives, so game is OVER
			}

					/* FINISHED AREA */

			FadeOutGameCLUT();
			ShowBonusScreen();
			OptimizeMemory();
			if (gAreaNum == 2)
			{
				OptimizeMemory();
			}
		}
	}

	gWinFlag = true;											// if got here, then must have won


game_over:
	FadeOutGameCLUT();
	CleanMemory();												// clean up memory

	SaveDemoData();												// if was recording demo, save it

	if (!gGameIsDemoFlag)										// if demo, then dont bother with final stuff
	{
		if (gWinFlag)											// see if won or lost
			DoWinScreen();
		else
			DoLoseScreen();

		AddHighScore(gScore);									// try to add to high scores
	}
	InitScreenBuffers();										// restore offscreen buffers
}


/***************** DO 2 PLAYER GAME *******************/
//
// NOTE: A player's gSceneNum value == 3 when it has completed a scene
//		it is saved in playerdata as such so when it is reloaded, the
//		code knows if it should display the world intro screen.
//
// NOTE: Don't start cleaning up memory after PlayArea until the player data
//		is saved since some memory is saved by SavePlayer!
//

void Do2PlayerGame(void)
{
	gAreaNum = 0;
	gSceneNum = gStartingScene;

do_dept:
				/* DOUBLE CHECK THIS - HACK FIX */

	gSceneNum = gPlayerSaveData[gCurrentPlayer].scene;
	gAreaNum = gPlayerSaveData[gCurrentPlayer].area;
	if (gAreaNum >= 3)									// see if next scene/department
	{
		gAreaNum = 0;
		gSceneNum++;
	}

	DoOverheadMap();									// show the overhead map
	DoSceneScreen();									// show the world intro
	goto ok_screens;

do_area:
	DoOverheadMap();									// only show the overhead map

ok_screens:
	InitArea();											// init the area
again:
	PlayArea();											// play it

														// SEE IF I WAS KILLED
														//====================
	if (gGlobFlag_MeDoneDead)
		goto me_dead;

	if (gAbortGameFlag)									// see if game was ABORTED
		goto game_over;

					/*****************/
					/* FINISHED AREA */
					/*****************/

	ShowBonusScreen();

	if (gAreaNum == 2)
	{
		OptimizeMemory();
	}

	gAreaNum++;
	InitKeys();				// hack thing
	SaveCurrentPlayer();										// save current player's info
	OptimizeMemory();

	gPlayerSaveData[gCurrentPlayer].newAreaFlag = true; 		// get ready for new area

																// SEE IF NEXT SCENE
																//==================
	if (gAreaNum >= 3)
	{
																// SEE IF WON THE GAME
																//====================
		if ((gSceneNum+1) >= MAX_SCENES)
		{
			DoWinScreen();
			AddHighScore(gScore);									// try to add to high scores
			gPlayerSaveData[gCurrentPlayer].donePlayingFlag = true;	// not coming back again
			if (gPlayerSaveData[gCurrentPlayer^1].donePlayingFlag)	// check if both are done
				goto game_over;
		}
	}
	goto next_player;

					/*************/
					/* DO  DEATH */
					/*************/
me_dead:
	if (--gNumLives)											// see if got another dude
	{
		ReviveMe();
		SaveCurrentPlayer();
		OptimizeMemory();
		goto next_player;
	}
	else														// DUDE IS TOTALLY GONE
	{
		gPlayerSaveData[gCurrentPlayer].donePlayingFlag = true;	// this one aint comin' back
		SaveCurrentPlayer();
		OptimizeMemory();
		DoLoseScreen();											// do lose screen
		AddHighScore(gScore);									// try to add to high scores
		if (gPlayerSaveData[gCurrentPlayer^1].donePlayingFlag)	// check if both are dead
			goto game_over;
		goto next_player;										// the other dude is still alive
	}

					/*****************/
					/*  NEXT PLAYER  */
					/*****************/

next_player:
	if (gPlayerSaveData[gCurrentPlayer^1].donePlayingFlag)		// check if other player is DEACTIVATED
	{
		if (gAreaNum >= 3)										// see if ready for next scene/dept
		{
			gAreaNum = 0;
			gSceneNum++;										// (already checked for finish under FINISHED AREA above)
			FadeOutGameCLUT();
			goto do_dept;
		}
		if (gGlobFlag_MeDoneDead)								// see if continue from a kill
			goto again;
		else
			goto do_area;										// or continue from completed area
	}

	SwitchPlayer();												// load "other" player
	FadeOutGameCLUT();
	if (gAreaNum >= 3)											// see if next scene/department
	{
		gAreaNum = 0;
		gSceneNum++;											// (already checked for finish under FINISHED AREA above)
		goto do_dept;
	}
	goto do_area;

					/*****************/
					/*  GAME OVER    */
					/*****************/

game_over:
	FadeOutGameCLUT();
	CleanMemory();												// clean up memory
	InitScreenBuffers();										// restore offscreen buffers
}


/***************** CLEAN MEMORY *******************/
//
// This will purge all allocated memory which is not needed to perform the basic
// operations of the game.
//

void CleanMemory(void)
{
	ZapAllAddedSounds();
	KillSong();
	DisposeCurrentMapData();
	ZapShapeTable(0xff);								// zap all shape tables
	MaxMem(&someLong);									// clean up again
	CompactMem(maxSize);
}


/*************** OPTIMIZE MEMORY ****************/
//
// This is called at the beginning and/or end of each area
// to delete all old data and optimize memory
// for the next area (or SPIN movie)
//

void OptimizeMemory(void)
{
	DisposeCurrentMapData();									// attempt to clean up memory
	StopAllSound();
	ZapAllAddedSounds();
	KillSong();
	ZapShapeTable(GROUP_AREA_SPECIFIC);							// zap before loading new song
	ZapShapeTable(GROUP_AREA_SPECIFIC2);
	ZapShapeTable(GROUP_OVERHEAD);
}

/******************** INIT DEFAULT PREFS **********************/

static void InitDefaultPrefs(void)
{
	memset(&gGamePrefs, 0, sizeof(gGamePrefs));
	snprintf(gGamePrefs.magic, sizeof(gGamePrefs.magic), "%s", PREFS_MAGIC);
	gGamePrefs.interlaceMode = false;
	gGamePrefs.difficulty = DIFFICULTY_NORMAL;
#if _DEBUG
	gGamePrefs.fullscreen = false;
#else
	gGamePrefs.fullscreen = true;
#endif
	gGamePrefs.pfSize = PFSIZE_MEDIUM;
	gGamePrefs.vsync = true;
	gGamePrefs.integerScaling = true;
	gGamePrefs.uncappedFramerate = true;
	gGamePrefs.filterDithering = true;
	gGamePrefs.interpolateAudio = true;
	gGamePrefs.gameTitlePowerPete = false;
	memcpy(gGamePrefs.keys, kDefaultKeyBindings, sizeof(kDefaultKeyBindings));
}

/******************** LOAD PREFS **********************/
//
// Load in standard preferences
//

OSErr LoadPrefs(void)
{
OSErr		iErr;
short		refNum;
FSSpec		file;
long		count;
PrefsType	prefs;
				
				/*************/
				/* READ FILE */
				/*************/
					
	FSMakeFSSpec(gPrefsFolderVRefNum, gPrefsFolderDirID, ":MightyMike:Prefs", &file);
	iErr = FSpOpenDF(&file, fsRdPerm, &refNum);	
	if (iErr)
		return iErr;

	iErr = GetEOF(refNum, &count);
	if (iErr)
	{
		FSClose(refNum);
		return iErr;
	}

	if (count != sizeof(PrefsType))
	{
		// size of file doesn't match size of struct
		FSClose(refNum);
		return badFileFormat;
	}

	count = sizeof(PrefsType);
	iErr = FSRead(refNum, &count,  (Ptr)&prefs);		// read data from file
	FSClose(refNum);
	if (iErr
		|| count < (long)sizeof(PrefsType)
		|| 0 != strncmp(PREFS_MAGIC, prefs.magic, sizeof(prefs.magic)))
	{
		return iErr;
	}
	
	gGamePrefs = prefs;
	
	return noErr;
}


/******************** SAVE PREFS **********************/

void SavePrefs(void)
{
FSSpec				file;
OSErr				iErr;
short				refNum;
long				count;

				/* CREATE BLANK FILE */

	FSMakeFSSpec(gPrefsFolderVRefNum, gPrefsFolderDirID, ":MightyMike:Prefs", &file);
	FSpDelete(&file);															// delete any existing file
	iErr = FSpCreate(&file, 'MMik', 'Pref', smSystemScript);					// create blank file
	if (iErr)
		return;

				/* OPEN FILE */
					
	iErr = FSpOpenDF(&file, fsRdWrPerm, &refNum);
	if (iErr)
	{
		FSpDelete(&file);
		return;
	}

				/* WRITE DATA */

	count = sizeof(PrefsType);
	FSWrite(refNum, &count, (Ptr)&gGamePrefs);
	FSClose(refNum);
}


/************************************************************/
/******************** PROGRAM MAIN ENTRY  *******************/
/************************************************************/

void GameMain(void)
{
	ToolBoxInit();
 	MaxApplZone();
 	MoreMasters();
	VerifySystem();

	InitDefaultPrefs();
	LoadPrefs();
	ApplyPrefs();
	MakeGameWindow();	// now called by ApplyPrefs

	InitInput();                                    // init ISp
	HideCursor();
	InitPaletteStuff();
	CreatePlayfieldPermanentMemory();				// init permanent playfield stuff
	InitObjectManager();							// call this just to allocate memory
	InitSoundTools();
	GetDateTime ((unsigned long *)(&someLong));		// init random seed
	SetMyRandomSeed(someLong);
	LoadHighScores();

#if _DEBUG											// Source port TEMP: in debug mode, boot straight to game
	printf("WARNING: DEBUG MODE: Jumping straight to game\n");
	gSceneNum = 0;	// 0...4
	gAreaNum = 0;	// 0...2
	InitGame();

	gNumWeaponsIHave = 0;
	for (int i = 0; i <= WEAPON_TYPE_ROCKETGUN; i++)
	{
		gMyWeapons[i].type = i;
		gMyWeapons[i].life = 999;
		gNumWeaponsIHave++;
	}

	//Do1PlayerGame();
	InitArea();
	PlayArea();
#endif

	DoLegal();
	DoPangeaLogo();

loop:
	CleanMemory();							// clean up memory

	DoTitleScreen();						// do MAIN MENU / title screen

	if (gDemoMode == DEMO_MODE_OFF)
	{
		ChoosePlayerMode();					// choose 1 or 2 players
		if (gAbortDemoFlag)
			goto loop;
	}
	else
		gPlayerMode = ONE_PLAYER;

	StopMusic();

	WipeScreenBuffers();					// free some of that buffer memory
	InitGame();

	if (gPlayerMode == TWO_PLAYER)
		Do2PlayerGame();
	else
		Do1PlayerGame();

	goto loop;								// go back to main menu
}


