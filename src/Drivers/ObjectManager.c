/****************************/
/*    OBJECT MANAGER        */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/
#include "myglobals.h"
#include "playfield.h"
#include "window.h"
#include "object.h"
#include "misc.h"
#include "shape.h"
#include <string.h>

extern	Handle		gOffScreenHandle;
extern	Handle		gBackgroundHandle;
extern	char  			gMMUMode;
extern	long			gScreenRowOffsetLW,gScreenRowOffset;
extern	uint8_t*		gScreenLookUpTable[VISIBLE_HEIGHT];
extern	uint8_t*		gOffScreenLookUpTable[OFFSCREEN_HEIGHT];


/****************************/
/*    CONSTANTS             */
/****************************/

#define	MAX_REGIONS			(MAX_OBJECTS*2)

#define	MAX_CLIP_REGIONS	5					// see reserved clip regions

#define INVALID_NODE_FLAG	0xffffffffL			// put into CType when node is deleted

/**********************/
/*     VARIABLES      */
/**********************/

long		gRightSide,gLeftSide,gTopSide,gBottomSide;

											// Region Stuff
static	Rect		regionList[MAX_REGIONS];
static	long		numRegions;

											// OBJECT LIST
long		NumObjects;
ObjNode		*FirstNodePtr;
ObjNode		*ObjectList = nil;
ObjNode		*FreeNodeStack[MAX_OBJECTS];
long		NodeStackFront;

ObjNode		*gThisNodePtr,*gMostRecentlyAddedNode;

long		gDX,gDY,gSumDX,gSumDY;		// global object stuff

union {
	long L;
	short Frac;
	short	Int;
	} gX;

union {
	long L;
	short Frac;
	short	Int;
	} gY;

long		gRegionClipTop[MAX_CLIP_REGIONS],gRegionClipBottom[MAX_CLIP_REGIONS],
			gRegionClipLeft[MAX_CLIP_REGIONS],gRegionClipRight[MAX_CLIP_REGIONS];


/************************ INIT OBJECT MANAGER **********************/

void InitObjectManager(void)
{
short		i;
Handle	ObjHandle;

					/* INIT CLIPPING REGIONS */

	for (i=0; i<MAX_CLIP_REGIONS; i++)						// init ALL clipping areas to full buffer size
	{
		gRegionClipTop[i] = OFFSCREEN_WINDOW_TOP;
		gRegionClipBottom[i] = OFFSCREEN_WINDOW_TOP+VISIBLE_HEIGHT-1;
		gRegionClipLeft[i] = OFFSCREEN_WINDOW_LEFT;
		gRegionClipRight[i] = OFFSCREEN_WINDOW_LEFT+VISIBLE_WIDTH-1;
	}


				/* INIT LIKED LIST */


	if (ObjectList == nil)								// see if need to allocate memory for object list
	{
		ObjHandle = AllocHandle(sizeof(ObjNode)*MAX_OBJECTS);
		HLockHi(ObjHandle);
		ObjectList = (ObjNode *)*ObjHandle;
	}


	gThisNodePtr = nil;

					/* CLEAR ENTIRE OBJECT LIST */

	FirstNodePtr = nil;									// no node yet
	NumObjects = 0;
	for (i=0; i<MAX_OBJECTS; i++)
	{
		ObjectList[i].Genre = 0;
		ObjectList[i].Z = 0;
		ObjectList[i].Type = 0;
		ObjectList[i].SubType = 0;
		ObjectList[i].DrawFlag = false;
		ObjectList[i].EraseFlag = false;
		ObjectList[i].MoveFlag = false;
		ObjectList[i].AnimFlag = false;
		ObjectList[i].X.L = 0;
		ObjectList[i].Y.L = 0;
		ObjectList[i].DX = 0;
		ObjectList[i].DY = 0;
		ObjectList[i].AnimsList = nil;
		ObjectList[i].PrevNode = nil;
		ObjectList[i].NextNode = nil;
		ObjectList[i].NodeNum = i;
	}

					/* INIT FREE NODE STACK */

	NodeStackFront = 0;
	for (i=0; i<MAX_OBJECTS; i++)
	{
		FreeNodeStack[i] = &ObjectList[i];
	}


					/* INIT UPDATE REGIONS */

	InitRegionList();							// init regions
}


/*********************** MAKE NEW OBJECT ******************/
//
// MAKE NEW OBJECT & RETURN PTR TO IT
//
// The linked list is sorted from LARGEST z to smallest!
//

ObjNode	*MakeNewObject(Byte genre,short x,short y,unsigned short z,void *moveCall)
{
register ObjNode	*newNodePtr,*scanNodePtr,*reNodePtr;


	if (NumObjects == (MAX_OBJECTS-1))			// check for overflow
		return(nil);

				/* INITIALIZE NEW NODE */

	newNodePtr = FreeNodeStack[NodeStackFront];	// get new node from stack
	NodeStackFront++;

		newNodePtr->MoveCall = moveCall;		// save move routine
		newNodePtr->Genre = genre;
		newNodePtr->Z = z;
		newNodePtr->X.Int = (long)x;
		newNodePtr->Y.Int = (long)y;
		newNodePtr->DrawFlag = false;
		newNodePtr->EraseFlag = false;
		newNodePtr->MoveFlag = true;
		newNodePtr->AnimFlag =
		newNodePtr->Flag0 =
		newNodePtr->Flag1 =
		newNodePtr->Flag2 =
		newNodePtr->Flag3 =
		newNodePtr->CType =						// must init ctype to something ( INVALID_NODE_FLAG might be set from last delete)
		newNodePtr->CBits =
		newNodePtr->DX =
		newNodePtr->DY = 0;
		newNodePtr->ShadowIndex =
		newNodePtr->OwnerToMessageNode =
		newNodePtr->MessageToOwnerNode = nil;

		newNodePtr->TopSide = newNodePtr->OldTopSide =
		newNodePtr->BottomSide = newNodePtr->OldBottomSide =
		newNodePtr->LeftSide = newNodePtr->OldLeftSide =
		newNodePtr->RightSide = newNodePtr->OldRightSide = 0;

		newNodePtr->ItemIndex = nil;					// assume it didnt come from ItemList

					/* FIND INSERTION PLACE FOR NODE */

	if (FirstNodePtr == nil)							// special case only entry
	{
		FirstNodePtr = newNodePtr;
		newNodePtr->PrevNode = nil;
		newNodePtr->NextNode = nil;
	}
	else if (z >= FirstNodePtr->Z)						// INSERT AS FIRST NODE
	{
		newNodePtr->PrevNode = nil;						// no prev
		newNodePtr->NextNode = FirstNodePtr; 		// next pts to old 1st
		FirstNodePtr->PrevNode = newNodePtr; 		// old pts to new 1st
		FirstNodePtr = newNodePtr;
	}
	else
	{
		reNodePtr = FirstNodePtr;
		scanNodePtr = FirstNodePtr;

		while (scanNodePtr != nil)
		{
			if (z >= scanNodePtr->Z)						// INSERT IN MIDDLE HERE
			{
				newNodePtr->NextNode = scanNodePtr;
				newNodePtr->PrevNode = reNodePtr;
				reNodePtr->NextNode = newNodePtr;
				scanNodePtr->PrevNode = newNodePtr;
				goto out;
			}
			reNodePtr = scanNodePtr;
			scanNodePtr=(ObjNode *)scanNodePtr->NextNode;	// try next node
		}

		newNodePtr->NextNode = nil;							// TAG TO END
		newNodePtr->PrevNode = reNodePtr;
		reNodePtr->NextNode = newNodePtr;
	}

out:
	NumObjects++;											// its done
	gMostRecentlyAddedNode = newNodePtr;					// remember this
	return(newNodePtr);
}


/*******************************  MOVE OBJECTS **************************/

void MoveObjects(void)
{
register	ObjNode		*thisNodePtr;


	if (FirstNodePtr == nil)								// see if there are any objects
		return;

	thisNodePtr = FirstNodePtr;

					/* MAIN NODE TASK LOOP */

	do
	{
		if ((thisNodePtr->MoveFlag) && (thisNodePtr->MoveCall != nil))
		{
			gThisNodePtr = thisNodePtr;						// set current object node
			gThisNodePtr->OldX = gThisNodePtr->X.Int;		// set old info
			gThisNodePtr->OldY = gThisNodePtr->Y.Int;
			gThisNodePtr->OldLeftSide = gThisNodePtr->LeftSide;
			gThisNodePtr->OldRightSide = gThisNodePtr->RightSide;
			gThisNodePtr->OldTopSide = gThisNodePtr->TopSide;
			gThisNodePtr->OldBottomSide = gThisNodePtr->BottomSide;

			thisNodePtr->MoveCall();						// call object's move routine
		}

		if ((thisNodePtr->AnimFlag) && (thisNodePtr->CType != INVALID_NODE_FLAG))
			AnimateASprite(thisNodePtr);					// animate the sprite

		thisNodePtr = (ObjNode *)thisNodePtr->NextNode;		// next node
	}
	while (thisNodePtr != nil);

}


/********************** ERASE OBJECTS **********************/

void EraseObjects(void)
{
register	ObjNode		*thisNodePtr;

	if (FirstNodePtr == nil)				// see if there are any objects
		return;

	thisNodePtr = FirstNodePtr;

				/* MAIN NODE TASK LOOP */

	do
	{
		if	(thisNodePtr->EraseFlag)
			EraseASprite(thisNodePtr);
		thisNodePtr = (ObjNode *)thisNodePtr->NextNode;
	}
	while (thisNodePtr != nil);
}


/**************************** DRAW OBJECTS ***************************/

void DrawObjects(void)
{
register	ObjNode		*thisNodePtr;

	if (FirstNodePtr == nil)				// see if there are any objects
		return;

	thisNodePtr = FirstNodePtr;

					/* MAIN NODE TASK LOOP */

	do
	{
		if (thisNodePtr->DrawFlag)
			DrawASprite(thisNodePtr);			// draw it
		thisNodePtr = (ObjNode *)thisNodePtr->NextNode;
	}while (thisNodePtr != nil);
}


/************************ INIT REGION LIST *****************/

void InitRegionList(void)
{
		numRegions = 0;
}


/*********************** ADD UPDATE REGION ****************/

void AddUpdateRegion(Rect theRegion,Byte clipNum)
{

	theRegion.left &= 0xfffffffc;						// on long boundary
	theRegion.right &= 0xfffffffc;

					/* CHECK BOUNDS */

	if ((theRegion.bottom < gRegionClipTop[clipNum]) ||			// see if completely off screen
		(theRegion.top > gRegionClipBottom[clipNum]) ||
		(theRegion.left > gRegionClipRight[clipNum]) ||
		(theRegion.right < gRegionClipLeft[clipNum]))
			return;


	if (theRegion.top < gRegionClipTop[clipNum])				// see if clip top
		theRegion.top = gRegionClipTop[clipNum];

	if (theRegion.bottom > gRegionClipBottom[clipNum])			// see if clip bottom
		theRegion.bottom = gRegionClipBottom[clipNum];

	if (theRegion.left < gRegionClipLeft[clipNum])				// see if clip left
		theRegion.left = gRegionClipLeft[clipNum];

	if (theRegion.right > gRegionClipRight[clipNum])			// see if clip right
		theRegion.right = gRegionClipRight[clipNum];

	if (theRegion.right < theRegion.left)				// check for cross overlapping
		theRegion.right = theRegion.left;

					/* ADD TO LIST */

	if (numRegions < MAX_REGIONS)				// make sure dont overflow list
	{
		regionList[numRegions++] = theRegion;
	}
}




/**************************** GET OBJECT INFO ***********************/

void GetObjectInfo(void)
{
	gDX = gThisNodePtr->DX;
	gDY = gThisNodePtr->DY;
	gX.L = gThisNodePtr->X.L;
	gY.L = gThisNodePtr->Y.L;
}

/************************** UPDATE OBJECT ***********************/

void UpdateObject(void)
{
	gThisNodePtr->X.L = gX.L;
	gThisNodePtr->Y.L = gY.L;
	gThisNodePtr->DX = gDX;
	gThisNodePtr->DY = gDY;

	gThisNodePtr->TopSide = gTopSide;
	gThisNodePtr->BottomSide = gBottomSide;
	gThisNodePtr->LeftSide = gLeftSide;
	gThisNodePtr->RightSide = gRightSide;
}


/******************** CALC OBJECT BOX *****************/


void CalcObjectBox(void)
{
	gTopSide = gY.Int+gThisNodePtr->TopOff;
	gBottomSide = gY.Int+gThisNodePtr->BottomOff;
	gLeftSide = gX.Int+gThisNodePtr->LeftOff;
	gRightSide = gX.Int+gThisNodePtr->RightOff;
}

/******************** CALC OBJECT BOX 2 *****************/
//
// v2 Calcs directly into node
//

void CalcObjectBox2(ObjNode *theNode)
{
	theNode->TopSide = (theNode->Y.Int)+theNode->TopOff;
	theNode->BottomSide = (theNode->Y.Int)+theNode->BottomOff;
	theNode->LeftSide = (theNode->X.Int)+theNode->LeftOff;
	theNode->RightSide = (theNode->X.Int)+theNode->RightOff;
}


/******************** DELETE ALL OBJECTS ********************/

void DeleteAllObjects(void)
{
	while (FirstNodePtr != nil)
		DeleteObject(FirstNodePtr);
}


/************************ DELETE OBJECT ****************/

void DeleteObject(ObjNode	*theNode)
{
ObjNode *tempNode;
Rect	box;

	if (theNode == nil)								// see if passed a bogus node
		return;

	if (theNode->CType == INVALID_NODE_FLAG)		// see if already deleted
	{
//#if BETA
//Str255	errString;		//-----------
//		DoAlert("Attempted to Double Delete an Object.  Object was already deleted!");
//		NumToString(theNode->Type,errString);		//------------
//		DoFatalAlert(errString);					//---------
//#else
		return;
//#endif
	}

					/* DO NODE SWITCHING */

	if (theNode->PrevNode == nil)					// special case 1st node
	{
		FirstNodePtr = (ObjNode *)theNode->NextNode;
		tempNode = (ObjNode *)theNode->NextNode;
		tempNode->PrevNode = nil;
	}
	else if (theNode->NextNode == nil)				// special case last node
	{
		tempNode = (ObjNode *)theNode->PrevNode;
		tempNode->NextNode = nil;
	}
	else											// generic middle deletion
	{
		tempNode = (ObjNode *)theNode->PrevNode;
		tempNode->NextNode = theNode->NextNode;
		tempNode = (ObjNode *)theNode->NextNode;
		tempNode->PrevNode = theNode->PrevNode;
	}

	NodeStackFront--;								// put node back on stack
	FreeNodeStack[NodeStackFront] = &ObjectList[theNode->NodeNum];

	NumObjects--;									// 1 less obj


					/* ERASE SPRITE OBJECT */

	if (theNode->EraseFlag)							// (assume only a sprite Obj would have this flag set)
	{
		if (!theNode->PFCoordsFlag)
		{
			EraseASprite(theNode);						// erase it
			box.left = theNode->drawBox.left;		// update it if not a playfield item
			box.right = theNode->drawBox.right;
			box.top = theNode->drawBox.top;
			box.bottom = theNode->drawBox.bottom;
			AddUpdateRegion(box,theNode->ClipNum);
		}
	}

	theNode->CType = INVALID_NODE_FLAG;				// INVALID_NODE_FLAG indicates its deleted


			/* SEE IF MAP ITEM NEEDS TO BE RE-ACTIVATED */

	if (theNode->ItemIndex != nil)
	{
		theNode->ItemIndex->type &= (-1)^ITEM_IN_USE; //~ITEM_IN_USE;	// clear in-use flag
	}

			/* SEE IF ZAP SHADOW */

	if (theNode->ShadowIndex != nil)
	{
		theNode->ShadowIndex->ShadowIndex = nil;	// make sure shadow doesn't have a shadow
		DeleteObject(theNode->ShadowIndex);
		theNode->ShadowIndex = nil;
	}

		/* SEE IF ZAP OWNER'S MESSAGE BALLOON */

	if (theNode->OwnerToMessageNode != nil)
	{
		theNode->OwnerToMessageNode->OwnerToMessageNode = nil;	// make sure message doesn't have a message
		DeleteObject(theNode->OwnerToMessageNode);
		theNode->OwnerToMessageNode = nil;
	}
}


/******************** MOVE OBJECT *******************/

void MoveObject(void)
{
	gY.L += gDY;
	gX.L += gDX;
}



/******************* DEACTIVATE OBJECT DRAW *****************/
//
// It is best to call this routine to turn off the drawing flag of
// an object.  It assures that the object will be erased via an Update Region.
//

void DeactivateObjectDraw(ObjNode *theNode)
{
static	Rect	box;

	theNode->DrawFlag = false;						// deactivate

	if (!theNode->PFCoordsFlag)
	{
		box.left = theNode->drawBox.left;			// erase existing image
		box.right = theNode->drawBox.right;
		box.top = theNode->drawBox.top;
		box.bottom = theNode->drawBox.bottom;
		AddUpdateRegion(box,theNode->ClipNum);
		EraseASprite(theNode);
	}
}


/****************** SORT OBJECTS BY Y *********************/
//
// Does a pseudo bubble sort based on an object's Y coord.
// Remember that list is in LARGEST to SMALLEST order, so Y coord is
// inversely related to Z coord.
//

void SortObjectsByY(void)
{
register	ObjNode 	*nodePtr,*nextNode;

	if (NumObjects < 2)									// see if anything to sort
		return;

	nodePtr = FirstNodePtr;								// start with 1st node

				/* SKIP Z'S WHICH ARE IN "FARTHEST" RANGE */

	while (nodePtr->Z >= FARTHEST_Z)
	{
		nodePtr = nodePtr->NextNode;
		if (nodePtr == nil)								// if end, then exit
			return;
	}


						/* SORT */

	while ((nextNode = nodePtr->NextNode) != nil)		// scan until end
	{
		if (nextNode->Z <= NEAREST_Z)					// stop if gets to "Nearest" range
			return;

		nodePtr->Z = (0x7FFF - nodePtr->Y.Int);			// Z = (MAXY - Y coord)

		if (nodePtr->Y.Int > nextNode->Y.Int)			// if this Y is below next, then must swap
		{
			nextNode->Z = (0x7FFF - nextNode->Y.Int);	// set this Z since about to get swapped
			if (nodePtr == FirstNodePtr)				// see if was 1st node
			{
				FirstNodePtr = nextNode;				// next is now the 1st node
				nextNode->PrevNode = nil;
				nodePtr->NextNode = nextNode->NextNode;
				nextNode->NextNode->PrevNode = nodePtr;	// next's next's prev now pts to current
				nodePtr->PrevNode = nextNode;
				nextNode->NextNode = nodePtr;
			}
			else
			{
				nextNode->PrevNode = nodePtr->PrevNode;
				nodePtr->PrevNode->NextNode	= nextNode;	// current's prev's next pts to next
				nodePtr->NextNode = nextNode->NextNode;
				nextNode->NextNode->PrevNode = nodePtr;	// next's next's prev now pts to current
				nodePtr->PrevNode = nextNode;
				nextNode->NextNode = nodePtr;
			}
		}
		nodePtr = nextNode;								// point to next node
	}
}


/******************** SIMPLE OBJECT MOVE ******************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void SimpleObjectMove(void)
{
	if (TrackItem())
		DeleteObject(gThisNodePtr);
}


#ifdef __MWERKS__
//=======================================================================================
//                 POWERPC CODE
//=======================================================================================

/********************* DUMP UPDATE REGIONS ***************/

void DumpUpdateRegions(void)
{
	if (numRegions == 0)
		return;


					/* UPDATE ALL OF THE REGIONS */

	for (int regionNum = 0; regionNum < numRegions; regionNum++)
	{
		int top		= regionList[regionNum].top;
		int bottom	= regionList[regionNum].bottom;
		int height	= bottom-top;

		int left	= 	regionList[regionNum].left;
		int right	=	regionList[regionNum].right;

		int width = ((right-left)>>2)+1;				// in longs
		width <<= 2;									// convert back to bytes

		GAME_ASSERT(top-OFFSCREEN_WINDOW_TOP >= 0);
		GAME_ASSERT(left-OFFSCREEN_WINDOW_LEFT >= 0);

		const uint8_t* srcPtr	= gOffScreenLookUpTable[top]+left;
		uint8_t* destPtr		= gScreenLookUpTable[top-OFFSCREEN_WINDOW_TOP] + left-OFFSCREEN_WINDOW_LEFT;

						/* DO THE QUICK COPY */

		if (!height)									// special check for 0 heights
			height = 1;

		do
		{
			memcpy(destPtr, srcPtr, width);

			destPtr += gScreenRowOffset;				// Bump to start of next row.
			srcPtr += OFFSCREEN_WIDTH;

		} while (--height);
	}

	numRegions = 0;								// reset # regions to 0

	PresentIndexedFramebuffer();
}


#else
//=======================================================================================
//                  68000 CODE
//=======================================================================================

/********************* DUMP UPDATE REGIONS ***************/

void DumpUpdateRegions(void)
{
register 	long		*destPtr,*srcPtr;
register	short			height,col;
static		long		*srcStartPtr,*destStartPtr;
			Byte		width;
			long		top,bottom,left,right;
			short			regionNum;

	if (numRegions == 0)
		return;

						/* GET SCREEN PIXMAP INFO */

//	gMMUMode = true32b;							// we must do this in 32bit addressing mode
//	SwapMMUMode(&gMMUMode);


					/* UPDATE ALL OF THE REGIONS */

	for (regionNum=0; regionNum<numRegions; regionNum++)
	{
		top = 	(long)regionList[regionNum].top;
		bottom = regionList[regionNum].bottom;
		height = bottom-top;

		left = 	regionList[regionNum].left;
		right =	regionList[regionNum].right;

		width = ((right-left)>>2)+1;

		if ((top-OFFSCREEN_WINDOW_TOP) < 0)
			SysBeep(0);
		if ((left-OFFSCREEN_WINDOW_LEFT) < 0)
			SysBeep(0);

		srcStartPtr = (long *)(gOffScreenLookUpTable[top]+left);
		destStartPtr = (long *)(gScreenLookUpTable[top-OFFSCREEN_WINDOW_TOP]+
						left-OFFSCREEN_WINDOW_LEFT);

						/* DO THE QUICK COPY */

		if (!height)									// special check for 0 heights
			height = 1;

		do
		{
			destPtr = destStartPtr;						// get line start ptrs
			srcPtr = srcStartPtr;

			col = (160-width)<<1;

			TODO_REWRITE_ASM();
#if 0		// TODO REWRITE ASM!
			asm
			{
					jmp		@inline(col)

				@inline
					move.l	(srcPtr)+,(destPtr)+	//0..15 longs
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+

					move.l	(srcPtr)+,(destPtr)+	//16..31
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+

					move.l	(srcPtr)+,(destPtr)+	//32..47
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+

					move.l	(srcPtr)+,(destPtr)+	//48..63
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+

					move.l	(srcPtr)+,(destPtr)+	//64..79
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+

					move.l	(srcPtr)+,(destPtr)+	//80..95
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+

					move.l	(srcPtr)+,(destPtr)+	//96..111
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+

					move.l	(srcPtr)+,(destPtr)+	//112..127
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+

					move.l	(srcPtr)+,(destPtr)+	//128..143
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+

					move.l	(srcPtr)+,(destPtr)+	//144..159
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+
					move.l	(srcPtr)+,(destPtr)+

			}
#endif

			destStartPtr += gScreenRowOffsetLW;				// Bump to start of next row.
			srcStartPtr += (OFFSCREEN_WIDTH>>2);

		} while (--height);

	} /* next region */

//	SwapMMUMode(&gMMUMode);						// Restore addressing mode
	numRegions = 0;								// reset # regions to 0
}


#endif