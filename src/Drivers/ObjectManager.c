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
#include "externs.h"

/****************************/
/*    CONSTANTS             */
/****************************/

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

MikeFixed	gX;
MikeFixed	gY;

Boolean		gDiscreteMovementFlag;

long		gRegionClipTop[MAX_CLIP_REGIONS],gRegionClipBottom[MAX_CLIP_REGIONS],
			gRegionClipLeft[MAX_CLIP_REGIONS],gRegionClipRight[MAX_CLIP_REGIONS];


/************************ INIT CLIPPING REGIONS **********************/

void InitClipRegions(void)
{
	for (int i = 0; i < MAX_CLIP_REGIONS; i++)				// init ALL clipping areas to full buffer size
	{
		gRegionClipTop[i] = OFFSCREEN_WINDOW_TOP;
		gRegionClipBottom[i] = OFFSCREEN_WINDOW_TOP+VISIBLE_HEIGHT-1;
		gRegionClipLeft[i] = OFFSCREEN_WINDOW_LEFT;
		gRegionClipRight[i] = OFFSCREEN_WINDOW_LEFT+VISIBLE_WIDTH-1;
	}
}

/************************ INIT OBJECT MANAGER **********************/

void InitObjectManager(void)
{
					/* INIT CLIPPING REGIONS */

	InitClipRegions();


				/* INIT LIKED LIST */


	if (ObjectList == nil)								// see if need to allocate memory for object list
	{
		Handle ObjHandle = NewHandleClear(sizeof(ObjNode)*MAX_OBJECTS);
		GAME_ASSERT(ObjHandle);
		ObjectList = (ObjNode *)*ObjHandle;
	}


	gThisNodePtr = nil;

					/* CLEAR ENTIRE OBJECT LIST */

	FirstNodePtr = nil;									// no node yet
	NumObjects = 0;
	for (int i = 0; i < MAX_OBJECTS; i++)
	{
		// No need to init most fields to 0 since we used NewHandleClear.
		ObjectList[i].NodeNum = i;
	}

					/* INIT FREE NODE STACK */

	NodeStackFront = 0;
	for (int i = 0; i < MAX_OBJECTS; i++)
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

ObjNode	*MakeNewObject(Byte genre, short x, short y, unsigned short z, void (*moveCall)(void))
{
register ObjNode	*newNodePtr,*scanNodePtr,*reNodePtr;


	if (NumObjects == (MAX_OBJECTS-1))			// check for overflow
		return(nil);

				/* INITIALIZE NEW NODE */

	newNodePtr = FreeNodeStack[NodeStackFront];	// get new node from stack
	NodeStackFront++;

	long nodeNumBackup = newNodePtr->NodeNum;	// back up node number before zeroing out record

	memset(newNodePtr, 0, sizeof(ObjNode));		// set all fields to 0

	newNodePtr->NodeNum = nodeNumBackup;		// restore node number

		newNodePtr->MoveCall = moveCall;		// save move routine
		newNodePtr->Genre = genre;
		newNodePtr->X.Int = (long)x;
		newNodePtr->Y.Int = (long)y;
		newNodePtr->Z = z;
		newNodePtr->DrawFlag = false;
		newNodePtr->EraseFlag = false;
		newNodePtr->MoveFlag = true;
		newNodePtr->CType = 0;					// must init ctype to something ( INVALID_NODE_FLAG might be set from last delete)
		newNodePtr->CBits = 0;
		newNodePtr->ItemIndex = nil;			// assume it didnt come from ItemList

		newNodePtr->OldX.Int = (long)x;
		newNodePtr->OldY.Int = (long)y;

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
static ObjNode*	nodesToMove[MAX_OBJECTS];
int numNodesToMove = 0;

	if (FirstNodePtr == nil)								// see if there are any objects
		return;

					/* FREEZE LIST OF OBJECTS THAT NEED TO BE UPDATED */
					//
					// An ObjNode's move routine may insert/delete nodes in the global linked list.
					// This may modify the order of the nodes in the list, so we must ensure
					// not to update any given ObjNode more than once.
					//

	for (ObjNode* node = FirstNodePtr; node != nil; node = node->NextNode)
	{
		if (node->CType == INVALID_NODE_FLAG)
			continue;

		nodesToMove[numNodesToMove] = node;
		numNodesToMove++;
	}

					/* MOVE THE OBJECTS */

	for (int i = 0; i < numNodesToMove; i++)
	{
		ObjNode* node = nodesToMove[i];

		if (node->CType == INVALID_NODE_FLAG)		// node was deleted by another node's move routine
			continue;

		if (node->MoveFlag && node->MoveCall != nil)
		{
			gThisNodePtr = node;					// set current object node

			gThisNodePtr->OldX = gThisNodePtr->X;	// set old info
			gThisNodePtr->OldY = gThisNodePtr->Y;
			gThisNodePtr->OldYOffset = gThisNodePtr->YOffset;
			gThisNodePtr->OldLeftSide = gThisNodePtr->LeftSide;
			gThisNodePtr->OldRightSide = gThisNodePtr->RightSide;
			gThisNodePtr->OldTopSide = gThisNodePtr->TopSide;
			gThisNodePtr->OldBottomSide = gThisNodePtr->BottomSide;

			node->MoveCall();						// call object's move routine

			if (node->CType == INVALID_NODE_FLAG)	// move routine may have caused object to kill itself
				continue;
		}

		if (node->AnimFlag)
			AnimateASprite(node);					// animate the sprite
	}
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
	gDiscreteMovementFlag = false;
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

	if (gDiscreteMovementFlag)			// prevent movement interpolation
	{
		gThisNodePtr->OldX = gX;
		gThisNodePtr->OldY = gY;
	}
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
		DoAlert("Attempted to Double Delete an Object.  Object was already deleted!");
		return;
	}

					/* DO NODE SWITCHING */

	if (theNode->PrevNode == nil)					// special case 1st node
	{
		FirstNodePtr = (ObjNode *)theNode->NextNode;
		tempNode = (ObjNode *)theNode->NextNode;
		if (tempNode)
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


/************************** STOP OBJECT MOVEMENT ***********************/
//
// This stops movement extrapolation (preventing jitter on static objects)
// until a new non-0 speed is set on the object.
//

void StopObjectMovement(ObjNode* objNode)
{
	objNode->DX = 0;
	objNode->DY = 0;
	objNode->DZ = 0;

	if (objNode == gThisNodePtr)
	{
		gDX = 0;
		gDY = 0;
	}
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
				if (nextNode->NextNode)
					nextNode->NextNode->PrevNode = nodePtr;	// next's next's prev now pts to current
				nodePtr->PrevNode = nextNode;
				nextNode->NextNode = nodePtr;
			}
			else
			{
				nextNode->PrevNode = nodePtr->PrevNode;
				nodePtr->PrevNode->NextNode	= nextNode;	// current's prev's next pts to next
				nodePtr->NextNode = nextNode->NextNode;
				if (nextNode->NextNode)
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


/********************* DUMP UPDATE REGIONS (DON'T PRESENT FRAMEBUFFER) ***************/

void DumpUpdateRegions_DontPresentFramebuffer(void)
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

			destPtr += VISIBLE_WIDTH;					// Bump to start of next row.
			srcPtr += OFFSCREEN_WIDTH;

		} while (--height);
	}

	numRegions = 0;								// reset # regions to 0
}

/********************* DUMP UPDATE REGIONS (AND PRESENT FRAMEBUFFER) ***************/

void DumpUpdateRegions(void)
{
	DumpUpdateRegions_DontPresentFramebuffer();
	PresentIndexedFramebuffer();
}



/********************* INITIALIZE Y OFFSET ON A NEW OBJNODE ***************/
//
// Call this on a fresh ObjNode (rather than setting YOffset manually)
// to avoid a hiccup the first time the object's position is interpolated.
//

void InitYOffset(ObjNode* objNode, long yOffset)
{
	objNode->YOffset.L = objNode->OldYOffset.L = yOffset << 16;
}


/********************* INTERPOLATED OBJECT POSITION ***************/
//
// If the current graphics frame falls on a simulation tick, the returned position is exact;
// otherwise it is interpolated from the object's previous position.
// Static objects (without a MoveCall or MoveFlag) are not interpolated.
//

void TweenObjectPosition(ObjNode* node, int32_t* x, int32_t* y)
{
	if (	!node->MoveFlag								// the node might not have valid old coords
		||	!node->MoveCall								// the node might not have valid old coords
		||	gTweenFrameFactor.L >= 0x10000)				// or, no interpolation necessary on final position
	{
		*x = node->X.Int;
		*y = Fix32_Int(node->Y.L + node->YOffset.L);
	}
	else if (gTweenFrameFactor.L == 0)						// No extrapolation necessary on initial position
	{
		*x = node->OldX.Int;								// get short x coord (world)
		*y = Fix32_Int(node->OldY.L + node->OldYOffset.L);	// get foot y and add any y adjustment offset
	}
	else
	{
		int32_t newX = node->X.L;
		int32_t oldX = node->OldX.L;
		int32_t newY = node->Y.L	+ node->YOffset.L;
		int32_t oldY = node->OldY.L	+ node->OldYOffset.L;
		*x = Fix32_Int(Fix32_Mul(gOneMinusTweenFrameFactor.L, oldX) + Fix32_Mul(gTweenFrameFactor.L, newX));
		*y = Fix32_Int(Fix32_Mul(gOneMinusTweenFrameFactor.L, oldY) + Fix32_Mul(gTweenFrameFactor.L, newY));
	}
}
