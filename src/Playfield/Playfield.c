/****************************/
/*   PLAYFIELD ROUTINES     */
/* (c)1994 Pangea Software  */
/*   By Brian Greenstone    */
/****************************/


/***************/
/* EXTERNALS   */
/***************/

#include "myglobals.h"
//#include <pictutils.h>
//#include <palettes.h>
//#include <gestalt.h>
#include "window.h"
#include "playfield.h"
#include "object.h"
#include "enemy.h"
#include "traps.h"
#include "triggers.h"
#include "bonus.h"
#include "misc.h"
#include "miscanims.h"
#include "objecttypes.h"
#include "weapon.h"
#include "enemy2.h"
#include "enemy3.h"
#include "enemy4.h"
#include "enemy5.h"
#include "racecar.h"

extern	Handle			gOffScreenHandle;
extern	uint8_t*		gScreenLookUpTable[VISIBLE_HEIGHT];
extern	long			*gPFMaskLookUpTable;
extern	long			*gPFLookUpTable,*gPFCopyLookUpTable;
extern	Boolean	(*gItemAddPtrs[])(ObjectEntryType *);
extern	long				gMyX,gMyY;
extern	long				gRegionClipTop[],gRegionClipBottom[],gRegionClipLeft[],gRegionClipRight[];
extern	union_gX;
extern	union_gY;
extern	short			gMyDirection;
extern	ObjNode			*gThisNodePtr,*gMyNodePtr;
extern	char  			gMMUMode;
extern	long			gScreenRowOffsetLW,gScreenRowOffset;
extern	unsigned char	gInterlaceMode;
extern	long			gDX,gDY,gSumDX,gSumDY;
extern	Boolean			gTeleportingFlag;
extern	long			gMySumDX,gMySumDY,gMyDX,gMyDY;
extern	Boolean			gIsPPC603_604;


static void DisplayPlayfield603_604(void);

/****************************/
/*    CONSTANTS             */
/****************************/

											// FOR PPC, THESE ARE VARS SO WE CAN CHANGE THEM!
#if __USE_PF_VARS
long	PF_TILE_HEIGHT	=	13;				// dimensions of scrolling Playfield
long	PF_TILE_WIDTH	=	14;
long	PF_WINDOW_TOP	=	45;
long	PF_WINDOW_LEFT	=	24;				// left MUST be on 4 pixel boundary!!!!!
#endif

Boolean	gPPCFullScreenFlag = false;

#define	ITEM_WINDOW_RIGHT		5				// # tiles for item add window
#define	ITEM_WINDOW_LEFT		5
#define	ITEM_WINDOW_TOP			5
#define	ITEM_WINDOW_BOTTOM		5
#define	OUTER_SIZE				7				// size of border out of add window for delete window


#define SCROLL_WINDOW_XMARGIN	((PF_WINDOW_WIDTH/2)-40)
#define SCROLL_WINDOW_YMARGIN	((PF_WINDOW_HEIGHT/2)-40)
#define	SCROLL_WINDOW_LEFT		SCROLL_WINDOW_XMARGIN				// window for myguy's scroll movement
#define	SCROLL_WINDOW_RIGHT		(PF_WINDOW_WIDTH-SCROLL_WINDOW_XMARGIN)
#define	SCROLL_WINDOW_TOP		(SCROLL_WINDOW_YMARGIN)
#define	SCROLL_WINDOW_BOTTOM	(PF_WINDOW_HEIGHT-SCROLL_WINDOW_YMARGIN)

#define	SCROLL_BORDER			(TILE_SIZE*2L)					// 2 tile border around entire map

#define	VIEW_FACTOR		100				// amount to shift view for look-space

#define	MAX_PLAYFIELD_WIDTH	1000L		// max tiles wide the PF will ever be

#define	MAX_TILE_ANIMS	50						// max # of tile anims




#define	ILD(dest_,src_,width_)		\
			switch(width_)					\
			{								\
			case	80:						\
					dest_[79] = src_[79];	\
			case	79:						\
					dest_[78] = src_[78];	\
			case	78:						\
					dest_[77] = src_[77];	\
			case	77:						\
					dest_[76] = src_[76];	\
			case	76:						\
					dest_[75] = src_[75];	\
			case	75:						\
					dest_[74] = src_[74];	\
			case	74:						\
					dest_[73] = src_[73];	\
			case	73:						\
					dest_[72] = src_[72];	\
			case	72:						\
					dest_[71] = src_[71];	\
			case	71:						\
					dest_[70] = src_[70];	\
			case	70:						\
					dest_[69] = src_[69];	\
			case	69:						\
					dest_[68] = src_[68];	\
			case	68:						\
					dest_[67] = src_[67];	\
			case	67:						\
					dest_[66] = src_[66];	\
			case	66:						\
					dest_[65] = src_[65];	\
			case	65:						\
					dest_[64] = src_[64];	\
			case	64:						\
					dest_[63] = src_[63];	\
			case	63:						\
					dest_[62] = src_[62];	\
			case	62:						\
					dest_[61] = src_[61];	\
			case	61:						\
					dest_[60] = src_[60];	\
			case	60:						\
					dest_[59] = src_[59];	\
			case	59:						\
					dest_[58] = src_[58];	\
			case	58:						\
					dest_[57] = src_[57];	\
			case	57:						\
					dest_[56] = src_[56];	\
			case	56:						\
					dest_[55] = src_[55];	\
			case	55:						\
					dest_[54] = src_[54];	\
			case	54:						\
					dest_[53] = src_[53];	\
			case	53:						\
					dest_[52] = src_[52];	\
			case	52:						\
					dest_[51] = src_[51];	\
			case	51:						\
					dest_[50] = src_[50];	\
			case	50:						\
					dest_[49] = src_[49];	\
			case	49:						\
					dest_[48] = src_[48];	\
			case	48:						\
					dest_[47] = src_[47];	\
			case	47:						\
					dest_[46] = src_[46];	\
			case	46:						\
					dest_[45] = src_[45];	\
			case	45:						\
					dest_[44] = src_[44];	\
			case	44:						\
					dest_[43] = src_[43];	\
			case	43:						\
					dest_[42] = src_[42];	\
			case	42:						\
					dest_[41] = src_[41];	\
			case	41:						\
					dest_[40] = src_[40];	\
			case	40:						\
					dest_[39] = src_[39];	\
			case	39:						\
					dest_[38] = src_[38];	\
			case	38:						\
					dest_[37] = src_[37];	\
			case	37:						\
					dest_[36] = src_[36];	\
			case	36:						\
					dest_[35] = src_[35];	\
			case	35:						\
					dest_[34] = src_[34];	\
			case	34:						\
					dest_[33] = src_[33];	\
			case	33:						\
					dest_[32] = src_[32];	\
			case	32:						\
					dest_[31] = src_[31];	\
			case	31:						\
					dest_[30] = src_[30];	\
			case	30:						\
					dest_[29] = src_[29];	\
			case	29:						\
					dest_[28] = src_[28];	\
			case	28:						\
					dest_[27] = src_[27];	\
			case	27:						\
					dest_[26] = src_[26];	\
			case	26:						\
					dest_[25] = src_[25];	\
			case	25:						\
					dest_[24] = src_[24];	\
			case	24:						\
					dest_[23] = src_[23];	\
			case	23:						\
					dest_[22] = src_[22];	\
			case	22:						\
					dest_[21] = src_[21];	\
			case	21:						\
					dest_[20] = src_[20];	\
			case	20:						\
					dest_[19] = src_[19];	\
			case	19:						\
					dest_[18] = src_[18];	\
			case	18:						\
					dest_[17] = src_[17];	\
			case	17:						\
					dest_[16] = src_[16];	\
			case	16:						\
					dest_[15] = src_[15];	\
			case	15:						\
					dest_[14] = src_[14];	\
			case	14:						\
					dest_[13] = src_[13];	\
			case	13:						\
					dest_[12] = src_[12];	\
			case	12:						\
					dest_[11] = src_[11];	\
			case	11:						\
					dest_[10] = src_[10];	\
			case	10:						\
					dest_[9] = src_[9];	\
			case	9:						\
					dest_[8] = src_[8];	\
			case	8:						\
					dest_[7] = src_[7];	\
			case	7:						\
					dest_[6] = src_[6];	\
			case	6:						\
					dest_[5] = src_[5];	\
			case	5:						\
					dest_[4] = src_[4];	\
			case	4:						\
					dest_[3] = src_[3];	\
			case	3:						\
					dest_[2] = src_[2];	\
			case	2:						\
					dest_[1] = src_[1];	\
			case	1:						\
					dest_[0] = src_[0];	\
			}							\
			dest_ += width_;	\
			src_ += width_;



/**********************/
/*     VARIABLES      */
/**********************/

static	Handle			gTileSetHandle = nil;
static	Ptr				gTilesPtr;
static	short			*gTileXlatePtr;

Handle			gPlayfieldHandle = nil;
unsigned short	**gPlayfield = nil;
short			gPlayfieldTileWidth,gPlayfieldTileHeight;
short			gPlayfieldWidth,gPlayfieldHeight;

static	Byte	**gAlternateMap = nil;

long			gScrollX,gScrollY;
long			gScrollRow,gScrollCol,gOldScrollRow,gOldScrollCol;

short			gNumItems = -1;
static	ObjectEntryType	**gItemLookupTableX = nil;
ObjectEntryType *gMasterItemList = nil;

TileAttribType	*gTileAttributes;

short			gItemDeleteWindow_Bottom,gItemDeleteWindow_Top,gItemDeleteWindow_Left,gItemDeleteWindow_Right;

static	Rect			gViewWindow,gTargetViewWindow;

static	Boolean			gColorMaskArray[256];							// array of xparent tile colors, false = xparent

static	Boolean			gAltMapFlag = false;

static	short			gShakeyScreenCount;

static	Ptr				gMaxItemAddress;								// addr of last item in current item list

// Source port note: moved from TileAnim.c
static	short			gNumTileAnims;
static	TileAnimEntryType	gTileAnims[MAX_TILE_ANIMS];


/**********************/
/*     TABLES         */
/**********************/

#define	MAX_ITEM_NUM	55							// for error checking!
Boolean	(*gItemAddPtrs[])(ObjectEntryType *) = {
					AddEnemy_Caveman,
					AddAppearZone,
					NilAdd,						//store
					AddBunny,
					AddEnemy_Triceratops,
					AddEnemy_Turtle,
					AddManEatingPlant,
					AddDinoEgg,
					AddEnemy_BabyDino,
					AddEnemy_Rex,
					NilAdd,						//AddEnemy_ClownBalloon,
					AddEnemy_ClownCar,
					AddJackInTheBox,
					AddEnemy_Clown,
					AddMagicHat,
					AddHealthPOW,
					AddEnemy_FlowerClown,
					AddTeleport,
					AddRaceCar,
					AddKey,
					AddClowndoor,
					AddCandyMPlatform,
					AddCandyDoor,
					AddStar,
					AddEnemy_ChocBunny,
					AddEnemy_GBread,
					AddEnemy_Mint,
					nil,								// was cherrybomb
					AddEnemy_GBear,
					nil,								// my guy init coords
					nil,								// AddFinishLine,
					AddJurassicDoor,
					AddEnemy_Carmel,
					AddWeaponPowerup,
					AddMiscPowerup,
					AddGumBall,
					AddEnemy_LemonDrop,
					AddEnemy_Giant,
					AddEnemy_Dragon,
					AddEnemy_Witch,
					AddEnemy_BBWolf,
					AddEnemy_Soldier,
					AddMuffit,
					AddEnemy_Spider,
					AddFairyDoor,
					AddEnemy_Battery,
					AddPoisonApple,
					AddEnemy_Slinky,
					AddEnemy_8Ball,
					AddShipPOW,
					AddEnemy_Robot,
					AddEnemy_Doggy,
					AddBargainDoor,
					AddEnemy_Top,
					AddHydrant,
					AddKeyColor
				};


/****************** CLEAR TILE COLOR MASKS ***************/
//
// Initializes the array of flags which indicate whether a
// color is xparent or not with prioritized sprites.
//

void ClearTileColorMasks(void)
{
long		i;

	for (i=0; i < 256; i++)
	{
		gColorMaskArray[i] = true;				// assume nothing is xparent
	}
}


/********************* LOAD TILESET **********************/
//
// Loads a tileset into memory & does all initialization.
//


void LoadTileSet(Str255 fileName)
{
Ptr	tileSetPtr					= nil;
Ptr tileAnimList				= nil;
int16_t* tileXparentList		= nil;

	ClearTileColorMasks();									// clear this to begin with

	if (gTileSetHandle != nil)								// see if zap old tileset
		DisposeHandle(gTileSetHandle);

	gTileSetHandle = LoadPackedFile(fileName);				// load the file
	tileSetPtr = *gTileSetHandle;							// get fixed ptr

			/* GET OFFSETS */

	int offsetToTileDefinitions			= Byteswap32Signed(tileSetPtr+6)+2;		// base + offset + 2 (skip # tiles word)
	int offsetToXlateTable				= Byteswap32Signed(tileSetPtr+10)+2;	// base + offset + 2 (skip # entries word)
	int offsetToTileAttributes			= Byteswap32Signed(tileSetPtr+14)+2;	// base + offset + 2 (skip # entries word)
	int offsetToTileAnimList			= Byteswap32Signed(tileSetPtr+22)+2;
	int offsetToTileXparentColorList	= Byteswap32Signed(tileSetPtr+26)+2;

	GAME_ASSERT(offsetToTileDefinitions	< offsetToXlateTable);
	GAME_ASSERT(offsetToXlateTable		< offsetToTileAttributes);
	GAME_ASSERT(offsetToTileAttributes	< offsetToTileAnimList);
	GAME_ASSERT(offsetToTileAnimList	< offsetToTileXparentColorList);

			/* GET ENTRY COUNTS */

	int numTileDefinitions				= Byteswap16Signed(tileSetPtr + offsetToTileDefinitions			- 2	);
	int numXlateEntries					= Byteswap16Signed(tileSetPtr + offsetToXlateTable				- 2	);
	int numTileAttributeEntries			= Byteswap16Signed(tileSetPtr + offsetToTileAttributes			- 2	);
	gNumTileAnims						= Byteswap16Signed(tileSetPtr + offsetToTileAnimList			- 2	);
	int numTileXparentColors			= Byteswap16Signed(tileSetPtr + offsetToTileXparentColorList	- 2	);

			/* GET POINTERS TO TABLES */

	gTilesPtr			=						(	tileSetPtr + offsetToTileDefinitions		);
	gTileXlatePtr		=	(int16_t *)			(	tileSetPtr + offsetToXlateTable				);
	gTileAttributes		=	(TileAttribType *)	(	tileSetPtr + offsetToTileAttributes			);
	tileAnimList		=						(	tileSetPtr + offsetToTileAnimList			);
	tileXparentList		=	(int16_t *)			(	tileSetPtr + offsetToTileXparentColorList	);

			/* BYTESWAP STUFF */

	// Byteswap gTileXlatePtr
	ByteswapInts(2, numXlateEntries, gTileXlatePtr);

	// Byteswap gTileAttributes
	ByteswapStructs("Hh4b", sizeof(TileAttribType), numTileAttributeEntries, gTileAttributes);

	// Byteswap tileXparentList
	ByteswapInts(2, numTileXparentColors, tileXparentList);

	/***************** PREPARE TILE ANIMS ***********************/
	//
	// Source port note: moved from TileAnim.c
	//

	GAME_ASSERT(gNumTileAnims >= 0);
	GAME_ASSERT(gNumTileAnims <= MAX_TILE_ANIMS);
	Ptr currentTileAnimData = tileAnimList;
	for (int i = 0; i < gNumTileAnims; i++)
	{
		GAME_ASSERT(HandleBoundsCheck(gTileSetHandle, currentTileAnimData));

		char name[16];
		uint8_t nameLength = *currentTileAnimData;
		GAME_ASSERT(nameLength <= 15);
		BlockMove(currentTileAnimData+1, name, nameLength);
		name[nameLength] = '\0';

		TileAnimDefType* tileAnimDef = (TileAnimDefType*) (currentTileAnimData + 16);
		ByteswapInts(2, 3, currentTileAnimData+16);

		printf("PrepareTileAnims #%d: \"%s\", %d frames\n", i, name, tileAnimDef->numFrames);

		// Set tile anim
		gTileAnims[i].count = 0;
		gTileAnims[i].index = 0;
		gTileAnims[i].defPtr = tileAnimDef;

		// Advance pointer to next tile anim data
		currentTileAnimData += 16 + 2*3 + 2*tileAnimDef->numFrames;
	}


	/******************** SET TILE COLOR MASKS *********************/
	//
	// Activates color xparency on certain colors in palette.
	//

	for (int i = 0; i < numTileXparentColors; i++)
	{
		GAME_ASSERT(HandleBoundsCheck(gTileSetHandle, (Ptr) &tileXparentList[i]));
		GAME_ASSERT(tileXparentList[i] >= 0);
		GAME_ASSERT(tileXparentList[i] < sizeof(gColorMaskArray));

		gColorMaskArray[tileXparentList[i]] = false;
	}
}


/******************** DISPOSE CURRENT MAP DATA *********************/
//
// Disposes of all playfield related data ptrs and handles
//

void DisposeCurrentMapData(void)
{
	if (gPlayfieldHandle != nil)					// see if zap old playfield
	{
		DisposeHandle(gPlayfieldHandle);
		gPlayfieldHandle = nil;
	}

	if (gPlayfield != nil)
	{
		DisposePtr((Ptr)gPlayfield);
		gPlayfield = nil;
	}

	if (gAlternateMap != nil)
	{
		DisposePtr((Ptr)gAlternateMap);
		gAlternateMap = nil;
	}


	if (gTileSetHandle != nil)						// see if zap old tileset
	{
		DisposeHandle(gTileSetHandle);
		gTileSetHandle = nil;
	}

	gNumItems = -1;
	gMasterItemList = nil;	// this is just a pointer within gPlayfieldHandle, no need to dispose of it

}


/************************ LOAD PLAYFIELD *************************/
//
// NOTE: Assumes that previous playfield data has already been deleted
//

void LoadPlayfield(Str255 fileName)
{
uint16_t	*tempPtr;
long	i;
Ptr		bytePtr,pfPtr;

	gPlayfieldHandle = LoadPackedFile(fileName);					// load the file

	pfPtr = *gPlayfieldHandle;										// get fixed ptr


				/* BUILD MAP ARRAY */

	tempPtr = (uint16_t *)(pfPtr + Byteswap32Signed(pfPtr+2));		// point to MAP_IMAGE
	gPlayfieldTileWidth = Byteswap16(tempPtr++);					// get dimensions
	gPlayfieldTileHeight = Byteswap16(tempPtr++);
	gPlayfieldWidth = gPlayfieldTileWidth<<TILE_SIZE_SH;
	gPlayfieldHeight = gPlayfieldTileHeight<<TILE_SIZE_SH;

	gPlayfield = (uint16_t **)AllocPtr(sizeof(uint16_t *) * gPlayfieldTileHeight);	// alloc memory for 1st dimension of matrix
	if (gPlayfield == nil)
		DoFatalAlert("NewPtr failed trying to get gPlayfield!");
	for (i = 0; i < gPlayfieldTileHeight; i++)						// build 1st dimension of matrix
	{
		ByteswapInts(2, gPlayfieldTileWidth, tempPtr);
		gPlayfield[i]= (unsigned short *)tempPtr;					// set pointer to row
		tempPtr += gPlayfieldTileWidth;								// next row
	}


			/* GET ALTERNATE MAP */

	bytePtr = (Ptr)(pfPtr + Byteswap32Signed(pfPtr+10));						// point to ALTERNATE_MAP
	if (bytePtr == nil)
	{
		gAlternateMap = nil;										// no alt map to load
		gAltMapFlag = false;
	}
	else
	{
		gAlternateMap = (Byte **)AllocPtr(sizeof(Byte *) * gPlayfieldTileHeight);	// alloc memory for 1st dimension of matrix
		if (gPlayfield == nil)
			DoFatalAlert("NewPtr failed trying to get gAlternateMap!");
		for (i = 0; i < gPlayfieldTileHeight; i++)					// build matrix
		{
			gAlternateMap[i]= (Byte *)bytePtr;						// set pointer to row
			bytePtr += gPlayfieldTileWidth;							// next row
		}
		gAltMapFlag = true;
	}

	gScrollX = 0;													// default these
	gScrollY = 0;
}


/*************** INIT PLAYFIELD *******************/
//
// Draws entire playfield @ current scroll coords
// into playfield buffer
//

void InitPlayfield(void)
{
long		row,col,x,y,col2;
long		right,left,top,bottom;

				/* INIT PLAYFIELD CLIPPING REGION */

	gRegionClipTop[CLIP_REGION_PLAYFIELD] = OFFSCREEN_WINDOW_TOP+PF_WINDOW_TOP;
	gRegionClipBottom[CLIP_REGION_PLAYFIELD] = OFFSCREEN_WINDOW_TOP+PF_WINDOW_TOP+PF_WINDOW_HEIGHT;
	gRegionClipLeft[CLIP_REGION_PLAYFIELD] = OFFSCREEN_WINDOW_LEFT+PF_WINDOW_LEFT;
	gRegionClipRight[CLIP_REGION_PLAYFIELD] = OFFSCREEN_WINDOW_LEFT+PF_WINDOW_LEFT+PF_WINDOW_WIDTH+70;


					/* INIT VIEW WINDOW */

	gViewWindow.left = gTargetViewWindow.left = SCROLL_WINDOW_LEFT;
	gViewWindow.right = gTargetViewWindow.right = SCROLL_WINDOW_RIGHT;
	gViewWindow.top = gTargetViewWindow.top = SCROLL_WINDOW_TOP;
	gViewWindow.bottom = gTargetViewWindow.bottom = SCROLL_WINDOW_BOTTOM;

	gShakeyScreenCount = 0;										// init shakey screen

				/* VERIFY INITIAL SCROLL POSITION */

	gScrollX = gMyX-(PF_WINDOW_WIDTH/2);						// attempt to center me on screen
	gScrollY = gMyY-(PF_WINDOW_HEIGHT/2);

	if (gScrollX < SCROLL_BORDER)
		gScrollX = SCROLL_BORDER;
	else
	if (gScrollX > ((gPlayfieldWidth)-(SCROLL_BORDER+PF_WINDOW_WIDTH)))
		gScrollX = (gPlayfieldWidth)-(SCROLL_BORDER+PF_WINDOW_WIDTH);

	if (gScrollY < SCROLL_BORDER)
		gScrollY = SCROLL_BORDER;
	else
	if (gScrollY > ((gPlayfieldHeight)-(SCROLL_BORDER+PF_WINDOW_HEIGHT)))
		gScrollY = (gPlayfieldHeight)-(SCROLL_BORDER+PF_WINDOW_HEIGHT);

//				/* BUILD ITEM LIST */
//
//	BuildItemList();

				/* DRAW INITIAL PLAYFIELD */

	gScrollRow = gOldScrollRow = gScrollY>>TILE_SIZE_SH;		// calc scroll tile row/col
	gScrollCol = gOldScrollCol = gScrollX>>TILE_SIZE_SH;

	row = gScrollRow % PF_TILE_HEIGHT;							// calc row in buffer
	col = gScrollCol % PF_TILE_WIDTH;							// calc col in buffer

	for (y = 0; y < PF_TILE_HEIGHT; y++)
	{
		col2 = col;
		for (x = 0; x < PF_TILE_WIDTH; x++)
		{
			DrawATile(gPlayfield[gScrollRow+y][gScrollCol+x],row,col2,true);

			if (++col2 >= PF_TILE_WIDTH)
				col2 = 0;
		}
		if (++row >= PF_TILE_HEIGHT)
			row = 0;
	}

				/* ADD ITEMS IN THIS AREA */

	SetItemDeleteWindow();

	right = gScrollCol+PF_TILE_WIDTH+ITEM_WINDOW_RIGHT;				// create area
	if (right >= gPlayfieldTileWidth)								// check bounds
		right = gPlayfieldTileWidth-1;
	left = gScrollCol-ITEM_WINDOW_LEFT;
	if (left < 0)
		left = 0;
	top = gScrollRow-ITEM_WINDOW_TOP;
	if (top < 0)
		top = 0;
	bottom = gScrollRow+PF_TILE_HEIGHT+ITEM_WINDOW_BOTTOM;
	if (bottom >= gPlayfieldTileHeight)
		bottom = gPlayfieldTileHeight-1;

	ScanForPlayfieldItems(top,bottom,left,right);				// scan for any items

}


/************************ BUILD ITEM LIST ***********************/
//
// Build sorted lists of playfield items
//

void BuildItemList(void)
{
long	offset;
long	col,itemCol,itemNum,nextCol,prevCol;
ObjectEntryType *lastPtr;

	if	(gPlayfieldTileWidth > MAX_PLAYFIELD_WIDTH)				// see if bigger than max allowed
		DoFatalAlert("gPlayfieldTileWidth is greater than MAX_PLAYFIELD_WIDTH!");

					/* GET BASIC INFO */

	offset = Byteswap32Signed(*gPlayfieldHandle+6);						// get offset to OBJECT_LIST
	gNumItems = Byteswap16Signed(*gPlayfieldHandle+offset);				// get # items in file
	if (gNumItems == 0)
		return;
	gMasterItemList = (ObjectEntryType *)(*gPlayfieldHandle+offset+2);	// point to items in file

					/* BYTESWAP ALL OBJECT ENTRY STRUCTS */

	// Ensure the in-memory representation of the struct is tightly-packed to match the struct's layout on disk
	_Static_assert(sizeof(struct ObjectEntryType) == 4+4+2+4, "ObjectEntryType has incorrect size!");

	ByteswapStructs("2ih4b", sizeof(ObjectEntryType), gNumItems, gMasterItemList);

				/* BUILD HORIZ LOOKUP TABLE */

	gMaxItemAddress = (Ptr)&gMasterItemList[gNumItems-1];		// remember addr of last item
	lastPtr = &gMasterItemList[0];
	nextCol = 0;												// start @ col 0
	prevCol = -1;
	for (itemNum = 0; itemNum < gNumItems; itemNum++)
	{
		itemCol = gMasterItemList[itemNum].x>>TILE_SIZE_SH;		// get column of item
		if (itemCol != prevCol)									// see if changed
		{
			for (col = nextCol; col <= itemCol; col++)			// filler pointers
				gItemLookupTableX[col] = &gMasterItemList[itemNum];
			prevCol = itemCol;
			nextCol = itemCol+1;
			lastPtr = &gMasterItemList[itemNum];
		}
	}
	for (col = nextCol; col < gPlayfieldTileWidth; col++)		// set trailing column pointers
		gItemLookupTableX[col] = lastPtr;

}


/************************** SCROLL PLAYFIELD ****************************/
//
// Scrolls playfield to current gScrollX/Y coords.
//
// NOTE: max scroll speed is TILE_SIZE pixels
//

void ScrollPlayfield(void)
{

	gOldScrollRow = gScrollRow;									// save old row/col
	gOldScrollCol = gScrollCol;

	gScrollCol = gScrollX>>TILE_SIZE_SH;						// get new row/col
	gScrollRow = gScrollY>>TILE_SIZE_SH;


			/* SEE IF SCROLLED A TILE VERTICALLY */

	if (gOldScrollRow != gScrollRow)
	{
		if (gScrollRow > gOldScrollRow)							// see which way
			ScrollPlayfield_Down();
		else
			ScrollPlayfield_Up();
	}

			/* SEE IF SCROLLED A TILE HORIZONTALLY */

	if (gOldScrollCol != gScrollCol)
	{
		if (gScrollCol > gOldScrollCol)							// see which way
			ScrollPlayfield_Right();
		else
			ScrollPlayfield_Left();
	}

			/* CALC ITEM OUTER BOUNDARY WINDOW */

	SetItemDeleteWindow();
}


/****************** SET ITEM DELETE WINDOW ****************/
//
// Sets coords of the window/zone out of which items should delete
//

void SetItemDeleteWindow(void)
{
	gItemDeleteWindow_Top = (gScrollRow-ITEM_WINDOW_TOP-OUTER_SIZE)<<TILE_SIZE_SH;
	gItemDeleteWindow_Bottom = (gScrollRow+PF_TILE_HEIGHT+ITEM_WINDOW_BOTTOM+OUTER_SIZE)<<TILE_SIZE_SH;
	gItemDeleteWindow_Left = (gScrollCol-ITEM_WINDOW_LEFT-OUTER_SIZE)<<TILE_SIZE_SH;
	gItemDeleteWindow_Right = (gScrollCol+PF_TILE_WIDTH+ITEM_WINDOW_RIGHT+OUTER_SIZE)<<TILE_SIZE_SH;
}

/****************** SCROLL PLAYFIELD: DOWN **********************/

void ScrollPlayfield_Down(void)
{
long	row,col,x,mapRow,right;

				/* UPDATE TILES */

	mapRow = gScrollRow+(PF_TILE_HEIGHT-1);						// calc row in map matrix

	row = mapRow % PF_TILE_HEIGHT;								// calc row in buffer
	col = gScrollCol % PF_TILE_WIDTH;							// calc col in buffer

	for (x = 0; x < PF_TILE_WIDTH; x++)
	{
		DrawATile(gPlayfield[mapRow][gScrollCol+x],row,col,true);

		if (++col >= PF_TILE_WIDTH)
			col = 0;
	}

				/* UPDATE ITEMS */

	x = gScrollCol-ITEM_WINDOW_LEFT;							// calc area to check for new items
	if (x < 0)													// see if out of bounds
		x = 0;
	right = gScrollCol+PF_TILE_WIDTH+ITEM_WINDOW_RIGHT;
	if (right >= gPlayfieldTileWidth)
		right = gPlayfieldTileWidth-1;
	mapRow = gScrollRow+PF_TILE_HEIGHT+ITEM_WINDOW_BOTTOM;
	if (mapRow >= gPlayfieldTileHeight)
		mapRow = gPlayfieldTileHeight-1;

	ScanForPlayfieldItems(mapRow,mapRow,x,right);				// scan for any items
}


/****************** SCROLL PLAYFIELD: UP **********************/

void ScrollPlayfield_Up(void)
{
long	row,col,x,right;

				/* UPDATE TILES */

	row = gScrollRow % PF_TILE_HEIGHT;								// calc row in buffer
	col = gScrollCol % PF_TILE_WIDTH;								// calc col in buffer

	for (x = 0; x < PF_TILE_WIDTH; x++)
	{
		DrawATile(gPlayfield[gScrollRow][gScrollCol+x],row,col,true);
		if (++col >= PF_TILE_WIDTH)
			col = 0;
	}


				/* UPDATE ITEMS */

	x = gScrollCol-ITEM_WINDOW_LEFT;							// calc area to check for new items
	if (x < 0)													// see if out of bounds
		x = 0;
	right = gScrollCol+PF_TILE_WIDTH+ITEM_WINDOW_RIGHT;
	if (right >= gPlayfieldTileWidth)
		right = gPlayfieldTileWidth-1;
	row = gScrollRow-ITEM_WINDOW_TOP;
	if (row < 0)
		row = 0;

	ScanForPlayfieldItems(row,row,x,right);						// scan for any items

}


/****************** SCROLL PLAYFIELD: RIGHT **********************/
//
// playfield actually scrolls to the left, but new data added on right side
//

void ScrollPlayfield_Right(void)
{
long	row,col,y,mapCol,mapRowTop,mapRowBot;

				/* UPDATE TILES */

	mapCol = gScrollCol+(PF_TILE_WIDTH-1);						// calc col in map matrix
	row = gScrollRow % PF_TILE_HEIGHT;							// calc row in buffer
	col = mapCol  % PF_TILE_WIDTH;								// calc col in buffer

	for (y = 0; y < PF_TILE_HEIGHT; y++)
	{
		DrawATile(gPlayfield[gScrollRow+y][mapCol],row,col,true);
		if (++row >= PF_TILE_HEIGHT)
			row = 0;
	}

				/* UPDATE ITEMS */

	mapCol = gScrollCol+PF_TILE_WIDTH+ITEM_WINDOW_RIGHT;		// calc column to check for new objects
	if (mapCol >= gPlayfieldTileWidth)								// see if out of bounds
		return;

	mapRowTop = gScrollRow-ITEM_WINDOW_TOP;						// calc top/bottom bounds
	if (mapRowTop < 0)
		mapRowTop = 0;
	mapRowBot = gScrollRow+PF_TILE_HEIGHT+ITEM_WINDOW_BOTTOM;
	if (mapRowBot >= gPlayfieldTileHeight)
		mapRowBot = gPlayfieldTileHeight-1;

	ScanForPlayfieldItems(mapRowTop,mapRowBot,mapCol,mapCol);	// scan for any items

}

/****************** SCROLL PLAYFIELD: LEFT **********************/

void ScrollPlayfield_Left(void)
{
long	row,col,y,mapRowTop,mapRowBot,mapCol;

				/* UPDATE TILES */

	row = gScrollRow % PF_TILE_HEIGHT;							// calc row in buffer
	col = gScrollCol  % PF_TILE_WIDTH;								// calc col in buffer

	for (y = 0; y < PF_TILE_HEIGHT; y++)
	{
		DrawATile(gPlayfield[gScrollRow+y][gScrollCol],row,col,true);
		if (++row >= PF_TILE_HEIGHT)
			row = 0;
	}

				/* UPDATE ITEMS */

	mapCol = gScrollCol-ITEM_WINDOW_RIGHT;						// calc column to check for new objects
	if (mapCol < 0)												// see if out of bounds
		return;

	mapRowTop = gScrollRow-ITEM_WINDOW_TOP;						// calc top/bottom bounds
	if (mapRowTop < 0)
		mapRowTop = 0;
	mapRowBot = gScrollRow+PF_TILE_HEIGHT+ITEM_WINDOW_BOTTOM;
	if (mapRowBot >= gPlayfieldTileHeight)
		mapRowBot = gPlayfieldTileHeight-1;


	ScanForPlayfieldItems(mapRowTop,mapRowBot,mapCol,mapCol);	// scan for any items
}


/****************** SCAN FOR PLAYFIELD ITEMS *******************/
//
// Given this range, scan for items.  Coords are in row/col values.
//

void ScanForPlayfieldItems(long top, long bottom, long left, long right)
{
ObjectEntryType *itemPtr;
long	row,type;
Boolean		flag;

	if (gNumItems == 0)
		return;

	itemPtr = gItemLookupTableX[left];								// get pointer to 1st item at this X

	while (((itemPtr->x>>TILE_SIZE_SH) >= left) && ((itemPtr->x>>TILE_SIZE_SH) <= right))	// check all items in this column range
	{
		row = itemPtr->y>>TILE_SIZE_SH;
		if ((row >= top) && (row <= bottom))						// & this row range
		{
					/* ADD AN ITEM */

			if (!(itemPtr->type&ITEM_IN_USE))						// see if item available
			{
				type = itemPtr->type&ITEM_NUM;						// mask out status bits 15..12
				if (type > MAX_ITEM_NUM)							// error check!
					DoFatalAlert("Illegal Map Item Type!");
				else
				{
					flag = gItemAddPtrs[type](itemPtr);				// call item's ADD routine
					if (flag)
						itemPtr->type |= ITEM_IN_USE;				// set in-use flag
				}
			}
		}
		itemPtr++;													// point to next item
		if ((long)itemPtr > (long)gMaxItemAddress)					// see if its past the last address
			break;
	}
}


/************************ DO MY SCREEN SCROLL ************************/
//
// Update screen scroll based on MyGuy's position
//

void DoMyScreenScroll(void)
{
long	screenX,screenY;
long	scrollDX,scrollDY;

	UpdateViewWindow();

	screenX = gMyNodePtr->X.Int-gScrollX;					// calc screen coords
	screenY = gMyNodePtr->Y.Int-gScrollY;

	scrollDX = scrollDY = 0;								// assume no scroll

	if (screenX < gViewWindow.left)							// see if scroll to left
	{
		scrollDX = -(gViewWindow.left-screenX);
	}
	else
	if (screenX > gViewWindow.right)						// see if scroll to right
	{
		scrollDX = screenX-gViewWindow.right;
	}

	if (screenY < gViewWindow.top)							// see if scroll up
	{
		scrollDY = -(gViewWindow.top-screenY);
	}
	else
	if (screenY > gViewWindow.bottom)						// see if scroll down
	{
		scrollDY = screenY-gViewWindow.bottom;
	}


	if (scrollDX > (TILE_SIZE-1))								// limit scroll speed
		scrollDX = (TILE_SIZE-1);
	else
	if (scrollDX < -(TILE_SIZE-1))
		scrollDX = -(TILE_SIZE-1);

	if (scrollDY > (TILE_SIZE-1))
		scrollDY = (TILE_SIZE-1);
	else
	if (scrollDY < -(TILE_SIZE-1))
		scrollDY = -(TILE_SIZE-1);


	gScrollX += scrollDX;									// move it
	gScrollY += scrollDY;

	if (gScrollX < SCROLL_BORDER)							// check edges
	{
		gScrollX = SCROLL_BORDER;
		scrollDX = 0;
	}
	else
	if (gScrollX > ((gPlayfieldWidth)-(SCROLL_BORDER+PF_WINDOW_WIDTH)))
	{
		gScrollX = (gPlayfieldWidth)-(SCROLL_BORDER+PF_WINDOW_WIDTH);
		scrollDX = 0;
	}

	if (gScrollY < SCROLL_BORDER)
	{
		gScrollY = SCROLL_BORDER;
		scrollDY = 0;
	}
	else
	if (gScrollY > ((gPlayfieldHeight)-(SCROLL_BORDER+PF_WINDOW_HEIGHT)))
	{
		gScrollY = (gPlayfieldHeight)-(SCROLL_BORDER+PF_WINDOW_HEIGHT);
		scrollDY = 0;
	}

	if (!(scrollDX || scrollDY))							// stop teleporting when screen stops
		gTeleportingFlag = false;
}


/******************** UPDATE VIEW WINDOW ***********************/
//
// Moves the view window depending on MyGuy's direction
//

void UpdateViewWindow(void)
{
long	dir;

	dir = gMyDirection;

	if (!(gMyDX | gMyDY))											// if Im not moving, but Im "moving" then rethink how to aim me
	{
		if (gMySumDX | gMySumDY)
		{
			if (gMySumDY < 0)										// see if going up
				dir = AIM_UP;
			else
			if (gMySumDY > 0)										// see if going down
				dir = AIM_DOWN;
			else
			if (gMySumDX < 0)										// see if going left
				dir = AIM_LEFT;
			else
			if (gMySumDX > 0)										// see if going right
				dir = AIM_RIGHT;
		}
	}

				/* CHECK HORIZ SIDES */

	if ((dir == AIM_UP) || (dir == AIM_DOWN))						// if vertical, then center horiz
	{
		gTargetViewWindow.left = SCROLL_WINDOW_LEFT;
		gTargetViewWindow.right = SCROLL_WINDOW_RIGHT;
	}
	else
	if ((dir > AIM_UP) && (dir < AIM_DOWN))							// see if aiming right
	{
		gTargetViewWindow.left = SCROLL_WINDOW_LEFT-VIEW_FACTOR;
		gTargetViewWindow.right = SCROLL_WINDOW_RIGHT-VIEW_FACTOR;
	}
	else																// otherwise, aiming left
	{
		gTargetViewWindow.left = SCROLL_WINDOW_LEFT+VIEW_FACTOR;
		gTargetViewWindow.right = SCROLL_WINDOW_RIGHT+VIEW_FACTOR;
	}


				/* CHECK VERTICAL SIDES */

	if ((dir == AIM_RIGHT) || (dir == AIM_LEFT))						// if horiz, then center vert
	{
		gTargetViewWindow.top = SCROLL_WINDOW_TOP;
		gTargetViewWindow.bottom = SCROLL_WINDOW_BOTTOM;
	}
	else
	if ((dir > AIM_RIGHT) && (dir < AIM_LEFT))							// see if aiming down
	{
		gTargetViewWindow.top = SCROLL_WINDOW_TOP-VIEW_FACTOR;
		gTargetViewWindow.bottom = SCROLL_WINDOW_BOTTOM-VIEW_FACTOR;
	}
	else																// otherwise, aiming up
	{
		gTargetViewWindow.top = SCROLL_WINDOW_TOP+VIEW_FACTOR;
		gTargetViewWindow.bottom = SCROLL_WINDOW_BOTTOM+VIEW_FACTOR;
	}


				/* MOVE VIEW TO TARGET VIEW */

	if (gViewWindow.top < gTargetViewWindow.top)
		gViewWindow.top += 4;
	else
	if (gViewWindow.top > gTargetViewWindow.top)
		gViewWindow.top -= 4;

	if (gViewWindow.bottom < gTargetViewWindow.bottom)
		gViewWindow.bottom += 4;
	else
	if (gViewWindow.bottom > gTargetViewWindow.bottom)
		gViewWindow.bottom -= 4;

	if (gViewWindow.left < gTargetViewWindow.left)
		gViewWindow.left += 4;
	else
	if (gViewWindow.left > gTargetViewWindow.left)
		gViewWindow.left -= 4;

	if (gViewWindow.right < gTargetViewWindow.right)
		gViewWindow.right += 4;
	else
	if (gViewWindow.right > gTargetViewWindow.right)
		gViewWindow.right -= 4;

}




/*********************** TEST COORDINATE RANGE ************************/
//
// Returns true if global X/Y coords are out of playfield item window
//

Boolean TestCoordinateRange(void)
{

	if ((gX.Int < gItemDeleteWindow_Left) || (gX.Int > gItemDeleteWindow_Right) ||
		(gY.Int < gItemDeleteWindow_Top) || (gY.Int > gItemDeleteWindow_Bottom))
		return(true);
	else
		return(false);
}


/*********************** TRACK ITEM ************************/
//
// Returns true if gThisNodePtr is out if range
//

Boolean TrackItem(void)
{

	if ((gThisNodePtr->X.Int < gItemDeleteWindow_Left) ||
		(gThisNodePtr->X.Int > gItemDeleteWindow_Right) ||
		(gThisNodePtr->Y.Int < gItemDeleteWindow_Top) ||
		(gThisNodePtr->Y.Int > gItemDeleteWindow_Bottom))
	{
		return(true);
	}
	else
		return(false);
}


/********************* GET ALTERNATE TILE INFO **********************/
//
// Returns the alternate tile # at the given coordinates
//

Byte GetAlternateTileInfo(unsigned short x, unsigned short y)
{
	if ((y >= gPlayfieldHeight) || (x >= gPlayfieldWidth))		// check for bounds error (automatically checks for <0)
		return 0;

	return(gAlternateMap[y>>TILE_SIZE_SH][x>>TILE_SIZE_SH]);
}


/********************* GET MAP TILE ATTRIBS **********************/
//
// Returns the attrib bits of the tile @ x,y
//

unsigned short GetMapTileAttribs(unsigned short x, unsigned short y)
{
	if ((y >= gPlayfieldHeight) || (x >= gPlayfieldWidth))		// check for bounds error  (automatically checks for <0)
		return(0);

	return gTileAttributes[gPlayfield[y>>TILE_SIZE_SH][x>>TILE_SIZE_SH]&TILENUM_MASK].bits;
}


/********************* GET FULL MAP TILE ATTRIBS **********************/
//
// Returns a pointer to ALL the attribs of the tile @ x,y
//
//

TileAttribType *GetFullMapTileAttribs(unsigned short x, unsigned short y)
{
	if ((y >= gPlayfieldHeight) || (x >= gPlayfieldWidth))		// check for bounds error  (automatically checks for <0)
		return(nil);

	return (&(gTileAttributes[gPlayfield[y>>TILE_SIZE_SH][x>>TILE_SIZE_SH]&TILENUM_MASK]));
}



/****************** START SHAKEY SCREEN ***********************/
//
// Gets the screen shaking for "duration" ticks
//

void StartShakeyScreen(short duration)
{
	if (gShakeyScreenCount < duration)			// only reset if new duration is longer than current
		gShakeyScreenCount = duration;
}


/******************** MOVE ON PATH *******************/
//
// Moves given object along the Alternate Map's path tiles
//
// INPUT: gTheNodePtr = ptr to current node
//		gDX/Y etc. all set
//
// OUTPUT: tile that we're on
//

short MoveOnPath(long	speed, Boolean useDefault)
{
Byte	tile;

	switch(tile = GetAlternateTileInfo(gX.Int, gY.Int))			// see which way to go
	{
		case	ALT_TILE_DIR_UP:
				gDY = -speed;
				gDX = 0;
				break;

		case	ALT_TILE_DIR_RIGHT:
				gDX = speed;
				gDY = 0;
				break;

		case	ALT_TILE_DIR_DOWN:
				gDY = speed;
				gDX = 0;
				break;

		case	ALT_TILE_DIR_LEFT:
				gDX = -speed;
				gDY = 0;
				break;

		default:
				if (useDefault)			// if default flag, then stop
				{
					gDX = 0;
					gDY = 0;
				}
	}

	gX.L += gDX;
	gY.L += gDY;

	return(tile);
}


/******************** NIL ADD ***********************/
//
// nothing add
//

Boolean NilAdd(ObjectEntryType *itemPtr)
{
	return(false);
}


/******************* CREATE PLAYFIELD PERMANENT MEMORY ***********************/
//
// Create playfield memory items that ARE NEVER PURGED.. EVER!
//

void CreatePlayfieldPermanentMemory(void)
{
Handle		tempHand;

				/* ALLOC MEMORY FOR ITEM LOOKUP TABLE */

	tempHand = AllocHandle(sizeof(ObjectEntryType *)*MAX_PLAYFIELD_WIDTH);
	if (tempHand == nil)
		DoFatalAlert("Couldnt alloc memory for gItemLookupTableX");
	HLockHi(tempHand);
	gItemLookupTableX = (ObjectEntryType **)*tempHand;
}

//=======================================================================================
//                  POWERPC CODE
//=======================================================================================

#if 0
/************************ DRAW A TILE ***********************/

static void DrawATile(unsigned short tileNum, short row, short col, Boolean maskFlag)
{
double		*destPtr,*srcPtr,*destCopyPtr;
long		height,i;
Ptr			destStartPtr,destCopyStartPtr;
double		*copyOfSrc;
unsigned long	rowS,colS;								// shifted version of row & col
Byte		pixel;
Ptr			destPtrB,srcPtrB;
static long	fillFs[2] = {0xFFFFFFFFL,0XFFFFFFFFL};
double		t1,t2,t3,t4;

					/* CALC DEST POINTERS */

	destStartPtr = (Ptr)(gPFLookUpTable[rowS = row<<TILE_SIZE_SH]+(colS = col<<TILE_SIZE_SH));
	destCopyStartPtr = (Ptr)(gPFCopyLookUpTable[rowS]+colS);

					/* CALC TILE DEFINITION ADDR */

	copyOfSrc = srcPtr = (double *)(gTilesPtr+
					((long)(gTileXlatePtr[tileNum&TILENUM_MASK])<<(TILE_SIZE_SH*2)));
	destPtr = (double *)destStartPtr;
	destCopyPtr = (double *)destCopyStartPtr;

						/* DRAW THE TILE */

	height = TILE_SIZE;
	do
	{

		t1 = srcPtr[0];
		t2 = srcPtr[1];
		t3 = srcPtr[2];
		t4 = srcPtr[3];

		destPtr[0] = t1;						// copy 8 pixels at a time
		destCopyPtr[0] = t1;
		destPtr[1] = t2;
		destCopyPtr[1] = t2;
		destPtr[2] = t3;
		destCopyPtr[2] = t3;
		destPtr[3] = t4;
		destCopyPtr[3] = t4;

		srcPtr += 4;
		destPtr += (PF_BUFFER_WIDTH-TILE_SIZE)/8+4;			// next line
		destCopyPtr += (PF_BUFFER_WIDTH-TILE_SIZE)/8+4;
	}while(--height);


					/************************/
					/* SEE IF DRAW THE MASK */
					/************************/

	if (maskFlag)
	{
		destPtr = (double *)(gPFMaskLookUpTable[rowS]+colS);
		if (tileNum&TILE_PRIORITY_MASK)
		{
			if (tileNum&TILE_PRIORITY_MASK2)						// see if do pixel accurate mask or just tile mask
			{

						/* DRAW PIXEL TILE MASK */

				destPtrB = (Ptr)destPtr;
				srcPtrB = (Ptr)copyOfSrc;
				height = TILE_SIZE;
				do
				{
					for (i=0; i < TILE_SIZE; i++)
					{
						pixel = *srcPtrB++;							// get pixel value

						if (gColorMaskArray[pixel])
							*destPtrB++ = 0xff;						// make xparent
						else
							*destPtrB++ = 0x00;						// make solid
					}
					destPtrB += PF_BUFFER_WIDTH-TILE_SIZE;			// next line
				} while(--height);

			}
							/* DRAW WHOLE TILE MASK */
			else
			{
				height = TILE_SIZE;
				t1 = *((double *)fillFs);
				do
				{
					destPtr[0] = t1;		// 0xffffffffffffffff
					destPtr[1] = t1;		// 0xffffffffffffffff
					destPtr[2] = t1;		// 0xffffffffffffffff
					destPtr[3] = t1;		// 0xffffffffffffffff
					destPtr += ((PF_BUFFER_WIDTH-TILE_SIZE)/8)+4;			// next line
				} while (--height);
			}
		}
		else
		{
						/* CLEAR MASK HERE */

			height = TILE_SIZE;
			t1 = 0;
			do
			{
				destPtr[0] = t1;
				destPtr[1] = t1;
				destPtr[2] = t1;
				destPtr[3] = t1;
				destPtr += ((PF_BUFFER_WIDTH-TILE_SIZE)/8)+4;			// next line
			} while (--height);
		}
	}
}

#else

/************************ DRAW A TILE ***********************/

void DrawATile(unsigned short tileNum, short row, short col, Boolean maskFlag)
{
unsigned char *destPtr,*srcPtr,*destCopyPtr;
long		height,i;
Ptr			destStartPtr,destCopyStartPtr;
unsigned char	*copyOfSrc;
unsigned long	rowS,colS;								// shifted version of row & col
Byte		pixel;
Ptr			destPtrB,srcPtrB;

					/* CALC DEST POINTERS */

	destStartPtr = (Ptr)(gPFLookUpTable[rowS = row<<TILE_SIZE_SH]+(colS = col<<TILE_SIZE_SH));
	destCopyStartPtr = (Ptr)(gPFCopyLookUpTable[rowS]+colS);

					/* CALC TILE DEFINITION ADDR */

	GAME_ASSERT(HandleBoundsCheck(gTileSetHandle, &gTileXlatePtr[tileNum & TILENUM_MASK]));

	int xlate = gTileXlatePtr[tileNum&TILENUM_MASK];

	copyOfSrc = srcPtr = (unsigned char *)(gTilesPtr + (xlate<<(TILE_SIZE_SH*2)));
	destPtr = (unsigned char *)destStartPtr;
	destCopyPtr = (unsigned char *)destCopyStartPtr;

						/* DRAW THE TILE */

	height = TILE_SIZE;
	do
	{
		for (i = 0; i < TILE_SIZE; i++)
			destPtr[i] = destCopyPtr[i] = *srcPtr++;

		destPtr += PF_BUFFER_WIDTH;							// next line
		destCopyPtr += PF_BUFFER_WIDTH;
	}while(--height);


					/************************/
					/* SEE IF DRAW THE MASK */
					/************************/

	if (maskFlag)
	{
		destPtr = (unsigned char *)(gPFMaskLookUpTable[rowS]+colS);
		if (tileNum&TILE_PRIORITY_MASK)
		{
			if (tileNum&TILE_PRIORITY_MASK2)						// see if do pixel accurate mask or just tile mask
			{

						/* DRAW PIXEL TILE MASK */

				destPtrB = (Ptr)destPtr;
				srcPtrB = (Ptr)copyOfSrc;
				height = TILE_SIZE;
				do
				{
					for (i=0; i < TILE_SIZE; i++)
					{
						pixel = *srcPtrB++;							// get pixel value

						if (gColorMaskArray[pixel])
							*destPtrB++ = 0xff;						// make xparent
						else
							*destPtrB++ = 0x00;						// make solid
					}
					destPtrB += PF_BUFFER_WIDTH-TILE_SIZE;			// next line
				} while(--height);

			}
							/* DRAW WHOLE TILE MASK */
			else
			{
				height = TILE_SIZE;
				do
				{
					for (i = 0; i < TILE_SIZE; i++)
						destPtr[i] = 0xff;
					destPtr += PF_BUFFER_WIDTH;			// next line
				} while (--height);
			}
		}
		else
		{
						/* CLEAR MASK HERE */

			height = TILE_SIZE;
			do
			{
				for (i = 0; i < TILE_SIZE; i++)
					destPtr[i] = 0;
				destPtr += PF_BUFFER_WIDTH;			// next line
			} while (--height);
		}
	}
}
#endif


#if 0
/************************ DRAW A TILE : SIMPLE ***********************/
//
// This simple version doesnt worry about masks at all.
// Assumes that tileNum has ALREADY BEEN FILTERED!
//

void DrawATile_Simple(unsigned short tileNum, short row, short col)
{
double		*destPtr,*srcPtr,*destCopyPtr;
long		height;
Ptr			destStartPtr,destCopyStartPtr;
unsigned long	rowS,colS;								// shifted version of row & col
double		t1,t2,t3,t4;

					/* CALC DEST POINTERS */

	destStartPtr = (Ptr)(gPFLookUpTable[rowS = row<<TILE_SIZE_SH]+(colS = col<<TILE_SIZE_SH));
	destCopyStartPtr = (Ptr)(gPFCopyLookUpTable[rowS]+colS);

					/* CALC TILE DEFINITION ADDR */

	srcPtr = (double *)(gTilesPtr+((long)(gTileXlatePtr[tileNum])<<(TILE_SIZE_SH*2)));
	destPtr = (double *)destStartPtr;
	destCopyPtr = (double *)destCopyStartPtr;

						/* DRAW THE TILE */


	height = TILE_SIZE;
	do
	{
		t1 = srcPtr[0];
		t2 = srcPtr[1];
		t3 = srcPtr[2];
		t4 = srcPtr[3];

		destCopyPtr[0] = destPtr[0] = t1;
		destCopyPtr[1] = destPtr[1] = t2;
		destCopyPtr[2] = destPtr[2] = t3;
		destCopyPtr[3] = destPtr[3] = t4;

		srcPtr += 4;
		destPtr += (PF_BUFFER_WIDTH-TILE_SIZE)/8+4;			// next line
		destCopyPtr += (PF_BUFFER_WIDTH-TILE_SIZE)/8+4;
	}while(--height);
}

#else

/************************ DRAW A TILE : SIMPLE ***********************/
//
// This simple version doesnt worry about masks at all.
// Assumes that tileNum has ALREADY BEEN FILTERED!
//

void DrawATile_Simple(unsigned short tileNum, short row, short col)
{
unsigned char		*destPtr,*srcPtr,*destCopyPtr;
long		height;
short		i;
Ptr			destStartPtr,destCopyStartPtr;
unsigned long	rowS,colS;								// shifted version of row & col

					/* CALC DEST POINTERS */

	destStartPtr = (Ptr)(gPFLookUpTable[rowS = row<<TILE_SIZE_SH]+(colS = col<<TILE_SIZE_SH));
	destCopyStartPtr = (Ptr)(gPFCopyLookUpTable[rowS]+colS);

					/* CALC TILE DEFINITION ADDR */

	srcPtr = (unsigned char *)(gTilesPtr+((long)(gTileXlatePtr[tileNum])<<(TILE_SIZE_SH*2)));
	destPtr = (unsigned char *)destStartPtr;
	destCopyPtr = (unsigned char *)destCopyStartPtr;

						/* DRAW THE TILE */


	height = TILE_SIZE;
	do
	{
		for (i = 0; i < TILE_SIZE; i++)
			destCopyPtr[i] = destPtr[i] = *srcPtr++;

		destPtr += PF_BUFFER_WIDTH;			// next line
		destCopyPtr += PF_BUFFER_WIDTH;
	}while(--height);
}

#endif


#if 1
/********************* DISPLAY PLAYFIELD ***************/
//
// Dump Current playfield area to the screen
//

void DisplayPlayfield(void)
{
register double		*destPtr,*srcPtr;
register long		top,left,width;
register unsigned long	height;
long		numSegments,seg;
Ptr			destPtrs[4];
Ptr			srcPtrs[4];
unsigned long	heights[4];
long		widths[4];
register long		srcAdd,destAdd;
long		method;
long		*s,*d;

			/* IF ON POWER PC 603 OR 604, USE ALTERNATE VERSION */

	if (gIsPPC603_604)
	{
		DisplayPlayfield603_604();
		return;
	}

	top = (gScrollY % PF_BUFFER_HEIGHT);					// get PF buffer pixel coords to start @
	left = (gScrollX % PF_BUFFER_WIDTH);

	if (gShakeyScreenCount)									// see if do shakey screen
	{
		short		shakeyScreenX,shakeyScreenY;

		gShakeyScreenCount--;
		shakeyScreenX = MyRandomLong()&b1111 - 8;
		shakeyScreenY = MyRandomLong()&b1111 - 8;
		if ((top += shakeyScreenY) < 0)
			top = 0;
		if ((left += shakeyScreenX) < 0)
			left = 0;
		if ((top >= PF_BUFFER_HEIGHT))
			top = PF_BUFFER_HEIGHT-1;
		if ((left >= PF_BUFFER_WIDTH))
			left = PF_BUFFER_WIDTH-1;
	}


	if ((left+(PF_WINDOW_WIDTH-1)) > PF_BUFFER_WIDTH)		// see if 2 horiz segments
	{

						/* 2 HORIZ SEGMENTS */

		if ((top+(PF_WINDOW_HEIGHT-1)) > PF_BUFFER_HEIGHT)	// see if 2 vertical segments
		{
			method = 0;
			numSegments = 4;
			widths[0] = widths[2] = (PF_BUFFER_WIDTH-left);
			heights[0] = heights[1] = PF_BUFFER_HEIGHT-top;
			destPtrs[1] = (destPtrs[0] = (Ptr)(gScreenLookUpTable[PF_WINDOW_TOP]+
								+PF_WINDOW_LEFT)) + widths[0];

			widths[1] = widths[3] = (PF_WINDOW_WIDTH-widths[0]);
			heights[2] = heights[3] = PF_WINDOW_HEIGHT-heights[0];

			srcPtrs[2] = (Ptr)(gPFLookUpTable[0]+left);

			destPtrs[3] = (Ptr)((destPtrs[2] = (Ptr)(gScreenLookUpTable[PF_WINDOW_TOP+heights[0]]+PF_WINDOW_LEFT))+widths[0]);
			srcPtrs[3] = (Ptr)(gPFLookUpTable[0]);
			srcPtrs[0] = (srcPtrs[1] = (Ptr)(gPFLookUpTable[top])) + left;
		}
		else														// 1 vertical segment
		{
			method = 1;
			numSegments = 2;
			srcPtrs[0] = (Ptr)((srcPtrs[1] = (Ptr)(gPFLookUpTable[top]))+left);
			widths[0] = PF_BUFFER_WIDTH-left;
			heights[0] = heights[1] = PF_WINDOW_HEIGHT;

			destPtrs[1] = (Ptr)((destPtrs[0] = (Ptr)(gScreenLookUpTable[PF_WINDOW_TOP]+
								PF_WINDOW_LEFT))+widths[0]);
			widths[1] = (PF_WINDOW_WIDTH-widths[0]);
		}
	}
					/* ONLY 1 HORIZ SEGMENT */
	else
	{
		if ((top+(PF_WINDOW_HEIGHT-1)) > PF_BUFFER_HEIGHT)			// see if 2 vertical segments
		{
			method = 2;
			numSegments = 2;
			destPtrs[0] = (Ptr)(gScreenLookUpTable[PF_WINDOW_TOP]+
								PF_WINDOW_LEFT);
			widths[0] = widths[1] = PF_WINDOW_WIDTH;
			heights[1] = PF_WINDOW_HEIGHT-(heights[0] = PF_BUFFER_HEIGHT-top);

			destPtrs[1] = (Ptr)(gScreenLookUpTable[PF_WINDOW_TOP+heights[0]]+
								PF_WINDOW_LEFT);
			srcPtrs[1] = (Ptr)(gPFLookUpTable[0]+left);
			srcPtrs[0] = (Ptr)(gPFLookUpTable[top]+left);
		}
		else														// 1 vertical segment
		{
			method = 3;
			numSegments = 1;
			destPtrs[0] = (Ptr)(gScreenLookUpTable[PF_WINDOW_TOP]+
								PF_WINDOW_LEFT);
			srcPtrs[0] = (Ptr)gPFLookUpTable[top]+left;
			widths[0] = PF_WINDOW_WIDTH;
			heights[0] = PF_WINDOW_HEIGHT;
		}
	}

					/**********************************/
					/* SPECIAL CASE METHOD 0 / 4 SEGS */
					/**********************************/

	if (method == 0)
	{
register double	*destPtr2,*srcPtr2;
register long	width2,srcAdd2,destAdd2,pixWid,pixWid2;

		for (seg = 0; seg < 4; seg+=2)
		{
			destPtr = (double *)destPtrs[0+seg];
			srcPtr	= (double *)srcPtrs[0+seg];
			destPtr2 = (double *)destPtrs[1+seg];
			srcPtr2	= (double *)srcPtrs[1+seg];

			height = heights[0+seg];					// set height
			width = (pixWid = widths[0+seg])>>3;		// set double width
			width2 = (pixWid2 = widths[1+seg])>>3;		// set double width

			srcAdd = (PF_BUFFER_WIDTH>>3)-width;		// calc line add (for normal display)
			destAdd = (gScreenRowOffsetLW>>1)-width;
			srcAdd2 = (PF_BUFFER_WIDTH>>3)-width2;		// calc line add (for normal display)
			destAdd2 = (gScreenRowOffsetLW>>1)-width2;

						/* DO 1ST SEG */
			do
			{

				ILD(destPtr,srcPtr,width)

				switch(pixWid&b111)
				{
					case	7:
							*((Ptr)destPtr+6) = *((Ptr)srcPtr+6);
					case	6:
							*((Ptr)destPtr+5) = *((Ptr)srcPtr+5);
					case	5:
							*((Ptr)destPtr+4) = *((Ptr)srcPtr+4);
					case	4:
							*((Ptr)destPtr+3) = *((Ptr)srcPtr+3);
					case	3:
							*((Ptr)destPtr+2) = *((Ptr)srcPtr+2);
					case	2:
							*((Ptr)destPtr+1) = *((Ptr)srcPtr+1);
					case	1:
							*((Ptr)destPtr) = *((Ptr)srcPtr);
				}


						/* DO 2ND SEG */

//				w = pixWid2;

				if (((long)destPtr2 & b11) == 3)				// IF ON ALIGNMENT OF 3, DRAW WITH LONGS TO AVOID POWERMAC BUS GLITCH!!!!
				{
					s = (long *)srcPtr2;
					d = (long *)destPtr2;
					for (top=0; top < (width2*2); top++)
						*d++ = *s++;
					destPtr2 += width2;
					srcPtr2 += width2;
				}
				else
				{
					ILD(destPtr2,srcPtr2,width2);
				}

				switch(pixWid2&b111)
				{
					case	7:
							*((Ptr)destPtr2+6) = *((Ptr)srcPtr2+6);
					case	6:
							*((Ptr)destPtr2+5) = *((Ptr)srcPtr2+5);
					case	5:
							*((Ptr)destPtr2+4) = *((Ptr)srcPtr2+4);
					case	4:
							*((Ptr)destPtr2+3) = *((Ptr)srcPtr2+3);
					case	3:
							*((Ptr)destPtr2+2) = *((Ptr)srcPtr2+2);
					case	2:
							*((Ptr)destPtr2+1) = *((Ptr)srcPtr2+1);
					case	1:
							*((Ptr)destPtr2) = *((Ptr)srcPtr2);
				}

				srcPtr += srcAdd;							// next line
				destPtr += destAdd;
				srcPtr2 += srcAdd2;
				destPtr2 += destAdd2;
			} while (--height);
		}
	}
	else
	{
					/**********************************/
					/* NORMAL CASE METHOD 1..3        */
					/**********************************/

						/* COPY THE SEGMENTS */

		for (seg=0; seg < numSegments; seg++)
		{
			register double	*destStartPtr,*srcStartPtr;
			register long	pixWid;

			destStartPtr = (double *)destPtrs[seg];
			srcStartPtr	= (double *)srcPtrs[seg];

			height = heights[seg];						// set height
			width = (pixWid = widths[seg])>>3;			// set double width

			srcAdd = (PF_BUFFER_WIDTH>>3);				// calc line rowdoubles
			destAdd = (gScreenRowOffsetLW>>1);

			do
			{
				destPtr = destStartPtr;
				srcPtr = srcStartPtr;

				if (((long)destPtr & b11) == 3)				// IF ON ALIGNMENT OF 3, DRAW WITH LONGS TO AVOID POWERMAC BUS GLITCH!!!!
				{
					s = (long *)srcPtr;
					d = (long *)destPtr;
					for (top=0; top < (width*2); top++)
						*d++ = *s++;
					destPtr += width;
					srcPtr += width;
				}
				else
				{
					ILD(destPtr,srcPtr,width)
				}

				switch(pixWid&b111)								// draw remainder
				{
					case	7:
							*((Ptr)destPtr+6) = *((Ptr)srcPtr+6);
					case	6:
							*((Ptr)destPtr+5) = *((Ptr)srcPtr+5);
					case	5:
							*((Ptr)destPtr+4) = *((Ptr)srcPtr+4);
					case	4:
							*((Ptr)destPtr+3) = *((Ptr)srcPtr+3);
					case	3:
							*((Ptr)destPtr+2) = *((Ptr)srcPtr+2);
					case	2:
							*((Ptr)destPtr+1) = *((Ptr)srcPtr+1);
					case	1:
							*((Ptr)destPtr) = *((Ptr)srcPtr);
				}

				srcStartPtr += srcAdd;									// next line
				destStartPtr += destAdd;

			} while (--height);
		}
	}
}

#else

/********************* DISPLAY PLAYFIELD ***************/
//
// Dump Current playfield area to the screen
//

void DisplayPlayfield(void)
{
Ptr			destPtr,srcPtr;
long		top,left,width;
unsigned long	height;
long		numSegments,seg;
Ptr			destPtrs[4];
Ptr			srcPtrs[4];
unsigned long	heights[4];
long		widths[4];
long		srcAdd,destAdd;
long		method,i;

	top = (gScrollY % PF_BUFFER_HEIGHT);					// get PF buffer pixel coords to start @
	left = (gScrollX % PF_BUFFER_WIDTH);

	if (gShakeyScreenCount)									// see if do shakey screen
	{
		short		shakeyScreenX,shakeyScreenY;

		gShakeyScreenCount--;
		shakeyScreenX = MyRandomLong()&b1111 - 8;
		shakeyScreenY = MyRandomLong()&b1111 - 8;
		if ((top += shakeyScreenY) < 0)
			top = 0;
		if ((left += shakeyScreenX) < 0)
			left = 0;
		if ((top >= PF_BUFFER_HEIGHT))
			top = PF_BUFFER_HEIGHT-1;
		if ((left >= PF_BUFFER_WIDTH))
			left = PF_BUFFER_WIDTH-1;
	}


	if ((left+(PF_WINDOW_WIDTH-1)) > PF_BUFFER_WIDTH)		// see if 2 horiz segments
	{

						/* 2 HORIZ SEGMENTS */

		if ((top+(PF_WINDOW_HEIGHT-1)) > PF_BUFFER_HEIGHT)	// see if 2 vertical segments
		{
			method = 0;
			numSegments = 4;
			widths[0] = widths[2] = (PF_BUFFER_WIDTH-left);
			heights[0] = heights[1] = PF_BUFFER_HEIGHT-top;
			destPtrs[1] = (destPtrs[0] = (Ptr)(gScreenLookUpTable[PF_WINDOW_TOP]+
								+PF_WINDOW_LEFT)) + widths[0];

			widths[1] = widths[3] = (PF_WINDOW_WIDTH-widths[0]);
			heights[2] = heights[3] = PF_WINDOW_HEIGHT-heights[0];

			srcPtrs[2] = (Ptr)(gPFLookUpTable[0]+left);

			destPtrs[3] = (Ptr)((destPtrs[2] = (Ptr)(gScreenLookUpTable[PF_WINDOW_TOP+heights[0]]+PF_WINDOW_LEFT))+widths[0]);
			srcPtrs[3] = (Ptr)(gPFLookUpTable[0]);
			srcPtrs[0] = (srcPtrs[1] = (Ptr)(gPFLookUpTable[top])) + left;
		}
		else														// 1 vertical segment
		{
			method = 1;
			numSegments = 2;
			srcPtrs[0] = (Ptr)((srcPtrs[1] = (Ptr)(gPFLookUpTable[top]))+left);
			widths[0] = PF_BUFFER_WIDTH-left;
			heights[0] = heights[1] = PF_WINDOW_HEIGHT;

			destPtrs[1] = (Ptr)((destPtrs[0] = (Ptr)(gScreenLookUpTable[PF_WINDOW_TOP]+
								PF_WINDOW_LEFT))+widths[0]);
			widths[1] = (PF_WINDOW_WIDTH-widths[0]);
		}
	}
					/* ONLY 1 HORIZ SEGMENT */
	else
	{
		if ((top+(PF_WINDOW_HEIGHT-1)) > PF_BUFFER_HEIGHT)			// see if 2 vertical segments
		{
			method = 2;
			numSegments = 2;
			destPtrs[0] = (Ptr)(gScreenLookUpTable[PF_WINDOW_TOP]+
								PF_WINDOW_LEFT);
			widths[0] = widths[1] = PF_WINDOW_WIDTH;
			heights[1] = PF_WINDOW_HEIGHT-(heights[0] = PF_BUFFER_HEIGHT-top);

			destPtrs[1] = (Ptr)(gScreenLookUpTable[PF_WINDOW_TOP+heights[0]]+
								PF_WINDOW_LEFT);
			srcPtrs[1] = (Ptr)(gPFLookUpTable[0]+left);
			srcPtrs[0] = (Ptr)(gPFLookUpTable[top]+left);
		}
		else														// 1 vertical segment
		{
			method = 3;
			numSegments = 1;
			destPtrs[0] = (Ptr)(gScreenLookUpTable[PF_WINDOW_TOP]+
								PF_WINDOW_LEFT);
			srcPtrs[0] = (Ptr)gPFLookUpTable[top]+left;
			widths[0] = PF_WINDOW_WIDTH;
			heights[0] = PF_WINDOW_HEIGHT;
		}
	}

					/**********************************/
					/* SPECIAL CASE METHOD 0 / 4 SEGS */
					/**********************************/

	if (method == 0)
	{
		Ptr		destPtr2,srcPtr2;
		long	width2,srcAdd2,destAdd2,pixWid,pixWid2;

		for (seg = 0; seg < 4; seg+=2)
		{
			destPtr = destPtrs[0+seg];
			srcPtr	= srcPtrs[0+seg];
			destPtr2 = destPtrs[1+seg];
			srcPtr2	= srcPtrs[1+seg];

			height = heights[0+seg];					// set height
			width = (pixWid = widths[0+seg]);			// set width
			width2 = (pixWid2 = widths[1+seg]);			// set width

			srcAdd = (PF_BUFFER_WIDTH)-width;			// calc line add (for normal display)
			destAdd = (gScreenRowOffset)-width;
			srcAdd2 = (PF_BUFFER_WIDTH)-width2;			// calc line add (for normal display)
			destAdd2 = (gScreenRowOffset)-width2;

						/* DO 1ST SEG */
			do
			{
				BlockMove(srcPtr, destPtr, width);
//				for (i = 0; i < width; i++)
//					destPtr[i] = srcPtr[i];
				destPtr += width;
				srcPtr += width;


						/* DO 2ND SEG */

				BlockMove(srcPtr2, destPtr2, width2);
//				for (i = 0; i < width2; i++)
//					destPtr2[i] = srcPtr2[i];
				destPtr2 += width2;
				srcPtr2 += width2;

				srcPtr += srcAdd;							// next line
				destPtr += destAdd;
				srcPtr2 += srcAdd2;
				destPtr2 += destAdd2;
			} while (--height);
		}
	}
	else
	{
					/**********************************/
					/* NORMAL CASE METHOD 1..3        */
					/**********************************/

						/* COPY THE SEGMENTS */

		for (seg=0; seg < numSegments; seg++)
		{
			Ptr		destStartPtr,srcStartPtr;
			long	pixWid;

			destStartPtr = destPtrs[seg];
			srcStartPtr	= srcPtrs[seg];

			height = heights[seg];						// set height
			width = (pixWid = widths[seg]);			// set width

			srcAdd = (PF_BUFFER_WIDTH);				// calc line rowdoubles
			destAdd = (gScreenRowOffset);

			do
			{
				destPtr = destStartPtr;
				srcPtr = srcStartPtr;

				BlockMove(srcPtr, destPtr, width);
//				for (i = 0; i < width; i++)
//					destPtr[i] = srcPtr[i];

				destPtr += width;
				srcPtr += width;

				srcStartPtr += srcAdd;									// next line
				destStartPtr += destAdd;

			} while (--height);
		}
	}
}



#endif

/********************* DISPLAY PLAYFIELD 603 / 604 ***************/
//
// Special version for 603 & 604 processors.  Uses longs instead
// of doubles to blit the data in the hope that misaligned
// accesses will be minimized
//

static void DisplayPlayfield603_604(void)
{
register long		*destPtr,*srcPtr;
register long		top,left,width;
register unsigned long	height;
long		numSegments,seg;
Ptr			destPtrs[4];
Ptr			srcPtrs[4];
unsigned long	heights[4];
long		widths[4];
register long		srcAdd,destAdd;
long		method;

	top = (gScrollY % PF_BUFFER_HEIGHT);					// get PF buffer pixel coords to start @
	left = (gScrollX % PF_BUFFER_WIDTH);

	if (gShakeyScreenCount)									// see if do shakey screen
	{
		short		shakeyScreenX,shakeyScreenY;

		gShakeyScreenCount--;
		shakeyScreenX = MyRandomLong()&b1111 - 8;
		shakeyScreenY = MyRandomLong()&b1111 - 8;
		if ((top += shakeyScreenY) < 0)
			top = 0;
		if ((left += shakeyScreenX) < 0)
			left = 0;
		if ((top >= PF_BUFFER_HEIGHT))
			top = PF_BUFFER_HEIGHT-1;
		if ((left >= PF_BUFFER_WIDTH))
			left = PF_BUFFER_WIDTH-1;
	}


	if ((left+(PF_WINDOW_WIDTH-1)) > PF_BUFFER_WIDTH)		// see if 2 horiz segments
	{

						/* 2 HORIZ SEGMENTS */

		if ((top+(PF_WINDOW_HEIGHT-1)) > PF_BUFFER_HEIGHT)	// see if 2 vertical segments
		{
			method = 0;
			numSegments = 4;
			widths[0] = widths[2] = (PF_BUFFER_WIDTH-left);
			heights[0] = heights[1] = PF_BUFFER_HEIGHT-top;
			destPtrs[1] = (destPtrs[0] = (Ptr)(gScreenLookUpTable[PF_WINDOW_TOP]+
								+PF_WINDOW_LEFT)) + widths[0];

			widths[1] = widths[3] = (PF_WINDOW_WIDTH-widths[0]);
			heights[2] = heights[3] = PF_WINDOW_HEIGHT-heights[0];

			srcPtrs[2] = (Ptr)(gPFLookUpTable[0]+left);

			destPtrs[3] = (Ptr)((destPtrs[2] = (Ptr)(gScreenLookUpTable[PF_WINDOW_TOP+heights[0]]+PF_WINDOW_LEFT))+widths[0]);
			srcPtrs[3] = (Ptr)(gPFLookUpTable[0]);
			srcPtrs[0] = (srcPtrs[1] = (Ptr)(gPFLookUpTable[top])) + left;
		}
		else														// 1 vertical segment
		{
			method = 1;
			numSegments = 2;
			srcPtrs[0] = (Ptr)((srcPtrs[1] = (Ptr)(gPFLookUpTable[top]))+left);
			widths[0] = PF_BUFFER_WIDTH-left;
			heights[0] = heights[1] = PF_WINDOW_HEIGHT;

			destPtrs[1] = (Ptr)((destPtrs[0] = (Ptr)(gScreenLookUpTable[PF_WINDOW_TOP]+
								PF_WINDOW_LEFT))+widths[0]);
			widths[1] = (PF_WINDOW_WIDTH-widths[0]);
		}
	}
					/* ONLY 1 HORIZ SEGMENT */
	else
	{
		if ((top+(PF_WINDOW_HEIGHT-1)) > PF_BUFFER_HEIGHT)			// see if 2 vertical segments
		{
			method = 2;
			numSegments = 2;
			destPtrs[0] = (Ptr)(gScreenLookUpTable[PF_WINDOW_TOP]+
								PF_WINDOW_LEFT);
			widths[0] = widths[1] = PF_WINDOW_WIDTH;
			heights[1] = PF_WINDOW_HEIGHT-(heights[0] = PF_BUFFER_HEIGHT-top);

			destPtrs[1] = (Ptr)(gScreenLookUpTable[PF_WINDOW_TOP+heights[0]]+
								PF_WINDOW_LEFT);
			srcPtrs[1] = (Ptr)(gPFLookUpTable[0]+left);
			srcPtrs[0] = (Ptr)(gPFLookUpTable[top]+left);
		}
		else														// 1 vertical segment
		{
			method = 3;
			numSegments = 1;
			destPtrs[0] = (Ptr)(gScreenLookUpTable[PF_WINDOW_TOP]+
								PF_WINDOW_LEFT);
			srcPtrs[0] = (Ptr)gPFLookUpTable[top]+left;
			widths[0] = PF_WINDOW_WIDTH;
			heights[0] = PF_WINDOW_HEIGHT;
		}
	}

					/**********************************/
					/* SPECIAL CASE METHOD 0 / 4 SEGS */
					/**********************************/

	if (method == 0)
	{
register long	*destPtr2,*srcPtr2;
register long	width2,srcAdd2,destAdd2,pixWid,pixWid2;

		for (seg = 0; seg < 4; seg+=2)
		{
			destPtr = (long *)destPtrs[0+seg];
			srcPtr	= (long *)srcPtrs[0+seg];
			destPtr2 = (long *)destPtrs[1+seg];
			srcPtr2	= (long *)srcPtrs[1+seg];

			height = heights[0+seg];					// set height
			width = (pixWid = widths[0+seg])>>2;		// set long width
			width2 = (pixWid2 = widths[1+seg])>>2;		// set long width

			srcAdd = (PF_BUFFER_WIDTH>>2)-width;		// calc line add (for normal display)
			destAdd = gScreenRowOffsetLW-width;
			srcAdd2 = (PF_BUFFER_WIDTH>>2)-width2;		// calc line add (for normal display)
			destAdd2 = (gScreenRowOffsetLW)-width2;

						/* DO 1ST SEG */
			do
			{
				for (top = 0; top < width; top++)		// copy longs
					destPtr[top] = srcPtr[top];
				destPtr += width;
				srcPtr += width;

				switch(pixWid&b11)						// copy remaining pixels
				{
					case	3:
							*((Ptr)destPtr+2) = *((Ptr)srcPtr+2);
					case	2:
							*((Ptr)destPtr+1) = *((Ptr)srcPtr+1);
					case	1:
							*((Ptr)destPtr) = *((Ptr)srcPtr);
				}


						/* DO 2ND SEG */

				for (top=0; top < width2; top++)		// copy longs
					destPtr2[top] = srcPtr2[top];
				destPtr2 += width2;
				srcPtr2 += width2;

				switch(pixWid2&b11)							// copy remaining pixels
				{
					case	3:
							*((Ptr)destPtr2+2) = *((Ptr)srcPtr2+2);
					case	2:
							*((Ptr)destPtr2+1) = *((Ptr)srcPtr2+1);
					case	1:
							*((Ptr)destPtr2) = *((Ptr)srcPtr2);
				}

				srcPtr += srcAdd;							// next line
				destPtr += destAdd;
				srcPtr2 += srcAdd2;
				destPtr2 += destAdd2;
			} while (--height);
		}
	}
	else
	{

					/**********************************/
					/* NORMAL CASE METHOD 1..3        */
					/**********************************/

						/* COPY THE SEGMENTS */

		for (seg=0; seg < numSegments; seg++)
		{
			destPtr = (long *)destPtrs[seg];
			srcPtr	= (long *)srcPtrs[seg];

			height = heights[seg];						// set height
			width = (widths[seg])>>2;					// set long-width

			srcAdd = (PF_BUFFER_WIDTH>>2)-width;		// calc line add (for normal display)
			destAdd = (gScreenRowOffsetLW)-width;

			do
			{
				for (top = 0; top < width; top++)		// copy longs
					destPtr[top] = srcPtr[top];
				destPtr += width;
				srcPtr += width;


				top = widths[seg]&b11;					// draw remainder

				switch(top)
				{
					case	3:
							*((Ptr)destPtr+2) = *((Ptr)srcPtr+2);
					case	2:
							*((Ptr)destPtr+1) = *((Ptr)srcPtr+1);
					case	1:
							*((Ptr)destPtr) = *((Ptr)srcPtr);
				}

				srcPtr += srcAdd;							// next line
				destPtr += destAdd;

			} while (--height);
		}
	}
}



/**************** UPDATE TILE ANIMATION **********************/
// Source port note: moved from TileAnim.c

void UpdateTileAnimation(void)
{
unsigned long	row;
unsigned short	newTile;
register unsigned short	targetTile,*intPtr,*basePtr;
register long	col;
register long	x,y,animNum;
unsigned long 	origRow,origCol;

	origRow = gScrollRow % PF_TILE_HEIGHT;									// calc row in buffer
	origCol = gScrollCol % PF_TILE_WIDTH;									// calc col in buffer

	for (animNum = 0; animNum < gNumTileAnims; animNum++)
	{
						/* CHECK COUNTER */

		if ((gTileAnims[animNum].count -= gTileAnims[animNum].defPtr->speed) < 0)
		{
			gTileAnims[animNum].count = 0x100;								// reset counter

						/* SCAN VISIBLE AREA FOR TARGET TILE */

			targetTile = gTileAnims[animNum].defPtr->baseTile;				// get target basetile
			newTile = gTileAnims[animNum].defPtr->tileNums[gTileAnims[animNum].index];	// get tile to draw

			basePtr = (unsigned short *)&gPlayfield[gScrollRow][gScrollCol]; // get ptr to start of scan

			row = origRow;													// get modable row

			y = 0;
			do
			{
				intPtr = basePtr;											// get ptr to start of horiz scan
				col = origCol;
				x = PF_TILE_WIDTH;

				do
				{
					if ((*intPtr++ & TILENUM_MASK) == targetTile)
						DrawATile_Simple(newTile,row,col);

					if (++col >= PF_TILE_WIDTH)								// see if column wrap
						col = 0;
				} while (--x);

				if (++row >= PF_TILE_HEIGHT)								// see if row wrap
					row = 0;

				basePtr += gPlayfieldTileWidth;								// next row in map
			} while (++y < PF_TILE_HEIGHT);


			y = ++gTileAnims[animNum].index;								// increment index
			if (y  >= gTileAnims[animNum].defPtr->numFrames)				// see if at end of sequence
				gTileAnims[animNum].index = 0;
		}
	}
}

