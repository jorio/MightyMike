/****************************/
/*    	STORE               */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/****************************/
/*    EXTERNALS             */
/****************************/
#include "windows.h"
#include "object.h"
#include "triggers.h"
#include "shape.h"
#include "playfield.h"
#include "store.h"
#include "infobar.h"
#include "misc.h"
#include "io.h"
#include "sound2.h"
#include "objecttypes.h"
#include "weapon.h"

extern	ObjNode			*gThisNodePtr,*FirstNodePtr,*gMyNodePtr;
extern	long			gDX,gDY,gSumDX,gSumDY;
extern	WindowPtr		gGameWindow;
extern	union_gX;
extern	union_gY;
extern	Byte			gCurrentWeaponType,gSceneNum;
extern	long			gNumCoins;
extern	Boolean			gShootButtonDownFlag;
extern	unsigned char	gInterlaceMode;
extern	GDHandle		gMainScreen;
extern	PixMapHandle	gMainScreenPixMap;
extern	Ptr				gScreenAddr;
extern	char  			gMMUMode;
extern	long			gScreenRowOffsetLW,gScreenRowOffset;
extern	long			gScreenLookUpTable[VISIBLE_HEIGHT];
extern	WindowPtr		gGameWindow;
extern	long			gPFLookUpTable[PF_BUFFER_HEIGHT];
extern	int				gScrollX,gScrollY;
extern	Handle			gPFBufferHandle,gPFBufferCopyHandle;


/****************************/
/*    CONSTANTS             */
/****************************/

#define	STORE_RES_BASE	1000						// base resource # for stores

#define	ITEM_X		(30L)
#define	ITEM_Y		(255L)
#define	ITEM_DIST	(76+18)



/**********************/
/*     VARIABLES      */
/**********************/

Boolean		gEnterStoreFlag,gExitStoreFlag,gSelectButtonDownFlag;
short		gStoreNum;
Byte		gCursorSelectNum;


StoreDefType	**gStoreDefType = nil;





/****************** ENTER A STORE **********************/

void EnterAStore(void)
{
ObjNode		*oldFirstNodePtr;

				/* LOAD STORE DEFINITION RESOURCE */

	gStoreDefType = (StoreDefType **)GetResource('StOr',STORE_RES_BASE+gStoreNum);
	if (gStoreDefType == nil)
		DoFatalAlert("Couldnt Open Store Resource!");


				/* LOAD STORE SHAPES */

	LoadShapeTable(":data:shapes:storedefault.shapes",GROUP_STORE_DEFAULT,DONT_GET_PALETTE);		// defaults


				/* INIT STORE SPRITES */

	oldFirstNodePtr = FirstNodePtr;							// keep old environment
	FirstNodePtr = nil;										// trick into thinking that object list is empty

	DrawStoreImage();										// draw store image
	InitStoreCursor();										// init the cursor


	DrawStoreItems();										// show what's for sale
	DisplayStoreBuffer();


					/* PROCESS THE STORE */

	ProcessStore();

						/* EXIT THE STORE */

	ReleaseResource((Handle)gStoreDefType);					// release store resource

	ZapShapeTable(GROUP_STORE_SPECIFIC);					// zap shapes used
	ZapShapeTable(GROUP_STORE_DEFAULT);

	gMyNodePtr->Y.Int += 5;									// move me away from door
	gMyNodePtr->DY = 0;
	gEnterStoreFlag = FALSE;								// we're out
	gShootButtonDownFlag = TRUE;

	DeleteAllObjects();										// delete all store objects
	FirstNodePtr = oldFirstNodePtr;							// reset old environment

	EraseStore();
}



/********************** DRAW STORE IMAGE *****************************/
//
// Draws store image directly into Playfield main buffer
//

void DrawStoreImage(void)
{
int			width,height;
register int	i,k;
long		numToRead;
short		fRefNum,vRefNum;
Rect		box;
register long	*linePtr,*destPtr;
OSErr		iErr;

	GetVol(nil,&vRefNum);													// get default volume
	switch(gSceneNum)
	{

		case	SCENE_JURASSIC:
				iErr = FSOpen(":data:images:jurassicStore.image",vRefNum,&fRefNum);		// open file
				break;

		case	SCENE_CANDY:
				iErr = FSOpen(":data:images:candyStore.image",vRefNum,&fRefNum);
				break;

		case	SCENE_CLOWN:
				iErr = FSOpen(":data:images:clownStore.image",vRefNum,&fRefNum);
				break;

		default:
				iErr = FSOpen(":data:images:jurassicStore.image",vRefNum,&fRefNum);
	}

	if (iErr != noErr)
	{
		ShowSystemErr(iErr);
		DoFatalAlert("Cant open Tienda image!");
	}

	SetFPos(fRefNum,fsFromStart,256*sizeof(RGBColor)+8);				// skip palette & pack header


	numToRead = 2;
	FSRead(fRefNum,&numToRead,&width);									// read width
	numToRead = 2;
	FSRead(fRefNum,&numToRead,&height);									// read height

	linePtr = (long *)AllocPtr(width);									// alloc memory to hold line of data

			/* WRITE TO BUFFER */

	gMMUMode = true32b;									// we must do this in 32bit addressing mode
	SwapMMUMode(&gMMUMode);

	destPtr = (long *)(gPFLookUpTable[0]);

	for (i = 0; i < height; i++)
	{

		numToRead = width;												// read 1 line of data
		FSRead(fRefNum,&numToRead,linePtr);

		for (k = 0; k < (width>>2); k++)
		{
			*destPtr++ = linePtr[k];
		}
	}

	SwapMMUMode(&gMMUMode);								// Restore addressing mode

	FSClose(fRefNum);
	DisposePtr((Ptr)linePtr);

}

/******************* DISPLAY STORE BUFFER *********************/
//
// The store image has been drawn into the primary PF buffer, so copy the PF buffer
// to the screen.
//

void DisplayStoreBuffer(void)
{
long *srcPtr,*destPtr,*destStart;
int		x,y;

	srcPtr = (long *)(gPFLookUpTable[0]);
	destStart = (long *)(gScreenLookUpTable[PF_WINDOW_TOP]+PF_WINDOW_LEFT);

	gMMUMode = true32b;										// we must do this in 32bit addressing mode
	SwapMMUMode(&gMMUMode);

	for (y = 0; y < PF_WINDOW_HEIGHT; y++)
	{
		destPtr = destStart;

		for (x = 0; x < (PF_WINDOW_WIDTH/4); x++)
		{
			*destPtr++ = *srcPtr++;
		}
		destStart += gScreenRowOffsetLW;
	}

	SwapMMUMode(&gMMUMode);						// Restore addressing mode
}


/******************** INIT STORE CURSOR *************************/

void InitStoreCursor(void)
{
register	ObjNode		*newObj;

	newObj = MakeNewShape(GroupNum_StoreCursor,ObjType_StoreCursor,0,ITEM_X,ITEM_Y,100,MoveStoreCursor,SCREEN_RELATIVE);
	if (newObj == nil)
		DoFatalAlert("Cannot alloc Store Cursor!");

	newObj->DrawFlag = FALSE;
	newObj->EraseFlag = FALSE;


	gCursorSelectNum = 0;								// start on leftmost item

	gSelectButtonDownFlag = TRUE;
}


/****************** DRAW STORE ITEMS **********************/

void DrawStoreItems(void)
{
Byte	i;

	for (i=0; i < 3; i++)
	{

		DrawFrameAt_NoMask(ITEM_X+(i*ITEM_DIST)+1,ITEM_Y+2,GroupNum_WeaponIcon,
							ObjType_WeaponIcon,(*gStoreDefType)[i].type,(Ptr)gPFLookUpTable[0],PF_WINDOW_WIDTH);

		PrintNumAt((*gStoreDefType)[i].price,
				 3,
				 ITEM_X+(i*ITEM_DIST)+50,
				 ITEM_Y+111,
				 (Ptr)gPFLookUpTable[0],PF_WINDOW_WIDTH);

	}
}


/***************** PROCESS STORE *********************/
//
// Main loop for handling store functions
//

void ProcessStore(void)
{
	gExitStoreFlag = FALSE;

	do
	{
		RegulateSpeed(GAME_SPEED);
		ReadKeyboard();
		MoveObjects();
		UpdateInfoBar();

		if (GetKeyState(KEY_Q))						// see if key quit
			CleanQuit();

		DoSoundMaintenance();						// (must be after readkeyboard)

	} while(!gExitStoreFlag);
}


/*********************** MOVE STORE CURSOR ************************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveStoreCursor(void)
{
static	Boolean wasLeftFlag,wasRightFlag;
short	price,item;

				/* SEE IF MOVE LEFT */


	if (GetKeyState(KEY_K4) && (gCursorSelectNum > 0))
	{
		if (!wasLeftFlag)
		{
			wasLeftFlag = TRUE;
			gCursorSelectNum--;
			PlaySound(SOUND_SELECTCHIME);
			CopyZoneToZone(20,80,												// erase cursor
						(Ptr)gPFLookUpTable[0]+(ITEM_Y*PF_WINDOW_WIDTH)+gThisNodePtr->X.Int,
						(Ptr)gScreenLookUpTable[ITEM_Y+PF_WINDOW_TOP]+gThisNodePtr->X.Int+PF_WINDOW_LEFT,
						PF_WINDOW_WIDTH,gScreenRowOffset);
		}
	}
	else
		wasLeftFlag = FALSE;


				/* SEE IF MOVE RIGHT */

	if (GetKeyState(KEY_K6) && (gCursorSelectNum < 3))
	{
		if (!wasRightFlag)
		{
			wasRightFlag = TRUE;
			gCursorSelectNum++;
			PlaySound(SOUND_SELECTCHIME);
			CopyZoneToZone(20,80,												// erase cursor
						(Ptr)gPFLookUpTable[0]+(ITEM_Y*PF_WINDOW_WIDTH)+gThisNodePtr->X.Int,
						(Ptr)gScreenLookUpTable[ITEM_Y+PF_WINDOW_TOP]+gThisNodePtr->X.Int+PF_WINDOW_LEFT,
						PF_WINDOW_WIDTH,gScreenRowOffset);
		}
	}
	else
		wasRightFlag = FALSE;

	gThisNodePtr->X.Int = ITEM_X+(gCursorSelectNum*ITEM_DIST);			// set new x coord


				/* SEE IF SELECT SOMETHING */

	if (GetKeyState(KEY_SPACE))
	{
		if (!gSelectButtonDownFlag)
		{
			gSelectButtonDownFlag = TRUE;
			if (gCursorSelectNum == 3)									// see if select exit
			{
				gExitStoreFlag = TRUE;
			}
			else
			{
				price = (*gStoreDefType)[gCursorSelectNum].price;		// get price of item selected
				item = (*gStoreDefType)[gCursorSelectNum].type;			// get item type
				if (price > gNumCoins)									// see if cant afford it
				{

				}
				else													// yes I can afford it
				{
					LoseCoins(price);									// give the money
//					AddToInventory(item,TRUE);							// put into inventory
				}
			}
		}
	}
	else
		gSelectButtonDownFlag = FALSE;									// button wasnt pressed


			/* DRAW CURSOR */

	DrawSpriteAt(gThisNodePtr,gThisNodePtr->X.Int+PF_WINDOW_LEFT,gThisNodePtr->Y.Int+PF_WINDOW_TOP,
				(Ptr)gScreenLookUpTable[0],gScreenRowOffset);
}


/******************** ERASE STORE **************************/
//
// Blanks alternate lines for interlace mode
//

void EraseStore(void)
{
register	long	*destPtr;
register	int		height,width;
register	long	destAdd,size,i;
Ptr		a,b;

				/* COPY PF BUFFER 2 TO BUFFER 1 TO ERASE STORE IMAGE */

	size = GetHandleSize(gPFBufferHandle);
	a = *gPFBufferHandle;
	b = *gPFBufferCopyHandle;
	for (i = 0; i < size; i++)
	{
		*a++ = *b++;
	}

				/* ERASE INTERLACING ZONE FROM MAIN SCREEN */

	if (gInterlaceMode)
	{
		gMMUMode = true32b;										// we must do this in 32bit addressing mode
		SwapMMUMode(&gMMUMode);


		destPtr = (long *)(gScreenLookUpTable[PF_WINDOW_TOP+1]+PF_WINDOW_LEFT);
		destAdd = gScreenRowOffsetLW*2-(PF_WINDOW_WIDTH>>2);

		for ( height = PF_WINDOW_HEIGHT>>1; height > 0; height--)
		{
			for (width = PF_WINDOW_WIDTH>>2; width > 0; width--)
			{
				*destPtr++ = 0xfefefefe;				// dark grey
			}
			destPtr += destAdd;
		}

		SwapMMUMode(&gMMUMode);						// Restore addressing mode
	}
}


