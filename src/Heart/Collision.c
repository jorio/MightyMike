/****************************/
/*   COLLISION ROUTINES     */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/

#include "myglobals.h"
#include "playfield.h"
#include "object.h"
#include "collision.h"

extern	unsigned	short	**gPlayfield;
extern	short				*gTileXlatePtr;
extern	TileAttribType	*gTileAttributes;
extern	long			gSumDX,gSumDY;
extern	long			gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	short			gPlayfieldWidth,gPlayfieldHeight;
extern	ObjNode		*FirstNodePtr;
extern	ObjNode		*gThisNodePtr;
extern	union_gX;
extern	union_gY;


/****************************/
/*    CONSTANTS             */
/****************************/

#define	MAX_COLLISIONS		40



/****************************/
/*    VARIABLES             */
/****************************/


CollisionRec	gCollisionList[MAX_COLLISIONS];
short			gNumCollisions = 0;
Byte			gTotalSides;


/******************* COLLISION DETECT *********************/

void CollisionDetect(ObjNode *baseNode,unsigned long CType)
{
register	ObjNode 	*thisNode;
register	long		sideBits,cBits;
register	long		relDX,relDY;

	gNumCollisions = 0;							// clear list
	gTotalSides = 0;

	if (CType == CTYPE_BGROUND)					// see if only do BG collisions
	{
		AddBGCollisions(baseNode);
		return;
	}

				/*******************************/
				/* DO SPRITE/OBJECT COLLISIONS */
				/*******************************/


	thisNode = FirstNodePtr;						// start on 1st node

loop:
	if (!(thisNode->CType & CType))					// see if we want to check this Type
		goto next;

	if (!thisNode->CBits)							// see if this obj doesn't need collisioning
		goto next;

	if (thisNode == baseNode)						// dont collide against itself
		goto next;



					/* DO RECTANGLE INTERSECTION */

	if (gRightSide < thisNode->LeftSide)
		goto next;

	if	(gLeftSide > thisNode->RightSide)
		goto next;

	if	(gTopSide > thisNode->BottomSide)
		goto next;

	if (gBottomSide < thisNode->TopSide)
		goto next;


			/* THERE HAS BEEN A COLLISION SO CHECK WHICH SIDE PASSED THRU */

	sideBits = 0;
	cBits = thisNode->CBits;					// get collision info bits

	if (cBits & CBITS_TOUCHABLE)				// if it's generically touchable, then add it without side info
		goto	got_sides;

	relDX = gSumDX - thisNode->DX;				// calc relative deltas
	relDY = gSumDY - thisNode->DY;


					/* CHECK BOTTOM COLLISION */


	if ((cBits & SIDE_BITS_TOP) && (relDY > 0))			// see if target has solid top & we are going relatively down
	{
		if (baseNode->BottomSide < thisNode->OldTopSide)		// get old source bottom & see if already was in target
			if ((gBottomSide >= thisNode->TopSide) &&			// see if currently in target
				(gBottomSide <= thisNode->BottomSide))
				sideBits = SIDE_BITS_BOTTOM;
		goto check_sides;
	}

						/* CHECK TOP COLLISION */

	if ((cBits & SIDE_BITS_BOTTOM) && (relDY < 0))			// see if target has solid bottom & we are going relatively up
	{
		if (baseNode->TopSide > thisNode->OldBottomSide)	// get old source top & see if already was in target
			if ((gTopSide <= thisNode->BottomSide) &&		// see if currently in target
				(gTopSide >= thisNode->TopSide))
				sideBits = SIDE_BITS_TOP;
	}


check_sides:

					/* CHECK RIGHT COLLISION */


	if ((cBits & SIDE_BITS_LEFT) && (relDX > 0))			// see if target has solid left & we are going relatively right
	{
		if (baseNode->RightSide < thisNode->OldLeftSide)	// get old source right & see if already was in target
			if ((gRightSide >= thisNode->LeftSide) &&		// see if currently in target
				(gRightSide <= thisNode->RightSide))
				sideBits |= SIDE_BITS_RIGHT;
		goto end_sides;
	}

						/* CHECK COLLISION ON LEFT */

	if ((cBits & SIDE_BITS_RIGHT) && (relDX < 0))			// see if target has solid right & we are going relatively left
	{
		if (baseNode->LeftSide > thisNode->OldRightSide)	// get old source left & see if already was in target
			if ((gLeftSide <= thisNode->RightSide) &&		// see if currently in target
				(gLeftSide >= thisNode->LeftSide))
				sideBits |= SIDE_BITS_LEFT;
	}


					 /* SEE IF ANYTHING TO ADD */

end_sides:
	if (!sideBits)											// see if anything actually happened
		goto next;

got_sides:
	gCollisionList[gNumCollisions].sides = sideBits;		// add to collision list
	gCollisionList[gNumCollisions].type = COLLISION_TYPE_OBJ;
	gCollisionList[gNumCollisions].objectPtr = thisNode;
	gNumCollisions++;
	gTotalSides |= sideBits;								// remember total of this

next:
	if ((thisNode = thisNode->NextNode) != nil)				// next target node
		goto loop;


				/*******************************/
				/*   DO BACKGROUND COLLISIONS  */
				/*******************************/

	if (CType & CTYPE_BGROUND)					// see if do BG collision
		AddBGCollisions(baseNode);

}


/**************** ADD BG COLLISIONS *********************/
//
// Add BG Collisions to Collision List.  Called by CollisionDetect above.
//

static void AddBGCollisions(ObjNode *theNode)
{
//Boolean	TLCornerFlag,BLCornerFlag,TRCornerFlag,BRCornerFlag;
short		oldRow,left,right,oldCol,top,bottom;
register	short		count,num;
register	unsigned short		tileNum,bits;

//	TLCornerFlag = BLCornerFlag = TRCornerFlag = BRCornerFlag = 0;	// assume no corner hits

	if (gBottomSide >= gPlayfieldHeight)			// see if bottom is off of map
		return;
	if (gTopSide < 0)								// see if top is off of map
		return;
	if (gRightSide >= gPlayfieldWidth)				// see if right is off of map
		return;
	if (gLeftSide < 0)								// see if left is off of map
		return;


				/************************/
				/* CHECK VERTICAL SIDES */
				/************************/

	if (gSumDY > 0)										// see if down
	{
				/* SEE IF BOTTOM SIDE HIT BACKGROUND */


		oldRow = theNode->OldBottomSide>>TILE_SIZE_SH;	// get old bottom side & see if in same row as current

		left = gLeftSide>>TILE_SIZE_SH;
		right = gRightSide>>TILE_SIZE_SH;
		count = num = (right-left)+1;					// calc # of tiles wide to check.  num = original count value

		bottom = gBottomSide>>TILE_SIZE_SH;
		if (bottom == oldRow)							// if in same row as before,then skip
			goto check_x;

		for (; count > 0; count--)
		{
			tileNum = gPlayfield[bottom][left]&TILENUM_MASK;	// get tile #
			bits = gTileAttributes[tileNum].bits;				// get bit attributes for the tile
			if (bits & TILE_ATTRIB_TOPSOLID)					// see if tile solid on top
			{
				tileNum = gPlayfield[bottom-1][left]&TILENUM_MASK;	// get tile above this tile & see if ignore solidity
				bits = gTileAttributes[tileNum].bits;
				if (!(bits & TILE_ATTRIB_BOTTOMSOLID))			// see if adjacent tile is solid on bottom
				{
					gCollisionList[gNumCollisions].sides = SIDE_BITS_BOTTOM;
					gCollisionList[gNumCollisions].type = COLLISION_TYPE_TILE;
					gNumCollisions++;
					gTotalSides |= SIDE_BITS_BOTTOM;

//					if (count == 1)						// see if its a corner tile
//						BRCornerFlag = true;
//					else
//					if (count == num)
//						BLCornerFlag = true;
					goto check_x;
				}
			}
			left++;										// next column ->
		}
	}
	else
	if (gSumDY < 0)										// see if up
	{
						/* SEE IF TOP SIDE HIT BACKGROUND */

		oldRow = theNode->OldTopSide>>TILE_SIZE_SH;		// get old top side & see if in same row as current

		left = gLeftSide>>TILE_SIZE_SH;
		right = gRightSide>>TILE_SIZE_SH;
		count = num = (right-left)+1;					// calc # of tiles wide to check.  num = original count value

		top = gTopSide>>TILE_SIZE_SH;
		if (top == oldRow)								// if in same row as before,then skip
			goto check_x;

		for (; count > 0; count--)
		{
			tileNum = gPlayfield[top][left]&TILENUM_MASK;	// get tile #
			bits = gTileAttributes[tileNum].bits;			// get bit attributes for the tile
			if (bits & TILE_ATTRIB_BOTTOMSOLID)				// see if tile solid on bottom
			{
				tileNum = gPlayfield[top+1][left]&TILENUM_MASK;	// get tile below this tile & see if ignore solidity
				bits = gTileAttributes[tileNum].bits;
				if (!(bits & TILE_ATTRIB_TOPSOLID))			// see if adjacent tile is solid on top
				{
					gCollisionList[gNumCollisions].sides = SIDE_BITS_TOP;
					gCollisionList[gNumCollisions].type = COLLISION_TYPE_TILE;
					gNumCollisions++;
					gTotalSides |= SIDE_BITS_TOP;

//					if (count == 1)						// see if its a corner tile
//						TRCornerFlag = true;
//					else
//					if (count == num)
//						TLCornerFlag = true;
					goto check_x;
				}
			}
			left++;										// next column ->
		}
	}

				/************************/
				/* DO HORIZONTAL CHECK  */
				/************************/

check_x:

	if (gSumDX > 0)									// see if right
	{
				/* SEE IF RIGHT SIDE HIT BACKGROUND */

		oldCol = theNode->OldRightSide>>TILE_SIZE_SH;	// get old right side & see if in same col as current

		top = gTopSide>>TILE_SIZE_SH;
		bottom = gBottomSide>>TILE_SIZE_SH;
		count = num = (bottom-top)+1;				// calc # of tiles high to check.  num = original count value

		right = gRightSide>>TILE_SIZE_SH;
		if (right == oldCol)						// if in same col as before,then skip
			return;

		for (; count > 0; count--)
		{
			tileNum = gPlayfield[top][right]&TILENUM_MASK;	// get tile #
			bits = gTileAttributes[tileNum].bits;			// get bit attributes for the tile
			if (bits & TILE_ATTRIB_LEFTSOLID)				// see if tile solid on left
			{
				tileNum = gPlayfield[top][right-1]&TILENUM_MASK;	// get tile to left of this tile & see if ignore solidity
				bits = gTileAttributes[tileNum].bits;
				if (!(bits & TILE_ATTRIB_RIGHTSOLID))		// see if adjacent tile is solid on right
				{
					gCollisionList[gNumCollisions].sides = SIDE_BITS_RIGHT;
					gCollisionList[gNumCollisions].type = COLLISION_TYPE_TILE;
					gNumCollisions++;
					gTotalSides |= SIDE_BITS_RIGHT;

//					if (count == 1)						// see if its a corner tile
//						BRCornerFlag = true;
//					else
//					if (count == num)
//						TRCornerFlag = true;
					return;
				}
			}
			top++;										// next row
		}
	}
	else
	if (gSumDX < 0)										// see if left
	{
						/* SEE IF LEFT SIDE HIT BACKGROUND */

		oldCol = theNode->OldLeftSide>>TILE_SIZE_SH;	// get old left side & see if in same col as current

		top = gTopSide>>TILE_SIZE_SH;
		bottom = gBottomSide>>TILE_SIZE_SH;
		count = num = (bottom-top)+1;					// calc # of tiles high to check.  num = original count value

		left = gLeftSide>>TILE_SIZE_SH;
		if (left == oldCol)								// if in same col as before,then skip
			return;

		for (; count > 0; count--)
		{
			tileNum = gPlayfield[top][left]&TILENUM_MASK;	// get tile #
			bits = gTileAttributes[tileNum].bits;			// get bit attributes for the tile
			if (bits & TILE_ATTRIB_RIGHTSOLID)				// see if tile solid on right
			{
				tileNum = gPlayfield[top][left+1]&TILENUM_MASK;	// get tile to right of this tile & see if ignore solidity
				bits = gTileAttributes[tileNum].bits;
				if (!(bits & TILE_ATTRIB_LEFTSOLID))			// see if adjacent tile is solid on left
				{
					gCollisionList[gNumCollisions].sides = SIDE_BITS_LEFT;
					gCollisionList[gNumCollisions].type = COLLISION_TYPE_TILE;
					gNumCollisions++;
					gTotalSides |= SIDE_BITS_LEFT;

//					if (count == 1)						// see if its a corner tile
//						BLCornerFlag = true;
//					else
//					if (count == num)
//						TLCornerFlag = true;
					return;
				}
//				else
//					SysBeep(0);	//----------
			}
			top++;										// next row
		}
	}
}


/***************** HANDLE COLLISIONS ********************/
//
// This is a generic collision handler.  Takes care of
// all processing.
//
// INPUT:  cType = CType bit mask for collision matching
//			gSumDX,DY = set
//			T/B/L/R Sides all set.
//
// OUTPUT: totalSides
//		   new T/B/L/R
//

Byte HandleCollisions(unsigned long	cType)
{
register	Byte	totalSides;
register	short	i;
short			originalX,originalY,offset;

	CollisionDetect(gThisNodePtr,cType);					// get collision info

	originalX = gX.Int;										// remember starting coords
	originalY = gY.Int;
	totalSides = 0;

	for (i=0; i < gNumCollisions; i++)						// handle all collisions
	{
		totalSides |= gCollisionList[i].sides;				// keep sides info

		if (gCollisionList[i].type == COLLISION_TYPE_TILE)
		{
					/**************************/
					/* HANDLE TILE COLLISIONS */
					/**************************/

			if (gCollisionList[i].sides & SIDE_BITS_TOP)	// SEE IF HIT TOP
			{
				offset = TILE_SIZE-(gTopSide&0x1f);			// see how far over it went
				gY.Int = originalY+offset;					// adjust y coord
			}
			else
			if (gCollisionList[i].sides & SIDE_BITS_BOTTOM)	// SEE IF HIT BOTTOM
			{
				offset = (gBottomSide&0x1f)+1;				// see how far over it went
				gY.Int = originalY-offset;					// adjust y coord
			}


			if (gCollisionList[i].sides & SIDE_BITS_LEFT)	// SEE IF HIT LEFT
			{
				offset = TILE_SIZE-(gLeftSide&0x1f);			// see how far over it went
				gX.Int = originalX+offset;					// adjust x coord
			}
			else
			if (gCollisionList[i].sides & SIDE_BITS_RIGHT)	// SEE IF HIT RIGHT
			{
				offset = (gRightSide&0x1f)+1;				// see how far over it went
				gX.Int = originalX-offset;					// adjust x coord
			}

		}
		else
		if (gCollisionList[i].type == COLLISION_TYPE_OBJ)
		{
					/****************************/
					/* HANDLE OBJECT COLLISIONS */
					/****************************/

			if (gCollisionList[i].sides & SIDE_BITS_TOP)	// SEE IF HIT TOP
			{
				offset = (gCollisionList[i].objectPtr->BottomSide-gTopSide)+1;	// see how far over it went
				gY.Int = originalY+offset;					// adjust y coord
			}
			else
			if (gCollisionList[i].sides & SIDE_BITS_BOTTOM)	// SEE IF HIT BOTTOM
			{
				offset = (gBottomSide-gCollisionList[i].objectPtr->TopSide)+1;	// see how far over it went
				gY.Int = originalY-offset;					// adjust y coord
			}


			if (gCollisionList[i].sides & SIDE_BITS_LEFT)	// SEE IF HIT LEFT
			{
				offset = (gCollisionList[i].objectPtr->RightSide-gLeftSide)+1;	// see how far over it went
				gX.Int = originalX+offset;					// adjust x coord
			}
			else
			if (gCollisionList[i].sides & SIDE_BITS_RIGHT)	// SEE IF HIT RIGHT
			{
				offset = (gRightSide-gCollisionList[i].objectPtr->LeftSide)+1;	// see how far over it went
				gX.Int = originalX-offset;					// adjust x coord
			}
		}
	}
	CalcObjectBox();
	return(totalSides);
}


/******************** DO SIMPLE COLLISION *****************/
//
// Just does simple sprite box collision detect.  No side verification or background checks.
//
// INPUT: gThisNodePtr = source object
//		 cTypes = CType bit mask for collision matching

void DoSimpleCollision(unsigned long cTypes)
{
register	ObjNode		*targetNodePtr;

	gNumCollisions = 0;										// assume no collisions

	if (FirstNodePtr == nil)								// see if there are any objects
		return;

	targetNodePtr = FirstNodePtr;							// start on 1st node

					/* SCAN LOOP */

	do
	{
		if ((targetNodePtr->CType & cTypes) &&				// check for matching ctype
			(targetNodePtr != gThisNodePtr))				// cant collide against itself
		{
			if (targetNodePtr->TopSide > gBottomSide)		// box collision
				goto next;
			if	(targetNodePtr->BottomSide < gTopSide)
				goto next;
			if	(targetNodePtr->LeftSide > gRightSide)
				goto next;
			if	(targetNodePtr->RightSide < gLeftSide)
				goto next;

					/* A COLLISION OCCURED */

			gCollisionList[gNumCollisions].objectPtr = targetNodePtr;
			gCollisionList[gNumCollisions].sides = 0;
			gCollisionList[gNumCollisions].type = COLLISION_TYPE_OBJ;
			gNumCollisions++;
		}

next:
		targetNodePtr = (ObjNode *)targetNodePtr->NextNode;	// next node
	}
	while (targetNodePtr != nil);
}


/******************** DO POINT COLLISION *****************/
//
// INPUT: x,y = coords to check
//		 cTypes = CType bit mask for collision matching
//
// OUTPUT: NumCollisions = # of collisions
// 			true if any collisions at all

Boolean DoPointCollision(unsigned short x, unsigned short y, unsigned long cTypes)
{
register	ObjNode		*targetNodePtr;
register	unsigned	short			tileNum;
register	Byte		bits;								// only care about 8 bits worth of collision info

	if ((y >= gPlayfieldHeight) || (x >= gPlayfieldWidth))	// check for bounds error
		return(false);

	gNumCollisions = 0;										// assume no collisions

	if (FirstNodePtr == nil)								// see if there are any objects
		return(false);

	targetNodePtr = FirstNodePtr;							// start on 1st node


					/* OBJECT SCAN LOOP */

	do
	{
		if (targetNodePtr->CType & cTypes)					// check for matching ctype
		{
			if  (x > targetNodePtr->RightSide)				// see if point within object box
				goto next;
			if 	(x < targetNodePtr->LeftSide)
				goto next;
			if	(y > targetNodePtr->BottomSide)
				goto next;
			if (y < targetNodePtr->TopSide)
				goto next;

			gCollisionList[gNumCollisions].objectPtr = targetNodePtr;	// remember collision info
			gCollisionList[gNumCollisions].sides = 0;
			gCollisionList[gNumCollisions].type = COLLISION_TYPE_OBJ;
			gNumCollisions++;
		}

next:
		targetNodePtr = (ObjNode *)targetNodePtr->NextNode;	// next node
	}
	while (targetNodePtr != nil);

					/* CHECK BACKGROUND */

	if (cTypes & CTYPE_BGROUND)
	{
		tileNum = gPlayfield[y>>TILE_SIZE_SH][x>>TILE_SIZE_SH]&TILENUM_MASK;
		bits = gTileAttributes[tileNum].bits;
		if (bits & ALL_SOLID_SIDES)								// see if anything solid here
		{
			gCollisionList[gNumCollisions].sides = bits;
			gCollisionList[gNumCollisions].type = COLLISION_TYPE_TILE;
			gNumCollisions++;
		}
	}
	return (gNumCollisions>0);
}

