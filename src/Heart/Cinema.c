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

static void DoDifficultyScreen(void);
static void DoWhimpyWinScreen(void);
static void MoveCreditLetter2(void);


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

#define	VIEW_X				450
#define	VIEW_Y				330

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

static	Boolean	gDownButtonPressedFlag,gUpButtonPressedFlag;

static	int32_t		HighScoreList[MAX_HIGH_SCORES];							// note: keep arrays together for saving and loading
static	char		HighScoreNames[MAX_HIGH_SCORES][MAX_NAME_LENGTH+1];

static	long		gDemoTimeout;

static	Boolean		gCursorAtTargetFlag;


			/* CURSOR X */

static	short	cX[] = {CURSOR_X,CURSOR_X+OPTION_SPACING,CURSOR_X+(OPTION_SPACING*2),
				CURSOR_X+(OPTION_SPACING*3),CURSOR_X+(OPTION_SPACING*4),
				CURSOR_X+(OPTION_SPACING*5)};

static	ObjNode		*icons[10];


/******************** DO TITLE SCREEN ********************/

void DoTitleScreen(void)
{
register	ObjNode		*newObj;
Boolean		songFlag;
short		i,startSelection;

	gLoadOldGameFlag = false;

	ReadKeyboard();

	startSelection = 0;										// start selection on item #0

	gAbortDemoFlag = false;									// no demo has been aborted yet.
	gGameIsDemoFlag = false;								// assume wont be playing a demo.

	songFlag = false;
	gCursorAtTargetFlag = false;

re_enter:
	gStartingScene = 0;								// assume not cheating
	gDemoTimeout = TIME_TILL_DEMO;

					/* INITIAL LOADING */

	InitObjectManager();
	LoadShapeTable(":data:shapes:title.shapes",GROUP_MAIN,DONT_GET_PALETTE);	// load graphix & palette
	EraseCLUT();
	EraseBackgroundBuffer();
	LoadBackground(":data:images:titlepage.image",GET_PALETTE);
	DumpBackground();


					/* CREATE CURSOR */

	newObj = MakeNewShape(GroupNum_MeTitle,ObjType_MeTitle,4,cX[startSelection],CURSOR_Y,
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

#if !BETA
	if (Button())								// button + KEYS for final game = cheat
	{
#endif

		if (GetSDLKeyState(SDL_SCANCODE_R) && GetSDLKeyState(SDL_SCANCODE_E))	// see if record demo
		{
			gPlayerMode = ONE_PLAYER;
			ZapShapeTable(GROUP_MAIN);
			StartRecordingDemo();
			return;
		}

					/* CHECK FOR SCENE CHEATS */

		if (GetSDLKeyState(SDL_SCANCODE_1))
			gStartingScene = 1;
		else
		if (GetSDLKeyState(SDL_SCANCODE_2))
			gStartingScene = 2;
		else
		if (GetSDLKeyState(SDL_SCANCODE_3))
			gStartingScene = 3;
		else
		if (GetSDLKeyState(SDL_SCANCODE_4))
			gStartingScene = 4;

#if !BETA
	}
#endif

		if (gCursorMode != CURSOR_MODE_READY)
			continue;
		if (UserWantsOut())
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
		if (GetNeedState(kNeed_UILeft))				// see if left key
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

		if (GetNeedState(kNeed_UIRight))			// see if right key
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
	switch(gSceneNum)
	{
		case	SCENE_JURASSIC:
			LoadIMAGE(":data:images:dinoscene.image",SHOW_IMAGE_FLAG_FADEIN);
			break;

		case	SCENE_CANDY:
			LoadIMAGE(":data:images:candyscene.image",SHOW_IMAGE_FLAG_FADEIN);
			break;

		case	SCENE_CLOWN:
			LoadIMAGE(":data:images:clownscene.image",SHOW_IMAGE_FLAG_FADEIN);
			break;

		case	SCENE_FAIRY:
			LoadIMAGE(":data:images:fairyscene.image",SHOW_IMAGE_FLAG_FADEIN);
			break;

		case	SCENE_BARGAIN:
			LoadIMAGE(":data:images:bargainscene.image",SHOW_IMAGE_FLAG_FADEIN);
			break;

	}
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
	}
	FadeOutGameCLUT();
}

/********************* DISPLAY SCORES ********************/

void	DisplayScores(void)
{
short		i;

	EraseCLUT();
	LoadShapeTable(":data:shapes:highscore.shapes",GROUP_WIN,DONT_GET_PALETTE);
	LoadBackground(":data:images:scores.image",GET_PALETTE);
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

	InitScreenBuffers();					// restore offscreen buffers so we can animate

				/* SEE IF WORTHY */

	for (i=0; i<MAX_HIGH_SCORES; i++)
	{
		if (newScore > HighScoreList[i])
			goto gotit;
	}
	ShowLastScore();										// not a high score, so just show it
	WipeScreenBuffers();									// free some of that buffer memory
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
	WipeScreenBuffers();									// free some of that buffer memory
}

/************************ SHOW LAST SCORE ******************/

void ShowLastScore(void)
{

	LoadShapeTable(":data:shapes:highscore.shapes",GROUP_WIN,GET_PALETTE);
	EraseBackgroundBuffer();

	EraseCLUT();
	DumpBackground();										// dump to playfield
	DumpGameWindow();										// show the whole thing
	DumpUpdateRegions();

					/* DISPLAY SCORE */

	gHtab = VISIBLE_WIDTH/2-50;
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

	LoadShapeTable(":data:shapes:highscore.shapes",GROUP_WIN,GET_PALETTE);


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
		DoFatalAlert("TODO: Implement theChar = CheckKey();");
		EraseObjects();

		if (theChar == '\xBD')							// \xBD is the null key mark
			goto nokey;

		if (theChar == CHAR_RETURN)						// see if done
			goto exit;

					/*  SEE IF BACK CHAR */

		if ((theChar == CHAR_LEFT) || (theChar == CHAR_DELETE))
		{
			if (i>0)
			{
				i--;
				HighScoreNames[n][i] = '\0';
				gHtab = NAME_HTAB;						// set coords of name
				gVtab = TOP_SCORE_VTAB+(n*SCORE_LINE_GAP);
				SetRect(&eraseRect,gHtab+OFFSCREEN_WINDOW_LEFT-30,gVtab+OFFSCREEN_WINDOW_TOP-20,
						gHtab+OFFSCREEN_WINDOW_LEFT+350,gVtab+OFFSCREEN_WINDOW_TOP+20);
				AddUpdateRegion(eraseRect,0);				// erase existing name
				DumpUpdateRegions();
				WriteLn(HighScoreNames[n]);				// print it
				nameObj->X.Int = (gHtab+OFFSCREEN_WINDOW_LEFT);	// set cursor
				nameObj->Y.Int = (gVtab+OFFSCREEN_WINDOW_TOP);
			}
		}

				/* SEE IF NEW CHAR */

		else
		if (i < MAX_NAME_LENGTH)					// see if can enter another char
		{
			if (theChar >= 'a' && theChar <= 'z')
				theChar -= 0x20;					// convert to upper case

			HighScoreNames[n][i] = theChar;			// put char into string
			HighScoreNames[n][i+1] = '\0';
			i++;
			gHtab = NAME_HTAB;						// set coords of name
			gVtab = TOP_SCORE_VTAB+(n*SCORE_LINE_GAP);
			SetRect(&eraseRect,gHtab+OFFSCREEN_WINDOW_LEFT-30,gVtab+OFFSCREEN_WINDOW_TOP-20,
					OFFSCREEN_WINDOW_RIGHT,gVtab+OFFSCREEN_WINDOW_TOP+20);
			AddUpdateRegion(eraseRect,0);				// erase area of cursor
			DumpUpdateRegions();
			WriteLn(HighScoreNames[n]);				// print whole word
			nameObj->X.Int = (gHtab+OFFSCREEN_WINDOW_LEFT);	// set cursor
			nameObj->Y.Int = (gVtab+OFFSCREEN_WINDOW_TOP);
			nameObj->drawBox.left = nameObj->X.Int;
			nameObj->drawBox.right = nameObj->X.Int;
			nameObj->drawBox.top = nameObj->Y.Int;
			nameObj->drawBox.bottom = nameObj->Y.Int;
		}
nokey:
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
	PreLoadSpinFile(":data:movies:pangea.spin",DEFAULT_SPIN_PRELOAD_SIZE);	// load the file
	PlaySong(SONG_ID_PANGEA);

	PlaySpinFile(60*5);
	FadeOutGameCLUT();
	StopMusic();
}

/******************** DO LEGAL ********************/

void DoLegal(void)
{
	EraseCLUT();
	LoadIMAGE(":data:images:legal.image",SHOW_IMAGE_FLAG_FADEIN);
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
	LoadShapeTable(":data:shapes:playerchoose.shapes",GROUP_OVERHEAD,DONT_GET_PALETTE);
	EraseCLUT();
	EraseBackgroundBuffer();
	LoadBackground(":data:images:playerchoose.image",GET_PALETTE);
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

		if (gCursorSelection && GetNeedState(kNeed_UILeft))
		{
			DeactivateObjectDraw(batteryObj2);
			batteryObj->X.Int = BATTERY_X1;
			gCursorSelection = 0;
			PlaySound(SOUND_SELECTCHIME);
		}
		else
		if (!gCursorSelection && GetNeedState(kNeed_UIRight))
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

		if ((GetNewNeedState(kNeed_UILeft)) && (restoreMode > 0))
		{
			restoreMode--;
			cursorObj->X.Int = sgcursorx[restoreMode];
			PlaySound(SOUND_SELECTCHIME);
			SwitchAnim(cursorObj,restoreMode+1);
		}
		else
		if ((GetNewNeedState(kNeed_UIRight)) && (restoreMode < 4))
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

/******************** DO CREDITS SCREEN ********************/

void DoCredits(void)
{
Handle	textRez;
Ptr		textPtr;
short	lineCount,lineLength,x,i;
char	lineBuff[70];
Boolean	endFlag;
ObjNode	*theNode;

					/* INITIAL LOADING */

	FadeOutGameCLUT();
	PlaySong(SONG_ID_RACE);
	InitObjectManager();
	LoadShapeTable(":data:shapes:highscore.shapes",GROUP_WIN,DONT_GET_PALETTE);
	EraseBackgroundBuffer();
	LoadBackground(":data:images:credits1.image",GET_PALETTE);
	DumpBackground();											// dump to playfield

	textRez = GetResource('Cred',128);							// load credits rez
	if (textRez == nil)
		DoFatalAlert("No Credits Rez!");
	DetachResource(textRez);
	HLock(textRez);
	textPtr = *textRez;											// get ptr to text string

				/* SET CLIPPING ZONE */

	gRegionClipTop[3] = OFFSCREEN_WINDOW_TOP+94;
	gRegionClipBottom[3] = OFFSCREEN_WINDOW_TOP+VISIBLE_HEIGHT-20;
	gRegionClipLeft[3] = OFFSCREEN_WINDOW_LEFT;
	gRegionClipRight[3] = OFFSCREEN_WINDOW_LEFT+VISIBLE_WIDTH-1;


						/* LETS DO IT */

	lineCount = 100;
	endFlag = false;

	DumpGameWindow();
	DrawObjects();
	DumpUpdateRegions();
	FadeInGameCLUT();

	do
	{
		RegulateSpeed2(1);									// @ 30fps
		EraseObjects();
		MoveObjects();

					/* SEE IF MAKE NEXT TEXT LINE */

		if (++lineCount > 50)								// see if ready for next line
		{
			lineCount = 0;

			if (*textPtr == '~')							// see if @ the end
				endFlag = true;
			else
			{
				lineLength = 0;									// read next line into buffer
				do
				{
					lineBuff[lineLength++] = *textPtr;			// get a char
				} while(*textPtr++ != '^');
				if (--lineLength > 0)							// check for blank lines
				{
					x = 320-(FONT_WIDTH*lineLength/2);			// calc starting x
					for (i=0; i < lineLength; i++)
					{
						if (lineBuff[i] != ' ')					// skip spaces
						{
							theNode = MakeNewShape(GroupNum_BigFont,ObjType_BigFont,ASCIIToBigFont(lineBuff[i]),
										x,470,100,MoveCreditLetter,SCREEN_RELATIVE);
							theNode->ClipNum = 3;
							theNode->Special1 = MyRandomLong()&b111111111+0x500;
							theNode->DY = -0x10000L;
						}
						x += FONT_WIDTH;
					}
				}
			}
		}


		DrawObjects();
		DumpUpdateRegions();

		ReadKeyboard();
		DoSoundMaintenance(true);							// (must be after readkeyboard)

		if (Button())
			break;

	}while (!UserWantsOut() && NumObjects > 0);

	FadeOutGameCLUT();
	DisposeHandle(textRez);							// zap the text rez
	ZapShapeTable(GROUP_WIN);
}


/*********************** MOVE CREDIT LETTER ****************************/
//
// also moves letters for whimpy win text
//

void MoveCreditLetter(void)
{
	if (gThisNodePtr->Y.Int > 250)
		gThisNodePtr->Y.L -= 0x8000L;					// move up
	else
	{
		gThisNodePtr->Y.L += gThisNodePtr->DY;
		gThisNodePtr->DY -= gThisNodePtr->Special1;
	}

	if (gThisNodePtr->Y.Int < 95)
		DeleteObject(gThisNodePtr);

}

/*********************** MOVE CREDIT LETTER2 ****************************/
//
//  win text
//

static void MoveCreditLetter2(void)
{
	if (gThisNodePtr->Y.Int > 250)
		gThisNodePtr->Y.L -= 0xb000L;					// move up
	else
	{
		gThisNodePtr->Y.L += gThisNodePtr->DY;
		gThisNodePtr->DY -= gThisNodePtr->Special1;
	}

	if (gThisNodePtr->Y.Int < 30)					// (for whimpy text)
		DeleteObject(gThisNodePtr);
}

#pragma mark -

/******************** DO OVERHEAD MAP ********************/
//
// OUTPUT: true if aborted screen from demo playback
//

void DoOverheadMap(void)
{
ObjNode		*mikeObj;
static	Point	toCoord[MAX_SCENES][3] =						// coords to move Mike TO (y,x)
						{
							419,537,							// jurassic
							351,463,
							246,518,

							437,100,							// candy
							332,214,
							345,62,

							176,203,							// fairy
							181,281,
							167,383,

							144,438,							// clown
							94,512,
							93,375,

							115,176,							// bargain
							81,47,
							39,216
						};

static	Point		compCoord[MAX_SCENES] =						// coords to put COMPLETED icon
						{
							298,518,
							353,168,
							181,272,
							95,469,
							68,116
						};

short	destX,destY,fromX,fromY;
Boolean	flag;
short	counter,i;

	if (gDemoMode != DEMO_MODE_OFF)								// dont show in demo mode
		return;

	FadeOutGameCLUT();											// fade out old screen
	BlankEntireScreenArea();
	OptimizeMemory();											// free up some memory so we can allocate draw buffers for animation

	InitScreenBuffers();										// restore offscreen buffers so we can animate


					/* PLAY INTRO MELODY */

	switch(gSceneNum)
	{
		case	SCENE_JURASSIC:
				PlaySong(SONG_ID_JURASSIC_INTRO);
				break;

		case	SCENE_CANDY:
				PlaySong(SONG_ID_CANDY_INTRO);
				break;

		case	SCENE_CLOWN:
				PlaySong(SONG_ID_CLOWN_INTRO);
				break;

		case	SCENE_FAIRY:
				PlaySong(SONG_ID_FAIRY_INTRO);
				break;

		case	SCENE_BARGAIN:
				PlaySong(SONG_ID_BARGAIN_INTRO);
				break;
	}

					/* INITIAL LOADING */

	InitObjectManager();
	LoadShapeTable(":data:shapes:overheadmap.shapes",GROUP_OVERHEAD,DONT_GET_PALETTE);	// load graphix & palette
	EraseCLUT();
	EraseBackgroundBuffer();

	switch(gDifficultySetting)
	{
//		case	DIFFICULTY_NORMAL:
//				LoadBackground(":data:images:overheadmap3.image",GET_PALETTE);
//				break;
		case	DIFFICULTY_EASY:
				LoadBackground(":data:images:overheadmap2.image",GET_PALETTE);
				break;
		case	DIFFICULTY_NORMAL:
		case	DIFFICULTY_HARD:
				LoadBackground(":data:images:overheadmap.image",GET_PALETTE);
				break;
	}

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
	WipeScreenBuffers();							// free some of that buffer memory
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
	LoadIMAGE(":data:images:lose.image",SHOW_IMAGE_FLAG_FADEIN);
	WaitWhileMusic();
	StopMusic();
}


/******************** DO WIN SCREEN ********************/

void DoWinScreen(void)
{
ObjNode	*newObj;



//	if (gDifficultySetting != DIFFICULTY_HARD)
	if (gDifficultySetting < DIFFICULTY_NORMAL)
	{
		DoWhimpyWinScreen();
		goto more;
	}
	else
		DoWhimpyWinScreen();


					/* INITIAL LOADING */
	FadeOutGameCLUT();
	InitScreenBuffers();					// restore offscreen buffers so we can animate
	InitObjectManager();



	PlaySong(SONG_ID_WINGAME);
	LoadShapeTable(":data:shapes:win.shapes",GROUP_WIN,DONT_GET_PALETTE);
	EraseBackgroundBuffer();
	LoadBackground(":data:images:win.image",GET_PALETTE);
	DumpBackground();

						/* LETS DO IT */

	DumpGameWindow();
	DrawObjects();
	DumpUpdateRegions();
	FadeInGameCLUT();

	do
	{
		RegulateSpeed2(1);									// @ 30fps
		EraseObjects();
		MoveObjects();

							/* ADD CONFETTI */

		newObj = MakeNewShape(GroupNum_Confetti,ObjType_Confetti,RandomRange(0,9),
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

		if (Button())
			break;

	} while (!UserWantsOut() && IsMusicPlaying());

more:
	FadeOutGameCLUT();
	StopMusic();

				/* SHOW CREDITS WHEN DONE */

	DoCredits();
	StopMusic();
	ZapShapeTable(GROUP_WIN);
	WipeScreenBuffers();							// free some of that buffer memory
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


/******************** DO WHIMPY WIN SCREEN ********************/
//
// Lame scrolling text for whimpy win EASY/NORMAL difficulty
//

static void DoWhimpyWinScreen(void)
{
Handle	textRez;
Ptr		textPtr;
short	lineCount,lineLength,x,i;
char	lineBuff[70];
Boolean	endFlag;
ObjNode	*theNode;

	FadeOutGameCLUT();
	InitScreenBuffers();					// restore offscreen buffers so we can animate
	InitObjectManager();

					/* INITIAL LOADING */

	if (gDifficultySetting >= DIFFICULTY_NORMAL)
		PlaySong(SONG_ID_WINHUM);
	else
		PlaySong(SONG_ID_WINGAMELOOP);
	LoadShapeTable(":data:shapes:highscore.shapes",GROUP_WIN,DONT_GET_PALETTE);
	EraseBackgroundBuffer();
	LoadBackground(":data:images:winbw.image",GET_PALETTE);
	DumpBackground();											// dump to playfield

	if (gDifficultySetting == DIFFICULTY_EASY)
		textRez = GetResource('Cred',129);						// load credits rez
	else
//	if (gDifficultySetting == DIFFICULTY_NORMAL)
//		textRez = GetResource('Cred',130);
//	else
//	if (gDifficultySetting == DIFFICULTY_HARD)
		textRez = GetResource('Cred',131);

	if (textRez == nil)
		DoFatalAlert("No WinText Rez!");
	DetachResource(textRez);
	HLock(textRez);
	textPtr = *textRez;											// get ptr to text string

				/* SET CLIPPING ZONE */

	gRegionClipTop[3] = OFFSCREEN_WINDOW_TOP+40;
	gRegionClipBottom[3] = OFFSCREEN_WINDOW_TOP+VISIBLE_HEIGHT-20;
	gRegionClipLeft[3] = OFFSCREEN_WINDOW_LEFT;
	gRegionClipRight[3] = OFFSCREEN_WINDOW_LEFT+VISIBLE_WIDTH-1;


						/* LETS DO IT */

	lineCount = 100;
	endFlag = false;

	DumpGameWindow();
	DrawObjects();
	DumpUpdateRegions();
	FadeInGameCLUT();

	do
	{
		RegulateSpeed2(1);									// @ 30fps
		EraseObjects();
		MoveObjects();

					/* SEE IF MAKE NEXT TEXT LINE */

		if (++lineCount > 50)								// see if ready for next line
		{
			lineCount = 0;

			if (*textPtr == '~')							// see if @ the end
				endFlag = true;
			else
			{
				lineLength = 0;									// read next line into buffer
				do
				{
					lineBuff[lineLength++] = *textPtr;			// get a char
				} while(*textPtr++ != '^');
				if (--lineLength > 0)							// check for blank lines
				{
					x = 320-(FONT_WIDTH*lineLength/2);			// calc starting x
					for (i=0; i < lineLength; i++)
					{
						if (lineBuff[i] != ' ')					// skip spaces
						{
							theNode = MakeNewShape(GroupNum_BigFont,ObjType_BigFont,ASCIIToBigFont(lineBuff[i]),
										x,470,100,MoveCreditLetter2,SCREEN_RELATIVE);
							theNode->ClipNum = 3;
							theNode->Special1 = MyRandomLong()&b111111111+0x500;
							theNode->DY = -0x10000L;
						}
						x += FONT_WIDTH;
					}
				}
			}
		}


		DrawObjects();
		DumpUpdateRegions();

		ReadKeyboard();
		DoSoundMaintenance(true);							// (must be after readkeyboard)

		if (Button())
			break;

	}while(!UserWantsOut() && NumObjects > 0);

	DisposeHandle(textRez);								// zap the text rez
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
	LoadShapeTable(":data:shapes:difficulty.shapes",GROUP_MAIN,DONT_GET_PALETTE);	// load graphix & palette
	LoadBackground(":data:images:diff.image",GET_PALETTE);
	DumpBackground();

	mode = gDifficultySetting;


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

		if (GetNewNeedState(kNeed_UIRight))	// see if go right
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
		if (GetNewNeedState(kNeed_UILeft))	// see if left
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

	gDifficultySetting = mode;

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
	LoadShapeTable(":data:shapes:view.shapes",GROUP_MAIN,DONT_GET_PALETTE);	// load graphix & palette
	LoadBackground(":data:images:head.image",GET_PALETTE);
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
	LoadShapeTable(":data:shapes:bonus.shapes",GROUP_BONUS,DONT_GET_PALETTE);
	EraseCLUT();

	BlankEntireScreenArea();
	LoadIMAGE(":data:images:bonus.image",SHOW_IMAGE_FLAG_FADEIN);

	if (gMusicOnFlag)
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
		if (!(i&b1))
			PlaySound(SOUND_SELECTCHIME);
		DrawFrameToScreen(htab,vtab,GROUP_BONUS,0,12);			// draw jawbreaker
		bonus += 25;
		PrintBonusNum(bonus, 5, 540, 180);						// draw bonus #
		PresentIndexedFramebuffer();

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
	while(UserWantsOutContinuous());					// wait for button & key up

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

		if (GetNewNeedState(kNeed_UILeft))	// see if go left
		{
			if (selection > 0)
			{
				selection--;
				PlaySound(SOUND_SELECTCHIME);
			}
		}

		if (GetNewNeedState(kNeed_UIRight))	// see if go right
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



