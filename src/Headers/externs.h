#pragma once

#include "equates.h"
#include "structures.h"
#include "window.h"
#include <SDL.h>

#pragma mark - Bonus

extern	short					gShieldTimer;
extern	ObjNode*				gShieldNodePtr;
extern	short					gNumBunnies;
extern	Byte					gBunnyCounts[5][3];

# pragma mark - Collision

extern	CollisionRec			gCollisionList[MAX_COLLISIONS];
extern	Byte					gTotalSides;
extern	short					gNumCollisions;

#pragma mark - Enemy (generic)

extern	short					gNumEnemies;
extern	short					gEnemyFreezeTimer;

#pragma mark - Enemy (specific)

extern	long					gLastUngaTime;
extern	long					gLastGummyHahaTime;
extern	long					gLastWitchHahaTime;
extern	long					gLastDogRoarTime;
extern	long					gLastRobotDangerTime;
extern	long					gLastClownLaughTime;

#pragma mark - Infobar

extern	long					gScore;
extern	short					gMyHealth;
extern	short					gMyMaxHealth;
extern	long					gNumCoins;
extern	short					gNumLives;

#pragma mark - Input

extern const KeyBinding			kDefaultKeyBindings[NUM_CONTROL_NEEDS];
extern char						gTextInput[SDL_TEXTINPUTEVENT_TEXT_SIZE];
extern SDL_GameController*		gSDLController;

#pragma mark - IO

extern	short					gHtab;
extern	short					gVtab;
extern	short					gDemoMode;
extern	Boolean					gAbortDemoFlag;
extern	Boolean					gGameIsDemoFlag;

#pragma mark - Misc

extern	short					gPrefsFolderVRefNum;
extern	long					gPrefsFolderDirID;
extern	Boolean					gGlobalFlagList[MAX_GLOBAL_FLAGS];

#pragma mark - Main

extern	PrefsType				gGamePrefs;
extern	long					gFrames;
extern	Byte					gSceneNum;
extern	Byte					gAreaNum;
extern	Byte					gPlayerMode;
extern	Byte					gCurrentPlayer;
extern	Byte					gDifficultySetting;			// set at start of game
extern	short					gLoadOldGameNum;
extern	Boolean					gLoadOldGameFlag;
extern	Byte					gStartingScene;
extern	Byte					gStartingArea;
extern	MikeFixed				gTweenFrameFactor;			// progress from previous frame to current frame
extern	MikeFixed				gOneMinusTweenFrameFactor;	// 1 - gTweenFrameFactor
extern	short					gMainAppRezFile;
extern	long					someLong;
extern	Boolean					gFinishedArea;
extern	Boolean					gScreenScrollFlag;

#pragma mark - Main.cpp

extern	struct SDL_Window		*gSDLWindow;
extern	struct SDL_Renderer		*gSDLRenderer;
extern	struct SDL_Texture		*gSDLTexture;
extern	FSSpec					gDataSpec;
extern	int						gNumThreads;

#pragma mark - MyGuy

extern	short					gMyInitX;
extern	short					gMyInitY;
extern	long					gMyX;
extern	long					gMyY;
extern	long					gMySumDX;
extern	long					gMySumDY;
extern	long					gMyDX;
extern	long					gMyDY;
extern	short					gMyDirection;
extern	Byte					gMyMode;
extern	long					gMyNormalMaxSpeed;
extern	long					gMyMaxSpeedX;
extern	long					gMyMaxSpeedY;
extern	long					gMyAcceleration;
extern	Boolean					gMeOnIceFlag;
extern	Boolean					gMeOnWaterFlag;
extern	long					gMyWindDX;
extern	long					gMyWindDY;
extern	Boolean					gMyKeys[6];
extern	short					gLastNonDeathX;
extern	short					gLastNonDeathY;
extern	short					gMyBlinkieTimer;
extern	short					gSpeedyTimer;
extern	Boolean					gFrogFlag;
extern	Boolean					gSpaceShipFlag;

#pragma mark - ObjectManager

extern	long					NumObjects;
extern	ObjNode					*gThisNodePtr;
extern	ObjNode					*gMyNodePtr;
extern	ObjNode					*ObjectList;
extern	ObjNode					*FirstNodePtr;
extern	ObjNode					*gMostRecentlyAddedNode;
extern	ObjNode					*FreeNodeStack[MAX_OBJECTS];
extern	long					NodeStackFront;
extern	long					gRightSide;
extern	long					gLeftSide;
extern	long					gTopSide;
extern	long					gBottomSide;
extern	Boolean					gDiscreteMovementFlag;		// prevent movement interpolation on current node (applied in UpdateObject)
extern	MikeFixed				gX;
extern	MikeFixed				gY;
extern	long					gDX;
extern	long					gDY;
extern	long					gSumDX;
extern	long					gSumDY;
extern	long					gRegionClipTop[MAX_CLIP_REGIONS];
extern	long					gRegionClipBottom[MAX_CLIP_REGIONS];
extern	long					gRegionClipLeft[MAX_CLIP_REGIONS];
extern	long					gRegionClipRight[MAX_CLIP_REGIONS];

#pragma mark - Palette

extern	GamePalette				gGamePalette;
extern	Boolean					gScreenBlankedFlag;

#pragma mark - Playfield

extern	long					PF_TILE_HEIGHT;
extern	long					PF_TILE_WIDTH;
extern	long					PF_WINDOW_TOP;
extern	long					PF_WINDOW_LEFT;
extern	short					gPlayfieldWidth;
extern	short					gPlayfieldHeight;
extern	Handle					gPlayfieldHandle;
extern	uint16_t				**gPlayfield;
extern	long					gScrollX;
extern	long					gScrollY;
extern	long					gTweenedScrollX;
extern	long					gTweenedScrollY;
extern	short					gItemDeleteWindow_Bottom;
extern	short					gItemDeleteWindow_Top;
extern	short					gItemDeleteWindow_Left;
extern	short					gItemDeleteWindow_Right;
extern	short					gNumItems;
extern	ObjectEntryType			*gMasterItemList;
extern	struct TileAttribType	*gTileAttributes;

#pragma mark - Shape

extern	Ptr						gSHAPE_HEADER_Ptrs[MAX_SHAPE_GROUPS][MAX_SHAPES_IN_FILE];
extern	Handle					gShapeTableHandle[MAX_SHAPE_GROUPS];
extern	ObjNode					*gMostRecentShape;

#pragma mark - Sound

extern	short					gSoundNum_UngaBunga;
extern	short					gSoundNum_DinoBoom;
extern	short					gSoundNum_DoorOpen;
extern	short					gSoundNum_BarneyJump;
extern	short					gSoundNum_DogRoar;
extern	short					gSoundNum_ChocoBunny;
extern	short					gSoundNum_Carmel;
extern	short					gSoundNum_GummyHaha;
extern	short					gSoundNum_JackInTheBox;
extern	short					gSoundNum_WitchHaha;
extern	short					gSoundNum_Skid;
extern	short					gSoundNum_Shriek;
extern	short					gSoundNum_Ship;
extern	short					gSoundNum_ExitShip;
extern	short					gSoundNum_Frog;
extern	short					gSoundNum_RobotDanger;
extern	short					gSoundNum_ClownLaugh;

#pragma mark - Triggers

extern	Boolean					gTeleportingFlag;

#pragma mark - Weapon

extern	WeaponType				gMyWeapons[MAX_WEAPONS];
extern	Byte					gCurrentWeaponType;
extern	Byte					gNumWeaponsIHave;
extern	Byte					gCurrentWeaponIndex;
extern	Byte					gBonusWeaponStartScenes[];
extern	Byte					gNumBullets;
extern	long					gLastRocketTime;
extern	long					gLastPixieTime;

#pragma mark - Window

extern	int						VISIBLE_WIDTH;
extern	int						VISIBLE_HEIGHT;
extern	uint8_t					*gIndexedFramebuffer;
extern	uint8_t					*gRGBAFramebuffer;
extern	uint8_t					*gRGBAFramebufferX2;
extern	uint8_t					**gScreenLookUpTable;		// VISIBLE_HEIGHT elements
extern	uint8_t					**gOffScreenLookUpTable;	// OFFSCREEN_HEIGHT elements
extern	uint8_t					**gBackgroundLookUpTable;	// OFFSCREEN_HEIGHT elements
extern	Ptr						*gPFLookUpTable;
extern	Ptr						*gPFCopyLookUpTable;
extern	Ptr						*gPFMaskLookUpTable;
extern	long					gScreenXOffset;				// global centering offset applied to sprites
extern	long					gScreenYOffset;				// global centering offset applied to sprites
extern	Handle					gBackgroundHandle;
extern	Handle					gOffScreenHandle;
extern	Handle					gPFBufferHandle;
extern	uint8_t					*gRowDitherStrides;			// for dithering filter
