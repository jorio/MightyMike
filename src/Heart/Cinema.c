/****************************/
/*    CINEMA MANAGER        */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/
#include "myglobals.h"
#include "objecttypes.h"
#include "window.h"
#include "cinema.h"
#include "picture.h"
#include "object.h"
#include "misc.h"
#include "sound2.h"
#include "spin.h"
#include "shape.h"
#include "io.h"
#include "miscanims.h"
#include "main.h"
#include "infobar.h"
#include "input.h"
#include "externs.h"
#include "weapon.h"
#include "font.h"

static void DoDifficultyScreen(void);


/****************************/
/*    CONSTANTS             */
/****************************/

#define	TIME_TILL_DEMO	(60L*50L)

#define	CURSOR_X	70
#define CURSOR_Y	404
#define	MAX_NAME_LENGTH		17
#define	MAX_HIGH_SCORES		8

#define	SCORE_HTAB			70
#define	NAME_HTAB			250
#define	TOP_SCORE_VTAB		190
#define	SCORE_LINE_GAP		32

#define	BATTERY_X1			160
#define	BATTERY_X2			430
#define	BATTERY_X3 			(BATTERY_X2+80)
#define	BATTERY_Y			340


#define	OPTIONS_X			CURSOR_X
#define	OPTIONS_Y			410
#define	OPTION_SPACING		100				// must be even!!!

enum
{
	CURSOR_MODE_READY,
	CURSOR_MODE_WALK
};

enum
{
	CURSOR_ANIM_STAND = 4,
	CURSOR_ANIM_WALKLEFT = 2,
	CURSOR_ANIM_WALKRIGHT = 0
};


/**********************/
/*     VARIABLES      */
/**********************/

static	short		gCursorSelection,gCursorMode;

static	int32_t		HighScoreList[MAX_HIGH_SCORES];							// note: keep arrays together for saving and loading
static	char		HighScoreNames[MAX_HIGH_SCORES][MAX_NAME_LENGTH+1];

static	long		gDemoTimeout;


			/* CURSOR X */

static	short	cX[] = {CURSOR_X,CURSOR_X+OPTION_SPACING,CURSOR_X+(OPTION_SPACING*2),
				CURSOR_X+(OPTION_SPACING*3),CURSOR_X+(OPTION_SPACING*4),
				CURSOR_X+(OPTION_SPACING*5)};

static	ObjNode		*icons[10];


/******************** DO TITLE SCREEN ********************/

void DoTitleScreen(void)
{
Boolean		songFlag;
short		i,startSelection;

	gLoadOldGameFlag = false;

	ReadKeyboard();

	startSelection = 0;										// start selection on item #0

	gAbortDemoFlag = false;									// no demo has been aborted yet.
	gGameIsDemoFlag = false;								// assume wont be playing a demo.

	songFlag = false;

re_enter:
	gStartingScene = 0;								// assume not cheating
	gDemoTimeout = TIME_TILL_DEMO;

					/* INITIAL LOADING */

	InitObjectManager();
	LoadShapeTable(":shapes:title.shapes", GROUP_MAIN);
	EraseCLUT();
	EraseBackgroundBuffer();
	LoadBackground(gGamePrefs.gameTitlePowerPete ? ":images:titlepagepp.tga" : ":images:titlepage.tga");
	DumpBackground();


					/* CREATE CURSOR */

	MakeNewShape(GroupNum_MeTitle,ObjType_MeTitle,4,cX[startSelection],CURSOR_Y,
						100,MoveTitleCursor,SCREEN_RELATIVE);

	gCursorSelection = startSelection;						// start cursor on 1st item
	gCursorMode = CURSOR_MODE_READY;

					/* CREATE ICONS */

	for (i=0; i < 6; i++)
		icons[i] = MakeNewShape(GroupNum_TitleIcons,ObjType_TitleIcons,i,OPTIONS_X+(i*OPTION_SPACING),OPTIONS_Y,
						0,nil,SCREEN_RELATIVE);

	SwitchAnim(icons[startSelection],icons[startSelection]->SubType+6);		// flash this icon


						/* LETS DO IT */

	DumpGameWindow();
	DrawObjects();
	DumpUpdateRegions();
	if (!songFlag)
		PlaySong(SONG_ID_TITLE);
	songFlag = true;
	FadeInGameCLUT();

	do
	{
		RegulateSpeed2(1);
		EraseObjects();
		MoveObjects();
		DrawObjects();
		DumpUpdateRegions();

		ReadKeyboard();
		DoSoundMaintenance(true);							// (must be after readkeyboard)

//		if ((--gDemoTimeout < 0) || (GetKeyState(KEY_P)&&GetKeyState(KEY_L)))	// see if do demo
//		{
//			gPlayerMode = ONE_PLAYER;
//			FadeOutGameCLUT();
//			ZapShapeTable(GROUP_MAIN);
//			InitDemoPlayback();
//			return;
//		}

		if (GetSDLKeyState(SDL_SCANCODE_R) && GetSDLKeyState(SDL_SCANCODE_E))	// see if record demo
		{
			gPlayerMode = ONE_PLAYER;
			ZapShapeTable(GROUP_MAIN);
			StartRecordingDemo();
			return;
		}

					/* CHECK FOR SCENE CHEATS */

		if (SDL_GetMouseState(nil, nil))			// mouse button down
		{
			for (i = 1; i <= 5; i++)
			{
				if (GetSDLKeyState(SDL_SCANCODE_1 + i - 1))
				{
					gStartingScene = i - 1;
					gStartingArea = 0;
					InitGame();
					gNumWeaponsIHave = NUM_WEAPON_TYPES;
					for (int j = 0; j < gNumWeaponsIHave; j++)
					{
						gMyWeapons[j].type = j;
						gMyWeapons[j].life = 999;
					}
					Do1PlayerGame();
					KillSong();
					songFlag = false;
					goto re_enter;
				}
			}
		}

		if (gCursorMode != CURSOR_MODE_READY)
			continue;

		if (GetNewNeedState(kNeed_UIConfirm))
			break;

	} while(true);

				/* HANDLE SELECTION */

	startSelection = gCursorSelection;				// remember where to restart cursor

	switch(gCursorSelection)
	{
		case	0:									// PLAY
				break;

		case	1:									// SETTINGS
                DoSettingsScreen();
				goto re_enter;
				break;

		case	2:									// DIFFICULTY
				DoDifficultyScreen();
				goto re_enter;
				break;

		case	3:									// HI-SCORES
				ShowHighScores();
				goto re_enter;
				break;

		case	4:									// CREDITS
				DoCredits();
				PlaySong(SONG_ID_TITLE);
				goto re_enter;
				break;

		case	5:									// QUIT
				CleanQuit();
				break;

	}
	FadeOutGameCLUT();
	ZapShapeTable(GROUP_MAIN);
}


/******************* MOVE TITLE CURSOR *************************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveTitleCursor(void)
{

				/*********************************/
				/* SEE IF WAITING FOR USER INPUT */
				/*********************************/

	if (GetSDLKeyState(SDL_SCANCODE_P))											// see if go directly to PLAY
	{
		if (icons[gCursorSelection]->SubType >= 6)
			SwitchAnim(icons[gCursorSelection],icons[gCursorSelection]->SubType-6);
		gDemoTimeout = TIME_TILL_DEMO;
		gThisNodePtr->BaseX = cX[0];
		gCursorMode = CURSOR_MODE_WALK;
		gCursorSelection = 0;
	}
	else
	if (GetSDLKeyState(SDL_SCANCODE_S))											// see if go directly to SCORES
	{
		if (icons[gCursorSelection]->SubType >= 6)
			SwitchAnim(icons[gCursorSelection],icons[gCursorSelection]->SubType-6);
		gDemoTimeout = TIME_TILL_DEMO;
		gThisNodePtr->BaseX = cX[3];
		gCursorMode = CURSOR_MODE_WALK;
		gCursorSelection = 3;
	}
	else
	if (GetSDLKeyState(SDL_SCANCODE_D))											// see if go directly to DIFFICULTY
	{
		if (icons[gCursorSelection]->SubType >= 6)
			SwitchAnim(icons[gCursorSelection],icons[gCursorSelection]->SubType-6);
		gDemoTimeout = TIME_TILL_DEMO;
		gThisNodePtr->BaseX = cX[2];
		gCursorMode = CURSOR_MODE_WALK;
		gCursorSelection = 2;
	}
	else
	if (GetSDLKeyState(SDL_SCANCODE_C))											// see if go directly to CREDITS
	{
		if (icons[gCursorSelection]->SubType >= 6)
			SwitchAnim(icons[gCursorSelection],icons[gCursorSelection]->SubType-6);
		gDemoTimeout = TIME_TILL_DEMO;
		gThisNodePtr->BaseX = cX[4];
		gCursorMode = CURSOR_MODE_WALK;
		gCursorSelection = 4;
	}
	else
	if (GetSDLKeyState(SDL_SCANCODE_V))											// see if go directly to VIEW
	{
		if (icons[gCursorSelection]->SubType >= 6)
			SwitchAnim(icons[gCursorSelection],icons[gCursorSelection]->SubType-6);
		gDemoTimeout = TIME_TILL_DEMO;
		gThisNodePtr->BaseX = cX[1];
		gCursorMode = CURSOR_MODE_WALK;
		gCursorSelection = 1;
	}
	else
	if (GetSDLKeyState(SDL_SCANCODE_Q))											// see if go directly to QUIT
	{
		if (icons[gCursorSelection]->SubType >= 6)
			SwitchAnim(icons[gCursorSelection],icons[gCursorSelection]->SubType-6);
		gDemoTimeout = TIME_TILL_DEMO;
		gThisNodePtr->BaseX = cX[5];
		gCursorMode = CURSOR_MODE_WALK;
		gCursorSelection = 5;
	}


	if (gCursorMode == CURSOR_MODE_READY)
	{
		if (GetNeedState(kNeed_UILeft) || GetNewNeedState(kNeed_UIPrev))		// see if left key
		{
			gDemoTimeout = TIME_TILL_DEMO;
			if (gCursorSelection > 0)
			{
				if (icons[gCursorSelection]->SubType >= 6)
					SwitchAnim(icons[gCursorSelection],icons[gCursorSelection]->SubType-6);	// stop flash this icon
				gCursorSelection--;
				PlaySound(SOUND_SELECTCHIME);
				gThisNodePtr->BaseX = cX[gCursorSelection];						// set target X
				gCursorMode = CURSOR_MODE_WALK;
			}
		}

		if (GetNeedState(kNeed_UIRight) || GetNewNeedState(kNeed_UINext))		// see if right key
		{
			gDemoTimeout = TIME_TILL_DEMO;
			if (gCursorSelection < 5)
			{
				if (icons[gCursorSelection]->SubType >= 6)
					SwitchAnim(icons[gCursorSelection],icons[gCursorSelection]->SubType-6);		// stop flash this icon
				gCursorSelection++;
				PlaySound(SOUND_SELECTCHIME);
				gThisNodePtr->BaseX = cX[gCursorSelection];			// set target X
				gCursorMode = CURSOR_MODE_WALK;
			}
		}
	}
	else
				/*********************************/
				/* 		SEE IF WALKING 			 */
				/*********************************/

	if (gCursorMode == CURSOR_MODE_WALK)
	{
		if (gThisNodePtr->X.Int < gThisNodePtr->BaseX)					// see if go right
		{
			gThisNodePtr->X.Int += 2;
			if (gThisNodePtr->SubType != CURSOR_ANIM_WALKRIGHT)
				SwitchAnim(gThisNodePtr,CURSOR_ANIM_WALKRIGHT);
		}
		else
		if (gThisNodePtr->X.Int > gThisNodePtr->BaseX)					// see if go left
		{
			gThisNodePtr->X.Int -= 2;
			if (gThisNodePtr->SubType != CURSOR_ANIM_WALKLEFT)
				SwitchAnim(gThisNodePtr,CURSOR_ANIM_WALKLEFT);
		}
		else
		{
			gCursorMode = CURSOR_MODE_READY;						// its gotten there
			SwitchAnim(gThisNodePtr,CURSOR_ANIM_STAND);
			if (icons[gCursorSelection]->SubType < 6)
				SwitchAnim(icons[gCursorSelection],icons[gCursorSelection]->SubType+6);		// flash this icon
		}
	}
}



/******************** DO SCENE SCREEN ********************/

void DoSceneScreen(void)
{
	if (gDemoMode != DEMO_MODE_OFF)								// dont show in demo mode
		return;

	InitObjectManager();
	FadeOutGameCLUT();											// fade out old screen
	BlankEntireScreenArea();

					/* INITIAL LOADING */

	PlaySong(SONG_ID_WORLD_INTRO);

	const char* imagePath = nil;
	switch (gSceneNum)
	{
		case SCENE_JURASSIC:		imagePath = ":images:dinoscene.tga";		break;
		case SCENE_CANDY:			imagePath = ":images:candyscene.tga";		break;
		case SCENE_FAIRY:			imagePath = ":images:fairyscene.tga";		break;
		case SCENE_CLOWN:			imagePath = ":images:clownscene.tga";		break;
		case SCENE_BARGAIN:			imagePath = ":images:bargainscene.tga";		break;
		default:
			GAME_ASSERT_MESSAGE(false, "Unsupported scene ID!");
	}

	LoadImage(imagePath, LOADIMAGE_FADEIN);

	WaitWhileMusic();
	StopMusic();

}


/***************** CLEAR HIGH SCORES ******************/

void ClearHighScores(void)
{
	for (int i = 0; i < MAX_HIGH_SCORES; i++)
	{
		HighScoreNames[i][0] = '\0';

		HighScoreList[i] = 0;							// init score
	}
}


/********************** SHOW HIGH SCORES *********************/

void ShowHighScores(void)
{
	FadeOutGameCLUT();
loop:
	DisplayScores();									// show them
	PresentIndexedFramebuffer();

	UpdateInput();										// eat key press

	while (!UserWantsOut())								// wait for button
	{
		RegulateSpeed2(1);
		ReadKeyboard();
		if (GetSDLKeyState(SDL_SCANCODE_LCTRL) && GetSDLKeyState(SDL_SCANCODE_C))	// see if key clear
		{
			ClearHighScores();
			SaveHighScores();
			goto loop;
		}
		DoSoundMaintenance(false);
		PresentIndexedFramebuffer();
	}
	FadeOutGameCLUT();
}

/********************* DISPLAY SCORES ********************/

void	DisplayScores(void)
{
short		i;

	EraseCLUT();
	LoadShapeTable(":shapes:highscore.shapes", GROUP_WIN);
	LoadBackground(":images:scores.tga");
	DumpBackground();							// dump to playfield
	DumpGameWindow();									// show the whole thing

	DumpUpdateRegions();

					/* DISPLAY ENTRIES */


	for (i=0; i<MAX_HIGH_SCORES; i++)
	{
		gHtab = NAME_HTAB;								// print name
		gVtab = TOP_SCORE_VTAB+(i*SCORE_LINE_GAP);
		WriteLn(HighScoreNames[i]);

														// print number
		gHtab = SCORE_HTAB;
		gVtab = TOP_SCORE_VTAB+(i*SCORE_LINE_GAP);
		PrintBigNum(HighScoreList[i],7);
	}

	FadeInGameCLUT();
	ZapShapeTable(GROUP_WIN);
}


/********************** ADD HIGH SCORE **********************/
//
// If high score is worthy of adding, it does it
//

void AddHighScore(long	newScore)
{
short		i,z,m;

				/* SEE IF WORTHY */

	for (i=0; i<MAX_HIGH_SCORES; i++)
	{
		if (newScore > HighScoreList[i])
			goto gotit;
	}
	ShowLastScore();										// not a high score, so just show it
	return;

				/* ADD THE SCORE */

gotit:

	if (i != MAX_HIGH_SCORES-1)								// insert if not last, otherwise just tag to end
		for (z = MAX_HIGH_SCORES-1; z > i; z--)
		{
			HighScoreList[z] = HighScoreList[z-1];			// move scores up
			for (m=0; m<=MAX_NAME_LENGTH; m++)				// move names up
				HighScoreNames[z][m] = HighScoreNames[z-1][m];
		}


	HighScoreList[i] = newScore;							// put new score in spot
	HighScoreNames[i][0] = '\0';							// erase old name from spot


	DisplayScores();										// put em up there
	DoEnterName(i);											// ask for a name
	SaveHighScores();
}

/************************ SHOW LAST SCORE ******************/

void ShowLastScore(void)
{
	LoadShapeTable(":shapes:highscore.shapes", GROUP_WIN);
	EraseBackgroundBuffer();

	EraseCLUT();
	LoadBackground(":images:winbw.tga");					// just to load the palette
	DumpBackground();										// dump to playfield
	DumpGameWindow();										// show the whole thing
	DumpUpdateRegions();

					/* DISPLAY SCORE */

	gHtab = 640/2-50;
	gVtab = 480/2-50;
	WriteLn("SCORE");

	gHtab = 250;
	gVtab = 480/2;
	PrintBigNum(gScore,7);

	FadeInGameCLUT();

	Wait(60*10);

	ZapShapeTable(GROUP_WIN);
}


/********************** DO ENTER NAME **************************/

void DoEnterName(short n)
{
char		theChar;
short		i=0;
Rect		eraseRect;
ObjNode		*nameObj;


						/* LOAD LETTERS FOR WRITELN */

	LoadShapeTable(":shapes:highscore.shapes", GROUP_WIN);


						/* MAKE CURSOR SPRITE */

	InitObjectManager();

	gHtab = NAME_HTAB;											// calc coords of name
	gVtab = TOP_SCORE_VTAB+(n*SCORE_LINE_GAP);

	nameObj = MakeNewShape(GroupNum_BigFont,ObjType_BigFont,0,gHtab+OFFSCREEN_WINDOW_LEFT,
					gVtab+OFFSCREEN_WINDOW_TOP,100,nil,SCREEN_RELATIVE);


						/* ENTER NAME */

	do
	{
		RegulateSpeed2(1);

		ReadKeyboard();

		theChar = gTextInput[0];
		EraseObjects();

		if (GetNewSDLKeyState(SDL_SCANCODE_RETURN) || GetNewSDLKeyState(SDL_SCANCODE_KP_ENTER))			// see if done
			goto exit;

		if (gSDLController)
		{
			if (SDL_GameControllerGetButton(gSDLController, SDL_CONTROLLER_BUTTON_START))
				goto exit;
		} 

					/*  SEE IF BACK CHAR */

		if (GetNewSDLKeyState(SDL_SCANCODE_LEFT) || GetNewSDLKeyState(SDL_SCANCODE_BACKSPACE))
		{
			if (i>0)
			{
				i--;
				HighScoreNames[n][i] = '\0';
				gHtab = NAME_HTAB;						// set coords of name
				gVtab = TOP_SCORE_VTAB+(n*SCORE_LINE_GAP);
				eraseRect.left		= gScreenXOffset + gHtab + OFFSCREEN_WINDOW_LEFT - 30;
				eraseRect.top		= gScreenYOffset + gVtab + OFFSCREEN_WINDOW_TOP - 20;
				eraseRect.right		= gScreenXOffset + gHtab + OFFSCREEN_WINDOW_LEFT + 350;
				eraseRect.bottom	= gScreenYOffset + gVtab + OFFSCREEN_WINDOW_TOP + 20;
				AddUpdateRegion(eraseRect,0);				// erase existing name
				DumpUpdateRegions_DontPresentFramebuffer();	// commit erased regions; don't present framebuffer yet to avoid flashing
				WriteLn(HighScoreNames[n]);					// print it (will present framebuffer)
				nameObj->X.Int = (gHtab+OFFSCREEN_WINDOW_LEFT);	// set cursor
				nameObj->Y.Int = (gVtab+OFFSCREEN_WINDOW_TOP);
			}
		}

				/* SEE IF NEW CHAR */

		else
		if (theChar >= ' ' && theChar <= 'z' && i < MAX_NAME_LENGTH)	// see if can enter another char
		{
			if (theChar >= 'a' && theChar <= 'z')
				theChar -= 0x20;					// convert to upper case

			HighScoreNames[n][i] = theChar;			// put char into string
			HighScoreNames[n][i+1] = '\0';
			i++;
			gHtab = NAME_HTAB;						// set coords of name
			gVtab = TOP_SCORE_VTAB+(n*SCORE_LINE_GAP);
			eraseRect.left		= gScreenXOffset + gHtab + OFFSCREEN_WINDOW_LEFT - 30;
			eraseRect.top		= gScreenYOffset + gVtab + OFFSCREEN_WINDOW_TOP - 20;
			eraseRect.right		= OFFSCREEN_WINDOW_RIGHT;
			eraseRect.bottom	= gScreenYOffset + gVtab+OFFSCREEN_WINDOW_TOP + 20;

			AddUpdateRegion(eraseRect,0);				// erase area of cursor
			DumpUpdateRegions_DontPresentFramebuffer();	// commit erased regions; don't present framebuffer yet to avoid flashing

			WriteLn(HighScoreNames[n]);					// print whole word (will present framebuffer)
			nameObj->X.Int = (gHtab+OFFSCREEN_WINDOW_LEFT);	// set cursor
			nameObj->Y.Int = (gVtab+OFFSCREEN_WINDOW_TOP);
			nameObj->drawBox.left	= nameObj->X.Int + gScreenXOffset;
			nameObj->drawBox.right	= nameObj->X.Int + gScreenXOffset;
			nameObj->drawBox.top	= nameObj->Y.Int + gScreenYOffset;
			nameObj->drawBox.bottom	= nameObj->Y.Int + gScreenYOffset;
		}

		MoveObjects();
		DrawObjects();
		DumpUpdateRegions();

	} while (true);

exit:;

	ZapShapeTable(GROUP_WIN);
}


/******************** SAVE HIGH SCORES **********************/

void SaveHighScores(void)
{
OSErr		iErr;
FSSpec		spec;
short		refNum;
long		count;

	FSMakeFSSpec(gPrefsFolderVRefNum, gPrefsFolderDirID, ":MightyMike:HighScores", &spec);
	FSpDelete(&spec);															// delete any existing file

	iErr = FSpCreate(&spec, 'MMik', 'sKor', smSystemScript);					// create blank file
	GAME_ASSERT(iErr == noErr);

	iErr = FSpOpenDF(&spec, fsWrPerm, &refNum);
	GAME_ASSERT(iErr == noErr);

	count = sizeof(int32_t) * MAX_HIGH_SCORES;
	iErr = FSWrite(refNum, &count, (Ptr) HighScoreList);
	GAME_ASSERT(iErr == noErr);
	GAME_ASSERT(count == sizeof(HighScoreList));

	count = MAX_HIGH_SCORES*(MAX_NAME_LENGTH+1);
	iErr = FSWrite(refNum, &count, (Ptr) HighScoreNames);
	GAME_ASSERT(iErr == noErr);
	GAME_ASSERT(count == sizeof(HighScoreNames));

	iErr = FSClose(refNum);
	GAME_ASSERT(iErr == noErr);
}


/******************** LOAD HIGH SCORES **********************/

void LoadHighScores(void)
{
OSErr				iErr;
short				refNum;
FSSpec				file;
long				count;
bool				needClose = false;

	FSMakeFSSpec(gPrefsFolderVRefNum, gPrefsFolderDirID, ":MightyMike:HighScores", &file);
	iErr = FSpOpenDF(&file, fsRdPerm, &refNum);

	if (iErr == fnfErr)
		goto corrupt;
	else if (iErr)
		DoFatalAlert("LoadHighScores: Error opening High Scores file!");
	else
	{
		needClose = true;

		count = sizeof(int32_t) * MAX_HIGH_SCORES;
		iErr = FSRead(refNum, &count, (Ptr) HighScoreList);
		if (iErr != noErr || count != sizeof(HighScoreList))
			goto corrupt;

		count = MAX_HIGH_SCORES*(MAX_NAME_LENGTH+1);
		iErr = FSRead(refNum, &count, (Ptr) HighScoreNames);
		if (iErr != noErr || count != sizeof(HighScoreNames))
			goto corrupt;

		FSClose(refNum);
		return;
	}

corrupt:
	ClearHighScores();
	if (needClose)
		FSClose(refNum);
}


/******************** DO PANGEA LOGO ********************/

void DoPangeaLogo(void)
{
	EraseCLUT();
	PreLoadSpinFile(":movies:pangea.spin",DEFAULT_SPIN_PRELOAD_SIZE);	// load the file
	PlaySong(SONG_ID_PANGEA);

	PlaySpinFile(60*5);
	FadeOutGameCLUT();
	StopMusic();
}

/******************** DO LEGAL ********************/

void DoLegal(void)
{
	EraseCLUT();
	BlankEntireScreenArea();
	LoadImage(":images:legal.tga", LOADIMAGE_FADEIN);
	Wait2(60*6);
	FadeOutGameCLUT();
}

/****************** CHOOSE PLAYER MODE ************************/

void ChoosePlayerMode(void)
{
ObjNode		*batteryObj;
ObjNode		*batteryObj2,*cursorObj;
long		sgcursorx[] = {170,245,320,395,470};
long		restoreMode;

					/* INITIAL LOADING */

	InitObjectManager();
	LoadShapeTable(":shapes:playerchoose.shapes", GROUP_OVERHEAD);
	EraseCLUT();
	EraseBackgroundBuffer();
	LoadBackground(":images:playerchoose.tga");
	DumpBackground();										// dump to playfield


					/* CREATE CURSORS */

	batteryObj = MakeNewShape(GroupNum_PCBattery,ObjType_PCBattery,0,BATTERY_X1,BATTERY_Y,
						100,nil,SCREEN_RELATIVE);
	if (batteryObj == nil)
		DoFatalAlert("Err: Alloc Battery Cursor");

	batteryObj2 = MakeNewShape(GroupNum_PCBattery,ObjType_PCBattery,0,BATTERY_X3,BATTERY_Y,
						100,nil,SCREEN_RELATIVE);
	if (batteryObj2 == nil)
		DoFatalAlert("Err: Alloc Battery Cursor #2");
	batteryObj2->DrawFlag = false;

	gCursorSelection = 0;								// start cursor on 1st item

						/* LETS DO IT */

	DumpGameWindow();
	DrawObjects();
	DumpUpdateRegions();
	FadeInGameCLUT();

	do
	{
		RegulateSpeed2(1);
		EraseObjects();
		MoveObjects();
		DrawObjects();
		DumpUpdateRegions();

		ReadKeyboard();

		if (gCursorSelection && (GetNeedState(kNeed_UILeft) || GetNewNeedState(kNeed_UIPrev)))
		{
			DeactivateObjectDraw(batteryObj2);
			batteryObj->X.Int = BATTERY_X1;
			gCursorSelection = 0;
			PlaySound(SOUND_SELECTCHIME);
		}
		else
		if (!gCursorSelection && (GetNeedState(kNeed_UIRight) || GetNewNeedState(kNeed_UINext)))
		{
			batteryObj2->DrawFlag = true;
			batteryObj->X.Int = BATTERY_X2;
			gCursorSelection = 1;
			PlaySound(SOUND_SELECTCHIME);
		}

		DoSoundMaintenance(true);							// (must be after readkeyboard)

		if (GetNewNeedState(kNeed_UIBack))					// see if abort (use to demo flag to trick it)
		{
			gAbortDemoFlag = true;
			return;
		}

	} while(!GetNewNeedState(kNeed_UIConfirm) && !gAbortDemoFlag);


				/* HANDLE SELECTION */

	switch(gCursorSelection)
	{
		case	0:									// 1 PLAYER
				gPlayerMode = ONE_PLAYER;
				break;

		case	1:									// 2 PLAYER
				gPlayerMode = TWO_PLAYER;
				break;
	}


	batteryObj2->AnimFlag = false;
	batteryObj->AnimFlag = false;

			/*************************/
			/* DO SAVED GAME RESTORE */
			/*************************/

	restoreMode = 0;
	MakeNewShape(GroupNum_SaveGame,ObjType_SaveGame,0,320,450,100,nil,SCREEN_RELATIVE);	// put game restore dialog icon
	cursorObj = MakeNewShape(GroupNum_SaveGame,ObjType_SaveGame,1,
							sgcursorx[restoreMode],450,50,nil,SCREEN_RELATIVE);	// make cursor
	gLoadOldGameFlag = false;

	do
	{
		RegulateSpeed2(1);
		EraseObjects();
		MoveObjects();
		DrawObjects();
		DumpUpdateRegions();

		ReadKeyboard();

		if ((GetNewNeedState(kNeed_UILeft) || GetNewNeedState(kNeed_UIPrev)) && (restoreMode > 0))
		{
			restoreMode--;
			cursorObj->X.Int = sgcursorx[restoreMode];
			PlaySound(SOUND_SELECTCHIME);
			SwitchAnim(cursorObj,restoreMode+1);
		}
		else
		if ((GetNewNeedState(kNeed_UIRight) || GetNewNeedState(kNeed_UINext)) && (restoreMode < 4))
		{
			restoreMode++;
			cursorObj->X.Int = sgcursorx[restoreMode];
			PlaySound(SOUND_SELECTCHIME);
			SwitchAnim(cursorObj,restoreMode+1);
		}

		DoSoundMaintenance(true);							// (must be after readkeyboard)

		if (GetNewNeedState(kNeed_UIBack))					// see if abort (use to demo flag to trick it)
		{
			gAbortDemoFlag = true;
			return;
		}

			/* SEE IF HIT KEYS TO SELECT GAME RESTORE */

		if (GetNewSDLKeyState(SDL_SCANCODE_1))
		{
			restoreMode = 1;
			break;
		}
		if (GetNewSDLKeyState(SDL_SCANCODE_2))
		{
			restoreMode = 2;
			break;
		}
		if (GetNewSDLKeyState(SDL_SCANCODE_3))
		{
			restoreMode = 3;
			break;
		}
		if (GetNewSDLKeyState(SDL_SCANCODE_4))
		{
			restoreMode = 4;
			break;
		}

	} while(!GetNewNeedState(kNeed_UIConfirm) && !gAbortDemoFlag);

	
				/* SEE IF RESTORE GAME */

	if (restoreMode > 0)
	{
		gLoadOldGameFlag = true;
		gLoadOldGameNum = restoreMode;
	}


	ZapShapeTable(GROUP_OVERHEAD);
}

#pragma mark -

/*********************** MOVE CREDIT LETTER ****************************/
//
// Special1: base scroll speed
// Special2: random upwards acceleration
// Special3: Y coordinate above which letters disappear
//

static void MoveScrollingTextLetter(void)
{
	if (gThisNodePtr->Y.Int > 250)
	{
		gThisNodePtr->Y.L -= gThisNodePtr->Special1;			// scroll up
	}
	else
	{
		gThisNodePtr->Y.L += gThisNodePtr->DY;
		gThisNodePtr->DY -= gThisNodePtr->Special2;				// accelerate upwards
	}

	if (gThisNodePtr->Y.Int < gThisNodePtr->Special3)
	{
		DeleteObject(gThisNodePtr);								// vanish
	}
}

/******************** LAY OUT LETTERS FOR SCROLLING TEXT ********************/
//
// Returns next position in text buffer
//

static int LayOutScrollingTextLine(
		const char* const text,
		int textLength,
		int textStartOfLine,
		int letterBaseScrollSpeed,
		int letterVanishAtY)
{
	int lineLength = 0;
	int textPosition = textStartOfLine;

	while (textPosition < textLength)			// find out length of line
	{
		char c = text[textPosition++];

		if (c == '\r' && textPosition < textLength && text[textPosition] == '\n') // windows CRLF
		{
			textPosition++;						// skip LF
			break;
		}
		
		if (c == '\n' || c == '~')
			break;

		lineLength++;
	}

	int x = 320-(FONT_WIDTH*lineLength/2);		// calc starting x

	for (int i = 0; i < lineLength; i++)		// create letter objects
	{
		char c = text[textStartOfLine + i];

		if (c != ' ')				// skip spaces
		{
			ObjNode* theNode = MakeNewShape(GroupNum_BigFont, ObjType_BigFont, ASCIIToBigFont(c),
											x, 470, 100, MoveScrollingTextLetter, SCREEN_RELATIVE);
			theNode->ClipNum = 3;
			theNode->Special1 = letterBaseScrollSpeed;
			theNode->Special2 = (MyRandomLong() & 0b111111111) + 0x500;
			theNode->Special3 = letterVanishAtY;
			theNode->DY = -0x10000L;
		}

		x += FONT_WIDTH;
	}

	return textPosition;
}

/******************** GENERIC SCROLLING TEXT SCREEN ********************/

static void DoScrollingTextScreen(
		const char* backgroundFilePath,
		const char* textFilePath,
		int letterBaseScrollSpeed,
		int letterVanishAtY)
{
Handle	textRez = nil;
int		textPos;
int		textLength;
short	lineCount;

				/* INITIAL LOADING */

	InitObjectManager();
	LoadShapeTable(":shapes:highscore.shapes", GROUP_WIN);

	EraseBackgroundBuffer();
	LoadBackground(backgroundFilePath);
	DumpBackground();											// dump to playfield

				/* SET CLIPPING ZONE */

	gRegionClipTop[3]		= OFFSCREEN_WINDOW_TOP+letterVanishAtY;
	gRegionClipBottom[3] 	= OFFSCREEN_WINDOW_TOP+VISIBLE_HEIGHT-20;
	gRegionClipLeft[3]		= OFFSCREEN_WINDOW_LEFT;
	gRegionClipRight[3]		= OFFSCREEN_WINDOW_LEFT+VISIBLE_WIDTH-1;

				/* LET'S DO IT */

	textRez = LoadRawFile(textFilePath);						// load credits file
	GAME_ASSERT(textRez);

	textLength = GetHandleSize(textRez);
	textPos = 0;
	lineCount = 100;

	DumpGameWindow();
	DrawObjects();
	DumpUpdateRegions();
	FadeInGameCLUT();

	do
	{
		RegulateSpeed2(1);									// @ 60fps
		EraseObjects();
		MoveObjects();

					/* SEE IF MAKE NEXT TEXT LINE */

		if (textPos < textLength && ++lineCount > 50)		// see if ready for next line
		{
			lineCount = 0;
			textPos = LayOutScrollingTextLine(*textRez, textLength, textPos, letterBaseScrollSpeed, letterVanishAtY);
		}

		DrawObjects();
		DumpUpdateRegions();

		ReadKeyboard();
		DoSoundMaintenance(true);							// (must be after readkeyboard)

	} while (!UserWantsOut() && NumObjects > 0);

				/* CLEAN UP */

	DisposeHandle(textRez);							// zap the text rez
	ZapShapeTable(GROUP_WIN);

	FadeOutGameCLUT();
}

/******************** DO CREDITS SCREEN ********************/

void DoCredits(void)
{
	FadeOutGameCLUT();
	PlaySong(SONG_ID_RACE);
	DoScrollingTextScreen(":images:credits1.tga", ":system:credits.txt", 0x8000L, 95);
}


#pragma mark -

/******************** DO OVERHEAD MAP ********************/
//
// OUTPUT: true if aborted screen from demo playback
//

void DoOverheadMap(void)
{
ObjNode		*mikeObj;
static const Point	toCoord[MAX_SCENES][3] =					// coords to move Mike TO (y,x)
{
	{ {419,537}, {351,463}, {246,518} },						// jurassic
	{ {437,100}, {332,214}, {345, 62} },						// candy
	{ {176,203}, {181,281}, {167,383} },						// fairy
	{ {144,438}, { 94,512}, { 93,375} },						// clown
	{ {115,176}, { 81, 47}, { 39,216} },						// bargain
};

static const Point	compCoord[MAX_SCENES] =						// coords to put COMPLETED icon
{
	{298,518},
	{353,168},
	{181,272},
	{ 95,469},
	{ 68,116},
};

short	destX,destY,fromX,fromY;
Boolean	flag;
short	counter,i;

	if (gDemoMode != DEMO_MODE_OFF)								// dont show in demo mode
		return;

	FadeOutGameCLUT();											// fade out old screen
	BlankEntireScreenArea();
	OptimizeMemory();											// free up some memory so we can allocate draw buffers for animation


					/* PLAY INTRO MELODY */

	switch(gSceneNum)
	{
		case	SCENE_JURASSIC:		PlaySong(SONG_ID_JURASSIC_INTRO);		break;
		case	SCENE_CANDY:		PlaySong(SONG_ID_CANDY_INTRO);			break;
		case	SCENE_FAIRY:		PlaySong(SONG_ID_FAIRY_INTRO);			break;
		case	SCENE_CLOWN:		PlaySong(SONG_ID_CLOWN_INTRO);			break;
		case	SCENE_BARGAIN:		PlaySong(SONG_ID_BARGAIN_INTRO);		break;
	}

					/* INITIAL LOADING */

	InitObjectManager();
	LoadShapeTable(":shapes:overheadmap.shapes", GROUP_OVERHEAD);
	EraseCLUT();
	EraseBackgroundBuffer();

	if (gDifficultySetting == DIFFICULTY_EASY)
		LoadBackground(":images:overheadmap2.tga");				// clown and bargain scenes grayed out
	else
		LoadBackground(":images:overheadmap.tga");				// all scenes in full color

	DumpBackground();

					/* GET FROM / TO COORDS */

	destX = toCoord[gSceneNum][gAreaNum].h+OFFSCREEN_BORDER_WIDTHX;
	destY = toCoord[gSceneNum][gAreaNum].v+OFFSCREEN_BORDER_WIDTHY;

	if ((gSceneNum == 0) && (gAreaNum == 0))									// special case start
	{
		fromX = destX;
		fromY = destY;
	}
	else
	{
		if (gAreaNum == 0)
		{
			fromX = toCoord[gSceneNum-1][2].h+OFFSCREEN_BORDER_WIDTHX;
			fromY = toCoord[gSceneNum-1][2].v+OFFSCREEN_BORDER_WIDTHY;
		}
		else
		{
			fromX = toCoord[gSceneNum][gAreaNum-1].h+OFFSCREEN_BORDER_WIDTHX;
			fromY = toCoord[gSceneNum][gAreaNum-1].v+OFFSCREEN_BORDER_WIDTHY;
		}
	}


					/* CREATE MIKE */

	mikeObj = MakeNewShape(GroupNum_OverheadIcons,ObjType_OverheadIcons,1,fromX,fromY,
						100,nil,SCREEN_RELATIVE);


					/* CREATE COMPLETED'S & X'S */

	for (i=0; i < gSceneNum; i++)
		MakeNewShape(GroupNum_OverheadIcons,ObjType_OverheadIcons,2,compCoord[i].h,compCoord[i].v,
				200,nil,SCREEN_RELATIVE);

	for (i=0; i < gAreaNum; i++)
			MakeNewShape(GroupNum_OverheadIcons,ObjType_OverheadIcons,3,toCoord[gSceneNum][i].h,toCoord[gSceneNum][i].v,
						200,nil,SCREEN_RELATIVE);

						/* PLAYER SIGNAL */

//	PutPlayerSignal(false);										// put p1 or p2 message


						/* LETS DO IT */

	DumpGameWindow();
	DrawObjects();
	DumpUpdateRegions();
	FadeInGameCLUT();

					/* SHOW MIKE @ CURRENT SPOT */

	Wait2(50);

					/* MOVE MIKE TO DESTINATION */

	counter = 0;
	do
	{
		flag = false;
		RegulateSpeed2(1);
		EraseObjects();
		MoveObjects();

		if (mikeObj->X.Int != destX)				// see if move Mike X
		{
			if (mikeObj->X.Int < destX)
				mikeObj->X.L += 0x10000L;
			else
				mikeObj->X.L -= 0x10000L;
			flag = true;
		}
		if (mikeObj->Y.Int != destY)				// see if move Mike Y
		{
			if (mikeObj->Y.Int < destY)
				mikeObj->Y.L += 0x10000L;
			else
				mikeObj->Y.L -= 0x10000L;
			flag = true;
		}

		if (++counter > 10)							// see if drop a dot
		{
			counter = 0;
			MakeNewShape(GroupNum_OverheadIcons,ObjType_OverheadIcons,4,mikeObj->X.Int,
					mikeObj->Y.Int,200,nil,SCREEN_RELATIVE);
		}

		DrawObjects();
		DumpUpdateRegions();

		ReadKeyboard();
		DoSoundMaintenance(true);						// (must be after readkeyboard)

	} while(flag && (!gAbortDemoFlag));


					/* SHOW MIKE @ REST */

	SwitchAnim(mikeObj,0);							// stop mike blinking
	WaitWhileMusic();

	StopMusic();
	ZapShapeTable(GROUP_OVERHEAD);
	OptimizeMemory();
}


/******************** DO LOSE SCREEN ********************/

void DoLoseScreen(void)
{
	InitObjectManager();					// (needed for WaitWhileMusic)

	FadeOutGameCLUT();											// fade out old screen
	BlankEntireScreenArea();

					/* INITIAL LOADING */

	PlaySong(SONG_ID_LOSEGAME);
	LoadImage(":images:lose.tga", LOADIMAGE_FADEIN);
	WaitWhileMusic();
	StopMusic();
}


/******************** DO WIN SCREEN ********************/

void DoWinScreen(void)
{
	bool easyWin = gDifficultySetting == DIFFICULTY_EASY;


					/* WIN TEXT SCREEN */

	FadeOutGameCLUT();

	PlaySong(easyWin ? SONG_ID_WINGAMELOOP : SONG_ID_WINHUM);

	DoScrollingTextScreen(
			":images:winbw.tga",
			easyWin ? ":system:win1.txt" : ":system:win3.txt",		// note: win2 is unused
			0xb000L,
			30
	);


					/* SKIP CONFETTI SCREEN IF PLAYING ON EASY */

	if (easyWin)
		goto more;


					/* INITIAL LOADING */
	FadeOutGameCLUT();
	InitObjectManager();



	PlaySong(SONG_ID_WINGAME);
	LoadShapeTable(":shapes:win.shapes", GROUP_WIN);
	EraseBackgroundBuffer();
	LoadBackground(":images:win.tga");
	DumpBackground();

						/* LETS DO IT */

	DumpGameWindow();
	DrawObjects();
	DumpUpdateRegions();
	FadeInGameCLUT();

	do
	{
		RegulateSpeed2(1);									// @ 60fps
		EraseObjects();
		MoveObjects();

							/* ADD CONFETTI */

		ObjNode* newObj = MakeNewShape(GroupNum_Confetti,ObjType_Confetti,RandomRange(0,9),
								RandomRange(40,600),10,100,MoveConfetti,SCREEN_RELATIVE);
		if (newObj != nil)
		{
			newObj->DY = 0x7000L;
			newObj->DX = (long)RandomRange(0,0x8000)-0x4000;
			newObj->Special1 = RandomRange(400,470);
		}

		DrawObjects();
		DumpUpdateRegions();

		ReadKeyboard();
		DoSoundMaintenance(true);							// (must be after readkeyboard)

	} while (!UserWantsOut() && IsMusicPlaying());

	FadeOutGameCLUT();

more:
	StopMusic();

				/* SHOW CREDITS WHEN DONE */

	DoCredits();
	StopMusic();
	ZapShapeTable(GROUP_WIN);
}

/********************** MOVE CONFETTI ******************************/

void MoveConfetti(void)
{
	GetObjectInfo();

	gDY += 0x300L;
	gY.L += gDY;
	gX.L += gDX;

	if ((gY.Int > gThisNodePtr->Special1) || (gX.Int < 10) || (gX.Int > 640))
	{
		DeleteObject(gThisNodePtr);
		return;
	}

	UpdateObject();
}


#pragma mark -



/******************** DO DIFFICULTY SCREEN ********************/

static void DoDifficultyScreen(void)
{
ObjNode		*newObj,*headObj;
short		mode,tick = 0;
static	short	xCoords[] = {105,319,540};

					/* CHECK FOR SECRET HEAD SCREEN */

	ReadKeyboard();
#if __APPLE__
	if (GetSDLKeyState(SDL_SCANCODE_LGUI) && GetSDLKeyState(SDL_SCANCODE_LALT))
#else
	if (GetSDLKeyState(SDL_SCANCODE_LCTRL) && GetSDLKeyState(SDL_SCANCODE_LALT))
#endif
	{
		DoHeadScreen();
		return;
	}


					/* INITIAL LOADING */

	FadeOutGameCLUT();
	InitObjectManager();
	EraseBackgroundBuffer();
	LoadShapeTable(":shapes:difficulty.shapes", GROUP_MAIN);
	LoadBackground(":images:diff.tga");
	DumpBackground();

	mode = gGamePrefs.difficulty;


					/* CREATE CURSOR */

	newObj = MakeNewShape(GroupNum_DifficultyCursor,ObjType_DifficultyCursor,mode,xCoords[mode],430,
						100,nil,SCREEN_RELATIVE);


					/* CREATE HEAD */

	headObj = MakeNewShape(GroupNum_DifficultyHead,ObjType_DifficultyHead,mode+4,320,200,
						100,nil,SCREEN_RELATIVE);


						/* LETS DO IT */

	DumpGameWindow();
	DrawObjects();
	DumpUpdateRegions();
	FadeInGameCLUT();

	do
	{
		RegulateSpeed2(1);
		EraseObjects();
		MoveObjects();

		if (++tick > 5)						// make it flash
		{
			tick =0;
			if (newObj->DrawFlag)
				DeactivateObjectDraw(newObj);
			else
				newObj->DrawFlag = true;
		}

		DrawObjects();
		DumpUpdateRegions();

		ReadKeyboard();
		DoSoundMaintenance(true);							// (must be after readkeyboard)

		if (GetNewNeedState(kNeed_UIRight) || GetNewNeedState(kNeed_UINext))	// see if go right
		{
			if (mode < 2)
			{
				mode++;
				PlaySound(SOUND_SELECTCHIME);
				newObj->X.Int = xCoords[mode];
				SwitchAnim(newObj,mode);
				if (mode == 2)
					SwitchAnim(headObj,1);
				else
					SwitchAnim(headObj,0);
			}
		}
		else
		if (GetNewNeedState(kNeed_UILeft) || GetNewNeedState(kNeed_UIPrev))		// see if left
		{
			if (mode > 0)
			{
				mode--;
				PlaySound(SOUND_SELECTCHIME);
				newObj->X.Int = xCoords[mode];
				SwitchAnim(newObj,mode);
				if (mode == 1)
					SwitchAnim(headObj,2);
				else
					SwitchAnim(headObj,3);
			}
		}

	} while (!UserWantsOut());


				/* HANDLE SELECTION */

	gGamePrefs.difficulty = mode;

	FadeOutGameCLUT();
	ZapShapeTable(GROUP_MAIN);
	SavePrefs();
}


/************************ DO HEAD SCREEN *****************************/

void DoHeadScreen(void)
{
short	i;
ObjNode	*newObj;

					/* INITIAL LOADING */

	FadeOutGameCLUT();
	InitObjectManager();
	EraseCLUT();
	EraseBackgroundBuffer();
	LoadShapeTable(":shapes:view.shapes", GROUP_MAIN);
	LoadBackground(":images:head.tga");
	DumpBackground();



	for (i=0; i < 8; i++)
	{
		newObj = MakeNewShape(GroupNum_Head,ObjType_Head,0,RandomRange(80,600),RandomRange(80,400),
						100,MoveHead,SCREEN_RELATIVE);

		newObj->DX = RandomRange(0x1L,0xfL)*0x8000L;
		newObj->DY = RandomRange(0x1L,0xfL)*0x8000L;
	}

						/* LETS DO IT */

	DumpGameWindow();
	DrawObjects();
	DumpUpdateRegions();
	FadeInGameCLUT();

	do
	{
		RegulateSpeed2(1);
		EraseObjects();
		MoveObjects();
		DrawObjects();
		DumpUpdateRegions();

		ReadKeyboard();
		DoSoundMaintenance(true);							// (must be after readkeyboard)

	} while(!UserWantsOut());

	FadeOutGameCLUT();
	ZapShapeTable(GROUP_MAIN);
}


/******************* MOVE HEAD ********************/

void MoveHead(void)
{
	gThisNodePtr->X.L += gThisNodePtr->DX;
	gThisNodePtr->Y.L += gThisNodePtr->DY;

	if (gThisNodePtr->X.Int < 80)
	{
		gThisNodePtr->X.Int = 80;
		gThisNodePtr->DX = -gThisNodePtr->DX;
	}
	else
	if (gThisNodePtr->X.Int > 570)
	{
		gThisNodePtr->X.Int = 570;
		gThisNodePtr->DX = -gThisNodePtr->DX;
	}

	if (gThisNodePtr->Y.Int < 80)
	{
		gThisNodePtr->Y.Int = 80;
		gThisNodePtr->DY = -gThisNodePtr->DY;
	}
	else
	if (gThisNodePtr->Y.Int >400)
	{
		gThisNodePtr->Y.Int = 400;
		gThisNodePtr->DY = -gThisNodePtr->DY;
	}

}

/***************** SHOW BONUS SCREEN ********************/

void ShowBonusScreen(void)
{
short	i,htab,vtab;
long	bonus;
short	selection;

	ZapShapeTable(GROUP_AREA_SPECIFIC);							// minor cleanup first
	ZapShapeTable(GROUP_AREA_SPECIFIC2);

	InitObjectManager();
	LoadShapeTable(":shapes:bonus.shapes", GROUP_BONUS);
	EraseCLUT();

	BlankEntireScreenArea();
	LoadImage(":images:bonus.tga", LOADIMAGE_FADEIN);

	if (gGamePrefs.music)
	{
		WaitWhileMusic();									// wait for music to stop
		KillSong();
	}

				/* SHOW BUNNY BONUS */

	bonus = 0;
	for (i=0; i < gBunnyCounts[gSceneNum][gAreaNum]; i++)
	{
		htab = 80;
		vtab = 130;
		PlaySound(SOUND_SELECTCHIME);
		DrawFrameToScreen((i*37)+htab,vtab,GROUP_BONUS,0,1);	// draw bunny
		bonus += 600;
		PrintBonusNum(bonus, 5, 540, vtab);						// draw bonus #
		PresentIndexedFramebuffer();
		Wait4(15);
	}
	GetPoints(bonus);											// add bonus to score
	Wait4(60*1);


				/* SHOW JAWBREAKER BONUS */

	bonus = 0;
	vtab = 180;
	htab = 30;
	if (gNumCoins > 300)										// fix to a max amount
		gNumCoins = 300;
	for (i=0; i < gNumCoins; i++)
	{
		if (!(i&0b1))
			PlaySound(SOUND_SELECTCHIME);
		DrawFrameToScreen(htab,vtab,GROUP_BONUS,0,12);			// draw jawbreaker
		bonus += 25;
		PrintBonusNum(bonus, 5, 540, 180);						// draw bonus #
		PresentIndexedFramebuffer();

		ReadKeyboard();
		if (UserWantsOutContinuous())							// see how long to wait
			Wait4(2);
		else
			Wait4(5);

		htab += 13;
		if (htab > 410)
		{
			htab = 30;
			vtab += 13;
		}

		if (i == 200)											// see if get another heart
		{
			if (gMyMaxHealth < MAX_HEARTS)
			{
				gMyMaxHealth++;
				DrawFrameToScreen(520,210,GROUP_BONUS,0,13);	// draw heart
				PlaySound(SOUND_HEALTHDING);
				PresentIndexedFramebuffer();
			}
		}
	}
	GetPoints(bonus);											// add bonus to score
	gNumCoins = 0;												// reset counter

			/* IF FINISHED LAST LEVEL, THEN CANNOT SAVE GAME */

	if (gAreaNum == 2)
	{
		switch(gDifficultySetting)
		{
//			case	DIFFICULTY_NORMAL:
//					if (gSceneNum == 3)
//						goto bye;
//					break;
			case	DIFFICULTY_EASY:
					if (gSceneNum == 2)
						goto bye;
					break;
			case	DIFFICULTY_NORMAL:
			case	DIFFICULTY_HARD:
					if (gSceneNum == 4)
						goto bye;
					break;
		}
	}

				/* DO SAVE SELECTION */

	PresentIndexedFramebuffer();
	Wait4(60*1);
	while(UserWantsOutContinuous())						// wait for button & key up
	{
		Wait(10);
	}

	ReadKeyboard();
	i = 0;
	selection = 0;
	while(true)
	{
		RegulateSpeed2(1);

		if (++i < 10)											// blink sprite frames
			DrawFrameToScreen(320,390,GROUP_BONUS,0,14+selection+1);	// draw dialog sprite
		else
			DrawFrameToScreen(320,390,GROUP_BONUS,0,14);
		if (i > 20)
			i = 0;
		PresentIndexedFramebuffer();

		ReadKeyboard();

		if (GetNewNeedState(kNeed_UIConfirm))
			break;

		if (GetNewNeedState(kNeed_UILeft) || GetNewNeedState(kNeed_UIPrev))		// see if go left
		{
			if (selection > 0)
			{
				selection--;
				PlaySound(SOUND_SELECTCHIME);
			}
		}

		if (GetNewNeedState(kNeed_UIRight) || GetNewNeedState(kNeed_UINext))	// see if go right
		{
			if (selection < 4)
			{
				selection++;
				PlaySound(SOUND_SELECTCHIME);
			}
		}

				/* SEE IF MANUALLY SELECT SAVE #1..4 */

		if (GetNewSDLKeyState(SDL_SCANCODE_1))
		{
			selection = 1;
			break;
		}
		else
		if (GetNewSDLKeyState(SDL_SCANCODE_2))
		{
			selection = 2;
			break;
		}
		else
		if (GetNewSDLKeyState(SDL_SCANCODE_3))
		{
			selection = 3;
			break;
		}
		else
		if (GetNewSDLKeyState(SDL_SCANCODE_4))
		{
			selection = 4;
			break;
		}

	}

	if (selection > 0)												// see if save a game
	{
		if (gPlayerMode == ONE_PLAYER)
			SaveGame(selection,true);
		else								// in2 player mode, save both players
			TryToSaveBothPlayers(selection);
	}
bye:
	PresentIndexedFramebuffer();
	Wait(60*1);

	ZapShapeTable(GROUP_BONUS);
}


/*************** PRINT BONUS NUMBER ****************/
//
// Prints directly to the screen during bonus talley
//
// INPUT: htab = coord or leftmost digit
//

void PrintBonusNum(long	num, short numDigits, short htab, short vtab)
{
register short	i,digit;

	for (i=0; i<numDigits; i++)
	{
		digit = num-(num/10*10);
		DrawFrameToScreen_NoMask(htab,vtab,GROUP_BONUS,0,digit+2);
		htab -= 20;
		num = num/10;
	}
}



