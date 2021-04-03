// SETTINGS SCREEN
// (C) 2021 Iliyas Jorio
// This file is part of Mighty Mike. https://github.com/jorio/mightymike

/****************************/
/*    EXTERNALS             */
/****************************/

#include <stdio.h>
#include "myglobals.h"
#include "objecttypes.h"
#include "window.h"
#include "picture.h"
#include "playfield.h"
#include "object.h"
#include "infobar.h"
#include "cinema.h"
#include "misc.h"
#include "sound2.h"
#include "shape.h"
#include "io.h"
#include "main.h"
#include "input.h"
#include "version.h"
#include "externs.h"
#include <SDL.h>
#include <ctype.h>
#include <string.h>

/****************************/
/*    PROTOTYPES            */
/****************************/

typedef struct SettingEntry
{
	Byte*			valuePtr;
	const char*		label;
	void			(*callback)(void);
	unsigned int	numChoices;
	const char*		choices[8];
} SettingEntry;


static int LayOutText(const char* label, int row, int col, int flags);
static void NukeText(int row, int col);
static const char* ProcessScancodeName(int scancode);
static void OnEnterControls(void);
static void OnDone(void);
static void OnChangePlayfieldSizeViaSettings(void);
static void LayOutSettingsPageBackground(void);
static void LayOutSettingsPageObjects(void);
static void LayOutSettingsPage(void);
static void LayOutControlsPage(void);


/****************************/
/*    CONSTANTS             */
/****************************/

#define kSpecialLetterMagicValue 'LTTR'		// stored in an ObjNode's Special0 to signify that it is a letter node

#define SpecialLetterMagic	Special0
#define SpecialLetterBaseY	Special1
#define SpecialLetterRow	Special2
#define SpecialLetterCol	Special3
#define FlagLetterJitter	Flag0

enum
{
	kSettingsState_Off,
	kSettingsState_MainPage,
	kSettingsState_ControlsPage,
	kSettingsState_ControlsPage_AwaitingPress,
};

enum
{
	kTextFlags_AsObject			= 1 << 0,
	kTextFlags_BounceUp			= 1 << 1,
	kTextFlags_Condensed		= 1 << 2,
	kTextFlags_Jitter			= 1 << 3,
};

static const int kNumKeybindingRows		= NUM_REMAPPABLE_NEEDS + 2;  // +2 extra rows for Reset to defaults & Done
static const int kKeybindingRow_Reset	= NUM_REMAPPABLE_NEEDS + 0;
static const int kKeybindingRow_Done	= NUM_REMAPPABLE_NEEDS + 1;

static const char* kInputNeedCaptions[NUM_REMAPPABLE_NEEDS] =
{
	[kNeed_Up			] = "go up",
	[kNeed_Down			] = "go down",
	[kNeed_Left			] = "go left",
	[kNeed_Right		] = "go right",
	[kNeed_Attack		] = "attack",
	//[kNeed_PrevWeapon	] = "previous weapon",
	[kNeed_NextWeapon	] = "next weapon",
	[kNeed_Radar		] = "bunny radar",
};

static const int8_t kLetterWidths[] =
{
	20,20,20,20, 20,20,20,20, 20,20,20,20, 20,20,20,20,
	20,20,20,20, 20,20,20,20, 20,20,20,20, 20,20,20,20,
	 8, 5,13,13, 13,13,13, 5, 13,13,13,13,  5,13, 5,13,	//  !"# $%&' ()*+ ,-./
	12,12,12,12, 12,12,12,12, 12,12, 5, 5, 13,13,13,13,	// 0123 4567 89:; <=>?
	20,20,20,20, 20,20,20,20, 20,20,20,20, 20,20,20,20,	// @ABC DEFG HIJK LMNO
	20,20,20,20, 20,20,20,20, 20,20,20,20, 20,20,20,20,	// PQRS TUVW XYZ
	18,13,13,11, 15,11,11,18, 13, 5, 9,13, 10,19,15,18,	//  abc defg hijk lmno
	13,18,13,12, 11,13,15,19, 14,12,13,20, 20,20,20,20,	// pqrs tuvw xyz
};

static const int kColumnX[] = { 64, 300, 475, 550 };
static const int kRowY0 = 100;
static const int kRowHeight = 30;

/****************************/
/*    VARIABLES             */
/****************************/

static int gSettingsState = kSettingsState_Off;
static int gSettingsRow = 0;
static int gControlsRow = 0;
static int gControlsColumn = 0;

static SettingEntry gSettingEntries[] =
{
	{nil							, "configure controls"	, OnEnterControls,			0,	{ NULL } },
	{nil							, nil					, nil,						0,  { NULL } },
	{&gGamePrefs.fullscreen			, "fullscreen"			, SetFullscreenMode,		2,	{ "no", "yes" }, },
	{&gGamePrefs.pfSize				, "playfield size"		, OnChangePlayfieldSizeViaSettings,	2 /* TODO: set this to 3 to enable wide */,  { "small", "medium", "wide" } },
	{&gGamePrefs.integerScaling		, "upscaling"			, OnChangeIntegerScaling,	2,  { "stretch", "crisp" } },
	{&gGamePrefs.uncappedFramerate	, "frame rate"			, nil,						2,  { "32 fps original", "uncapped" } },
	{&gGamePrefs.filterDithering	, "dithering"			, nil,						2,  { "   raw", "   filtered" } },
	{&gGamePrefs.interpolateAudio	, "audio quality"		,OnChangeAudioInterpolation,2,	{ "raw", "interpolated" } },
	{&gGamePrefs.gameTitlePowerPete	, "game title"			, nil,						2,  { "mighty mike", "power pete" } },
	{nil							, nil					, nil,						0,  { NULL } },
	{nil							, "done"				, OnDone,					0,  { NULL } },
};

#define numSettingEntries ( (int) (sizeof(gSettingEntries) / sizeof(SettingEntry)) )

/******************************************************************************/

/****************************/
/*    UTILITIES             */
/****************************/
#pragma mark - Utilities

static void Cycle(SettingEntry* entry, int delta)
{
	if (entry->valuePtr)
	{
		unsigned int value = (unsigned int)*entry->valuePtr;
		value = PositiveModulo(value + delta, entry->numChoices);
		*entry->valuePtr = value;
	}

	if (entry->callback)
	{
		entry->callback();
	}
}

static void ForceUpdateBackground(void)
{
	Rect r;
	r.left		= 0;
	r.top		= 0;
	r.right		= OFFSCREEN_WIDTH;
	r.bottom	= OFFSCREEN_HEIGHT;
	AddUpdateRegion(r, CLIP_REGION_PLAYFIELD);
}

static int GetRowY(int row)
{
	return kRowY0 + row * kRowHeight;
}

/****************************/
/*    CALLBACKS             */
/****************************/
#pragma mark - Callbacks

static void OnEnterControls(void)
{
	ReadKeyboard();	// flush keypresses

	gSettingsState = kSettingsState_ControlsPage;
	FadeOutGameCLUT();
	LayOutControlsPage();
}

static void OnDone(void)
{
	ReadKeyboard();

	switch (gSettingsState)
	{
	case kSettingsState_ControlsPage:
		FadeOutGameCLUT();
		gSettingsState = kSettingsState_MainPage;
		LayOutSettingsPage();
		break;

	case kSettingsState_ControlsPage_AwaitingPress:
		PlaySound(SOUND_BADHIT);
		NukeText(gControlsRow, gControlsColumn + 1);
		LayOutText(ProcessScancodeName(gGamePrefs.keys[gControlsRow].key[gControlsColumn]), gControlsRow, gControlsColumn + 1, kTextFlags_AsObject | kTextFlags_BounceUp | kTextFlags_Condensed);
		gSettingsState = kSettingsState_ControlsPage;
		break;

	case kSettingsState_MainPage:
	default:
		FadeOutGameCLUT();
		gSettingsState = kSettingsState_Off;
		break;
	}
}

static void OnChangePlayfieldSizeViaSettings(void)
{
	gScreenBlankedFlag = true;
	OnChangePlayfieldSize();
	gScreenBlankedFlag = false;
	LayOutSettingsPageBackground();
}

/****************************/
/*    MOVE CALLS            */
/****************************/
#pragma mark - Move Calls

static void MoveText(void)
{
	GetObjectInfo();

	ObjNode* theNode = gThisNodePtr;

	theNode->DY += 0x10000L;							// add gravity

	theNode->YOffset.L += theNode->DY;					// move it

	if (theNode->YOffset.Int > 0)						// see if bounce
	{
		theNode->YOffset.Int = 0;						// rebound
		if (theNode->FlagLetterJitter)
			theNode->DY = -0x10000 * RandomRange(2, 4);
		else
			theNode->DY = -theNode->DY / 2;
	}

	gThisNodePtr->Y.L = theNode->SpecialLetterBaseY + theNode->YOffset.L;		// move up

//	UpdateObject();
}

static void MoveCursor(void)
{
	switch (gSettingsState)
	{
		case kSettingsState_MainPage:
			gThisNodePtr->X.Int = kColumnX[0] - 20;
			gThisNodePtr->Y.Int = GetRowY(gSettingsRow);
			break;

		case kSettingsState_ControlsPage:
		case kSettingsState_ControlsPage_AwaitingPress:
			if (gControlsRow < NUM_REMAPPABLE_NEEDS)
			{
				gThisNodePtr->X.Int = kColumnX[1 + gControlsColumn] - 20;
				gThisNodePtr->Y.Int = GetRowY(gControlsRow);
			}
			else
			{
				gThisNodePtr->X.Int = kColumnX[0] - 20;
				gThisNodePtr->Y.Int = GetRowY(gControlsRow + 1);
			}
			break;
	}
}

/****************************/
/*    SCANCODE STUFF        */
/****************************/
#pragma mark - Scancode stuff

static const char* ProcessScancodeName(int scancode)
{
	switch (scancode)
	{
		case 0:							return "---";
		case SDL_SCANCODE_SEMICOLON:	return "semicolon";
		case SDL_SCANCODE_LEFTBRACKET:	return "l bracket";
		case SDL_SCANCODE_RIGHTBRACKET:	return "r bracket";
		case SDL_SCANCODE_SLASH:		return "slash";
		case SDL_SCANCODE_BACKSLASH:	return "backslash";
		case SDL_SCANCODE_GRAVE:		return "grave";
		case SDL_SCANCODE_MINUS:		return "minus";
		case SDL_SCANCODE_EQUALS:		return "equals";
		case SDL_SCANCODE_APOSTROPHE:	return "apostrophe";
		case SDL_SCANCODE_COMMA:		return "comma";
		case SDL_SCANCODE_PERIOD:		return "period";
	}

#define kNameBufferLength 64
	static char nameBuffer[kNameBufferLength];

	const char* nameSource = SDL_GetScancodeName(scancode);
	for (int i = 0; i < kNameBufferLength - 1; i++)
	{
		char c = nameSource[i];
		if (c == '\0')
		{
			nameBuffer[i] = '\0';
			break;
		}
		else
		{
			nameBuffer[i] = tolower(c);
		}
	}
	nameBuffer[kNameBufferLength-1] = '\0';
	return nameBuffer;
#undef kNameBufferLength
}

static int16_t* GetSelectedKeybindingKeyPtr(void)
{
	GAME_ASSERT(gControlsRow >= 0 && gControlsRow < NUM_REMAPPABLE_NEEDS);
	GAME_ASSERT(gControlsColumn >= 0 && gControlsColumn < KEYBINDING_MAX_KEYS);
	KeyBinding* kb = &gGamePrefs.keys[gControlsRow];
	return &kb->key[gControlsColumn];
}

static void UnbindScancodeFromAllRemappableInputNeeds(int16_t sdlScancode)
{
	for (int i = 0; i < NUM_REMAPPABLE_NEEDS; i++)
	{
		for (int j = 0; j < KEYBINDING_MAX_KEYS; j++)
		{
			if (gGamePrefs.keys[i].key[j] == sdlScancode)
			{
				gGamePrefs.keys[i].key[j] = 0;
				NukeText(i, j+1);
				LayOutText(ProcessScancodeName(0), i, j+1, kTextFlags_AsObject | kTextFlags_BounceUp);
			}
		}
	}
}

/****************************/
/*    MENU NAVIGATION       */
/****************************/
#pragma mark - Menu navigation

static void NavigateSettingEntriesVertically(int delta)
{
	do
	{
		gSettingsRow += delta;
		gSettingsRow = PositiveModulo(gSettingsRow, (unsigned int)numSettingEntries);
	} while (nil == gSettingEntries[gSettingsRow].label);
	PlaySound(SOUND_SELECTCHIME);
}

static void NavigateSettingsPage(void)
{
	if (GetNewNeedState(kNeed_UIBack))
		OnDone();

	if (GetNewNeedState(kNeed_UIUp))
		NavigateSettingEntriesVertically(-1);

	if (GetNewNeedState(kNeed_UIDown))
		NavigateSettingEntriesVertically(1);

	if (GetNewNeedState(kNeed_UIConfirm) ||
		(gSettingEntries[gSettingsRow].valuePtr && (GetNewNeedState(kNeed_UILeft) || GetNewNeedState(kNeed_UIRight))))
	{
		SettingEntry* entry = &gSettingEntries[gSettingsRow];

		if (entry->valuePtr == &gGamePrefs.interpolateAudio)
			PlaySound(SOUND_COMEHERERODENT);
		else if (entry->callback == OnDone)		// let OnDone play its own sound
			PlaySound(SOUND_SQUEEK);
		else
			PlaySound(SOUND_GETPOW);

		int delta = GetNewNeedState(kNeed_UILeft) ? -1 : 1;
		Cycle(entry, delta);

		if (entry->numChoices > 0)
		{
			NukeText(gSettingsRow, 1);

			LayOutText(entry->choices[*entry->valuePtr], gSettingsRow, 1, kTextFlags_AsObject | kTextFlags_BounceUp);
		}
	}
}

static void NavigateControlsPage(void)
{
	if (GetNewNeedState(kNeed_UIBack))
		OnDone();

	if (GetNewNeedState(kNeed_UIUp))
	{
		gControlsRow = PositiveModulo(gControlsRow - 1, (unsigned int)kNumKeybindingRows);
		PlaySound(SOUND_SELECTCHIME);
	}

	if (GetNewNeedState(kNeed_UIDown))
	{
		gControlsRow = PositiveModulo(gControlsRow + 1, (unsigned int)kNumKeybindingRows);
		PlaySound(SOUND_SELECTCHIME);
	}

	if (GetNewNeedState(kNeed_UILeft))
	{
		gControlsColumn = PositiveModulo(gControlsColumn - 1, 2);
		PlaySound(SOUND_SELECTCHIME);
	}

	if (GetNewNeedState(kNeed_UIRight))
	{
		gControlsColumn = PositiveModulo(gControlsColumn + 1, 2);
		PlaySound(SOUND_SELECTCHIME);
	}

	if (GetNewSDLKeyState(SDL_SCANCODE_DELETE) || GetNewSDLKeyState(SDL_SCANCODE_BACKSPACE))
	{
		*GetSelectedKeybindingKeyPtr() = 0;
		PlaySound(SOUND_PIESQUISH);
		PlaySound(SOUND_POP);

		NukeText(gControlsRow, gControlsColumn + 1);
		LayOutText(ProcessScancodeName(0), gControlsRow, gControlsColumn + 1, kTextFlags_AsObject | kTextFlags_BounceUp | kTextFlags_Condensed);
	}

	if (GetNewSDLKeyState(SDL_SCANCODE_RETURN) && gControlsRow < NUM_REMAPPABLE_NEEDS)
	{
		gSettingsState = kSettingsState_ControlsPage_AwaitingPress;
		NukeText(gControlsRow, gControlsColumn + 1);
		LayOutText("press a key!", gControlsRow, gControlsColumn + 1, kTextFlags_AsObject | kTextFlags_BounceUp | kTextFlags_Jitter);
	}

	if (GetNewNeedState(kNeed_UIConfirm))
	{
		if (gControlsRow == kKeybindingRow_Reset)
		{
			memcpy(gGamePrefs.keys, kDefaultKeyBindings, sizeof(kDefaultKeyBindings));
			_Static_assert(sizeof(kDefaultKeyBindings) == sizeof(gGamePrefs.keys), "size mismatch: default keybindings / prefs keybindings");
			PlaySound(SOUND_FIREHOLE);
			LayOutControlsPage();
		}
		else if (gControlsRow == kKeybindingRow_Done)
		{
			PlaySound(SOUND_SQUEEK);
			OnDone();
		}
	}
}

static void NavigateControlsPage_AwaitingPress(void)
{
	if (GetNewNeedState(kNeed_UIBack))
	{
		OnDone();
		return;
	}

	for (int i = 0; i < SDL_NUM_SCANCODES; i++)
	{
		if (GetNewSDLKeyState(i))
		{
			UnbindScancodeFromAllRemappableInputNeeds(i);
			*GetSelectedKeybindingKeyPtr() = i;
			NukeText(gControlsRow, gControlsColumn + 1);
			LayOutText(ProcessScancodeName(i), gControlsRow, gControlsColumn + 1, kTextFlags_AsObject | kTextFlags_BounceUp | kTextFlags_Condensed);
			gSettingsState = kSettingsState_ControlsPage;
			PlaySound(SOUND_COINS);
			break;
		}
	}
}

/****************************/
/*    TEXT LAYOUT           */
/****************************/
#pragma mark - Text Layout

static int LayOutText(const char* label, int row, int col, int flags)
{
	bool asObject		= flags & kTextFlags_AsObject;
	bool bounceUp		= flags & kTextFlags_BounceUp;
	bool condensed		= flags & kTextFlags_Condensed;
	bool jitter			= flags & kTextFlags_Jitter;

	GAME_ASSERT(asObject || !bounceUp);
	GAME_ASSERT(col >= 0 && col < (int)(sizeof(kColumnX)/sizeof(kColumnX[0])));

	int x = kColumnX[col];
	int y = GetRowY(row);

	for (const char* c = label; *c; c++)
	{
		unsigned char cc = *c;

		if (cc == ' ')
		{
			if (condensed)
				x += kLetterWidths[' '] / 2;
			else
				x += kLetterWidths[' '];
			continue;
		}

		Byte frameID = ASCIIToBigFont(cc);
		if (frameID != 0)	// TODO: why do we have to decrement frameID to draw non-animated frames, but not for MakeNewShape?
			frameID--;

		const FrameHeader* fh = GetFrameHeader(GroupNum_BigFont, ObjType_BigFont, frameID, nil, nil);

		int myWidth = 13;
		int cancelXOff = -fh->x;
		int cancelYOff = 0;

		if (cc < sizeof(kLetterWidths) / sizeof(kLetterWidths[0]))
		{
			myWidth = kLetterWidths[cc];
		}

		if (cc >= 'a' && cc <= 'z')
		{
			cancelXOff = -fh->x;
			cancelYOff = -fh->y - 9;
		}
		else if (cc >= 'A' && cc <= 'Z')
		{
			cancelXOff = 0;
			cancelYOff = 0;
		}

		if (asObject)
		{
			ObjNode* newObj = MakeNewShape(
					GroupNum_BigFont,
					ObjType_BigFont,
					ASCIIToBigFont(cc),
					x + cancelXOff,
					y + cancelYOff,
					-row,
					MoveText,
					false);
			GAME_ASSERT_MESSAGE(newObj, "Too many objects on screen!");

			newObj->SpecialLetterMagic	= kSpecialLetterMagicValue;
			newObj->SpecialLetterRow	= row;
			newObj->SpecialLetterCol	= col;
			newObj->SpecialLetterBaseY	= (y + cancelYOff) << 16;
			newObj->FlagLetterJitter	= jitter;

			if (bounceUp)
			{
				newObj->DY = -0x10000 * RandomRange(3, 6);
			}
		}
		else
		{
			DrawFrameToBackground(x + cancelXOff, y + cancelYOff, GroupNum_BigFont, ObjType_BigFont, frameID);
		}

		x += myWidth + (condensed? 0: 2);
	}

	return x;
}

static void NukeText(int row, int col)
{
	ObjNode* theNode = FirstNodePtr;

	while (theNode)
	{
		ObjNode* next = theNode->NextNode;

		if (theNode->SpecialLetterMagic == kSpecialLetterMagicValue
			&& theNode->SpecialLetterRow == row
			&& theNode->SpecialLetterCol == col)
		{
			DeleteObject(theNode);
		}

		theNode = next;
	}
}

/****************************/
/*    PAGE LAYOUT           */
/****************************/
#pragma mark - Page Layout

static void LayOutSettingsPageBackground(void)
{
	EraseBackgroundBuffer();

	LayOutText("S E T T I N G S", -2, 0, 0);

	// Draw dithering pattern
	Ptr ditheringPatternPlot = *gBackgroundHandle;
	ditheringPatternPlot += (GetRowY(6) - kRowHeight/3) * OFFSCREEN_WIDTH;
	ditheringPatternPlot += kColumnX[1];
	for (int y = 0; y < 2*kRowHeight/3; y++)
	{
		for (int x = y % 2; x < 20; x += 2)
		{
			ditheringPatternPlot[x] = 215;
		}
		ditheringPatternPlot += OFFSCREEN_WIDTH;
	}

	for (int i = 0; i < numSettingEntries; i++)
	{
		SettingEntry* entry = &gSettingEntries[i];

		if (entry->label)
		{
			LayOutText(entry->label, i, 0, 0);
		}
	}

	ForceUpdateBackground();
	DumpBackground();											// dump to playfield
}

static void LayOutSettingsPageObjects(void)
{
	DeleteAllObjects();

	for (int i = 0; i < numSettingEntries; i++)
	{
		SettingEntry* entry = &gSettingEntries[i];

		if (entry->label && entry->numChoices > 0)
		{
			const char* choiceCaption = entry->choices[*entry->valuePtr];
			LayOutText(choiceCaption, i, 1, kTextFlags_AsObject | kTextFlags_BounceUp);
		}
	}

	// Make cursor
	MakeNewShape(GroupNum_Bone, ObjType_Bone, 0, 64, GetRowY(gSettingsRow), 0, MoveCursor, 0);
}

static void LayOutSettingsPage(void)
{
	LayOutSettingsPageBackground();
	LayOutSettingsPageObjects();
}


static void LayOutControlsPage(void)
{
	DeleteAllObjects();
	EraseBackgroundBuffer();

	LayOutText("C O N F I G U R E   C O N T R O L S", -2, 0, 0);

	for (int i = 0; i < NUM_REMAPPABLE_NEEDS; i++)
	{
		KeyBinding* kb = &gGamePrefs.keys[i];

		LayOutText(kInputNeedCaptions[i], i, 0, 0);

		for (int j = 0; j < 2; j++)
		{
			const char* name = ProcessScancodeName(kb->key[j]);

			LayOutText(name, i+0, j+1, kTextFlags_AsObject | kTextFlags_BounceUp | kTextFlags_Condensed);
		}
	}

	LayOutText("reset to defaults", NUM_REMAPPABLE_NEEDS+1, 0, 0);
	LayOutText("done", NUM_REMAPPABLE_NEEDS+2, 0, 0);

	ForceUpdateBackground();
	DumpBackground();											// dump to playfield

	// Make cursor
	MakeNewShape(GroupNum_Bone, ObjType_Bone, 0, 64, GetRowY(gControlsRow), 0, MoveCursor, 0);
}

/****************************/
/*    PUBLIC                */
/****************************/
#pragma mark - Public

void DoSettingsScreen(void)
{
					/* INITIAL LOADING */

	FadeOutGameCLUT();
	InitObjectManager();
	LoadShapeTable(":shapes:highscore.shapes", GROUP_WIN);
	LoadShapeTable(":shapes:jurassic1.shapes", GROUP_AREA_SPECIFIC);	// cursor bone
	LoadBackground(":images:winbw.tga");								// just to load the palette

						/* LETS DO IT */

	LayOutSettingsPage();


	gSettingsState = kSettingsState_MainPage;

	do
	{
		RegulateSpeed2(2);									// @ 30fps
		EraseObjects();
		MoveObjects();

		DrawObjects();
		DumpUpdateRegions();

		if (gScreenBlankedFlag)
			FadeInGameCLUT();

		ReadKeyboard();

		switch (gSettingsState)
		{
			case kSettingsState_MainPage:
				NavigateSettingsPage();
				break;

			case kSettingsState_ControlsPage:
				NavigateControlsPage();
				break;

			case kSettingsState_ControlsPage_AwaitingPress:
				NavigateControlsPage_AwaitingPress();
				break;

			default:
				break;
		}

		DoSoundMaintenance(true);							// (must be after readkeyboard)

	} while (gSettingsState != kSettingsState_Off);

	FadeOutGameCLUT();

	SavePrefs();
	
	ZapShapeTable(GROUP_WIN);
	ZapShapeTable(GROUP_AREA_SPECIFIC);
}

void ApplyPrefs(void)
{
	OnChangePlayfieldSize();
	SetFullscreenMode();
	OnChangeIntegerScaling();
}
