/****************************/
/*     SHAPE MANAGER        */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/
#include "myglobals.h"
#include "windows.h"
#include "playfield.h"
#include "object.h"
#include "misc.h"
#include "shape.h"

extern	Handle		gOffScreenHandle;
extern	Handle		gBackgroundHandle;
extern	long			gRightSide,gLeftSide,gTopSide,gBottomSide;
extern	GDHandle		gMainScreen;
extern	PixMapHandle	gMainScreenPixMap;
extern	char  			gMMUMode;
extern	long			gScreenRowOffsetLW,gScreenRowOffset;
extern	long			gScreenLookUpTable[VISIBLE_HEIGHT];
extern	long			gOffScreenLookUpTable[OFFSCREEN_HEIGHT];
extern	long			gBackgroundLookUpTable[OFFSCREEN_HEIGHT];
extern	long			gScrollX,gScrollY;
extern	long			gRegionClipTop[],gRegionClipBottom[],
						gRegionClipLeft[],gRegionClipRight[];
extern	long			*gPFLookUpTable;
extern	long			*gPFCopyLookUpTable;
extern	unsigned		short	**gPlayfield;
extern	long			*gPFMaskLookUpTable;
extern	unsigned char	gInterlaceMode;
extern	short			gPlayfieldWidth,gPlayfieldHeight;

#if __USE_PF_VARS
extern	long PF_TILE_HEIGHT;
extern	long PF_TILE_WIDTH;
extern	long PF_WINDOW_TOP;
extern	long PF_WINDOW_LEFT;
#endif
/****************************/
/*    PROTOTYPES            */
/****************************/

static void ErasePFSpriteInterlaced(ObjNode *theNodePtr);


/****************************/
/*    CONSTANTS             */
/****************************/



			/* *destPtr++ = (*destPtr & (*maskPtr++)) | (*srcPtr++); */

#define		MASK_MAC	MOVE.L    (destPtr),D0		\
						AND.L     (maskPtr)+,D0		\
						OR.L      (srcPtr)+,D0		\
						MOVE.L    D0,(destPtr)+


		/* *destPtr++ = (*destPtr & (*maskPtr++ | *tileMaskPtr)) | (*srcPtr++ & (*tileMaskPtr++ ^ 0xffffffffL); */

#define		MASK_MAC2	move.l	(maskPtr)+,d0	\
						or.l	(a0),d0			\
						and.l	(destPtr),d0	\
						move.l	(a0)+,d1		\
						not.l	d1				\
						and.l	(srcPtr)+,d1	\
						or.l	d1,d0			\
						MOVE.L    D0,(destPtr)+


#define	ILD(dest_,src_,width_)		\
			switch(width_)					\
			{								\
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

Handle	gShapeTableHandle[MAX_SHAPE_GROUPS] = {nil,nil,nil,nil,nil,nil,nil,nil};

Ptr		gSHAPE_HEADER_Ptrs[MAX_SHAPE_GROUPS][MAX_SHAPES_IN_FILE];			// holds ptr to each shape type's SHAPE_HEADER

static	short		gNumShapesInFile[MAX_SHAPE_GROUPS];

ObjNode	*gMostRecentShape = nil;


/************************ MAKE NEW SHAPE ***********************/
//
// MAKE NEW SHAPE OBJECT & RETURN PTR TO IT
//

ObjNode *MakeNewShape(long groupNum,long type,long subType,short x, short y, short z,void *moveCall,Boolean pfRelativeFlag)
{
ObjNode	*newSpritePtr;
Ptr		tempPtr;
long	offset;

	if (groupNum >= MAX_SHAPE_GROUPS)										// see if legal group
		DoFatalAlert("\pIllegal shape group #");

	newSpritePtr = MakeNewObject(SPRITE_GENRE,x,y,z,moveCall);

	if (newSpritePtr == nil)								// return nil if no object
		return(nil);

	newSpritePtr->Type = type;								// set sprite type
	newSpritePtr->SubType = subType;						// set sprite subtype
	newSpritePtr->SpriteGroupNum = groupNum;				// set which sprite group it belongs to
	newSpritePtr->UpdateBoxFlag = !pfRelativeFlag;			// no box if PF relative
	newSpritePtr->DrawFlag = 								// init flags
	newSpritePtr->EraseFlag =
	newSpritePtr->AnimFlag =
	newSpritePtr->TileMaskFlag = true;						// use tile masks (if applicable)
	newSpritePtr->PFCoordsFlag = pfRelativeFlag;			// set if playfield coords or not
	newSpritePtr->YOffset.L = 0;							// assume no draw offset

	if (newSpritePtr->PFCoordsFlag)
	{
		newSpritePtr->drawBox.left =						// init draw box coords
		newSpritePtr->drawBox.right =
		newSpritePtr->drawBox.top =
		newSpritePtr->drawBox.bottom = 0;
	}
	else
	{
		newSpritePtr->drawBox.left = x-2;
		newSpritePtr->drawBox.right = x+2;
		newSpritePtr->drawBox.top = y-2;
		newSpritePtr->drawBox.bottom = y+2;
	}

	newSpritePtr->AnimSpeed = 								// init animation stuff
	newSpritePtr->AnimConst = 0x100;
	newSpritePtr->AnimLine =
	newSpritePtr->CurrentFrame =							// set to 0 just to be safe!!!
	newSpritePtr->AnimCount = 0;

	newSpritePtr->DZ = 0;

	newSpritePtr->ClipNum = CLIP_REGION_PLAYFIELD;			// assume clip to playfield

	newSpritePtr->SHAPE_HEADER_Ptr = tempPtr =
				gSHAPE_HEADER_Ptrs[groupNum][type];			// set ptr to SHAPE_HEADER

						/* INIT PTR TO ANIM_LIST */

	offset = *((long *)(tempPtr+SHAPE_HEADER_ANIM_LIST));	// get offset to ANIM_LIST
	newSpritePtr->AnimsList = tempPtr+offset+2;				// set ptr to ANIM_LIST
															// but skip 1st word: #anims!!!!

	AnimateASprite(newSpritePtr);							// initialize anim by calling it

	gMostRecentShape = newSpritePtr;

	return (newSpritePtr);									// return ptr to new sprite node
}


/************************ LOAD SHAPE TABLE *****************/

void LoadShapeTable(Str255 fileName, long groupNum, Boolean usePalFlag)
{
					/* THE REAL WORK */

	if (gShapeTableHandle[groupNum] != nil)						// see if zap existing shapetable
		DisposeHandle(gShapeTableHandle[groupNum]);

	gShapeTableHandle[groupNum] = LoadPackedFile(fileName);

	if (usePalFlag)
		BuildShapePalette(groupNum);							// get shape table's pal going


	CreateShapeHeaderPointers(groupNum);
}


/***************** CREATE SHAPE HEADER POINTERS ********************/
//
// This is called whenever a shape table is moved in memory or loaded
//

static void CreateShapeHeaderPointers(long groupNum)
{
long		i;
short		*intPtr;
long		*longPtr,offset;
Ptr			shapeTablePtr;

	shapeTablePtr = *gShapeTableHandle[groupNum];				// get ptr to shape table

	longPtr = (long *)((shapeTablePtr)+SF_HEADER__SHAPE_LIST);	// get ptr to offset to SHAPE_LIST
	intPtr = (short *)((shapeTablePtr)+(*longPtr));				// get ptr to SHAPE_LIST

	gNumShapesInFile[groupNum] = *intPtr++;						// get # shapes in the file
	longPtr = (long *)intPtr;

	for (i=0; i<gNumShapesInFile[groupNum]; i++)
	{
		offset = *(longPtr++);									// get offset to SHAPE_HEADER_n
		gSHAPE_HEADER_Ptrs[groupNum][i] = StripAddress(shapeTablePtr)+offset;	// save ptr to SHAPE_HEADER
	}
}

/******************* OPTIMIZE SHAPE TABLES **********************/
//
// This unlocks all available shape tables and compactmems them
// DO NOT CALL THIS WHILE SHAPES/OBJECTS ARE ACTIVE!
//

void OptimizeShapeTables(void)
{
long	i;
long	growBytes;

				/* UNLOCK IT ALL */

	for (i = 0; i < MAX_SHAPE_GROUPS; i++)
	{
		if (gShapeTableHandle[i] != nil)
		{
			HUnlock(gShapeTableHandle[i]);			// unlock it
		}
	}

				/* COMPACT */

	MaxMem(&growBytes);								// compact all
	CompactMem(maxSize);

			/* RELOCK IT ALL */

	for (i = 0; i < MAX_SHAPE_GROUPS; i++)
	{
		if (gShapeTableHandle[i] != nil)
		{
			HLockHi(gShapeTableHandle[i]);			// relock it
			CreateShapeHeaderPointers(i);			// reset it
		}
	}
}


/************************ DRAW ON BACKGROUND ********************/
//
// NOTE: draws to both bg buffer and offscreen buffer simultaneously!!!
//

void DrawOnBackground(long x,long y,long groupNum,long shapeNum,long frameNum)
{
register	long	col,height;
register	long	*destPtr,*srcPtr,*maskPtr,*dest2Ptr;
long		*dest2StartPtr,*destStartPtr;
Ptr			tempPtr,shapePtr;
long		*longPtr,offset;
short		*intPtr;
long		width;
static		Rect	box;

	if (shapeNum >= gNumShapesInFile[groupNum])						// see if error
		DoFatalAlert("\pIllegal Shape #");

					/* CALC ADDRESS OF FRAME TO DRAW */

	shapePtr = 	gSHAPE_HEADER_Ptrs[groupNum][shapeNum];		// get ptr to SHAPE_HEADER

	offset = *((long *)(shapePtr+2));				// get offset to FRAME_LIST
	tempPtr = shapePtr+offset;						// get ptr to FRAME_LIST

	if (frameNum >= *((short *)tempPtr))				// see if error
		DoFatalAlert("\pIllegal Frame #");


	longPtr = (long *)(tempPtr+(frameNum<<2)+2);	// point to correct frame offset
	offset = *longPtr;								// get offset to FRAME_HEADER
	tempPtr = shapePtr+offset;						// get ptr to FRAME_HEADER


	intPtr = (short *)tempPtr;						// get height & width of frame
	width = (*intPtr++)>>2;							// word width
	height = *intPtr++;
	x += *intPtr++;									// use position offsets
	y += *intPtr++;

	if ((x < 0) ||									// see if out of bounds
		(x >= OFFSCREEN_WIDTH) ||
		(y < 0) ||
		(y >= OFFSCREEN_HEIGHT))
			return;

	box.left = x;									// create update region
	box.right = x+(width<<2)-1;
	box.top = y;
	box.bottom = y+height;

	longPtr = (long *)intPtr;
	srcPtr = (long *)((*longPtr++)+shapePtr);		// point to pixel data
	maskPtr = (long *)((*longPtr)+shapePtr);		// point to mask data

				/* CALC BG & OFFSCREEN BUFFER ADDRS TO DRAW */

	destStartPtr = (long *)(gBackgroundLookUpTable[y]+x);
	dest2StartPtr = (long *)(gOffScreenLookUpTable[y]+x);

						/* DO THE DRAW */

	if (!height)									// special check for 0 heights
		height = 1;
	do
	{
		destPtr = destStartPtr;						// get line start ptr
		dest2Ptr = dest2StartPtr;

		col = width;
		do
		{
			*destPtr++ = *dest2Ptr++ = (*destPtr & (*maskPtr++)) | (*srcPtr++);
		}while(--col);

		destStartPtr += (OFFSCREEN_WIDTH>>2);			// next row
		dest2StartPtr += (OFFSCREEN_WIDTH>>2);

	} while(--height);

	AddUpdateRegion(box,CLIP_REGION_SCREEN);
}

/************************ DRAW ON BACKGROUND_NOMASK ********************/
//
// NOTE: draws to both bg buffer and offscreen buffer simultaneously!!!
//		Doesnt use sprite masks!
//
// updateFlag = true if want to create update region for shape
//

void DrawOnBackground_NoMask(long x,long y,long groupNum,long shapeNum,long frameNum, Boolean updateFlag)
{
register	long	col;
register	long	height;
register	long	*destPtr,*srcPtr,*dest2Ptr;
register	long	*dest2StartPtr,*destStartPtr;
long		width;
Ptr			tempPtr,shapePtr;
long		*longPtr,offset;
short		*intPtr;
Rect		box;

					/* CALC ADDRESS OF FRAME TO DRAW */

	shapePtr = 	gSHAPE_HEADER_Ptrs[groupNum][shapeNum];		// get ptr to SHAPE_HEADER

	offset = *((long *)(shapePtr+2));				// get offset to FRAME_LIST
	tempPtr = shapePtr+offset;						// get ptr to FRAME_LIST

	longPtr = (long *)(tempPtr+(frameNum<<2)+2);	// point to correct frame offset
	offset = *longPtr;								// get offset to FRAME_HEADER
	tempPtr = shapePtr+offset;						// get ptr to FRAME_HEADER


	intPtr = (short *)tempPtr;						// get height & width of frame
	width = (*intPtr++)>>2;							// word width
	height = *intPtr++;
	x += *intPtr++;									// use position offsets
	y += *intPtr++;

	if ((x < 0) ||									// see if out of bounds
		(x >= OFFSCREEN_WIDTH) ||
		(y < 0) ||
		(y >= OFFSCREEN_HEIGHT))
			return;

	box.left = x;									// create update region
	box.right = x+(width<<2)-1;
	box.top = y;
	box.bottom = y+height;

	longPtr = (long *)intPtr;
	srcPtr = (long *)((*longPtr++)+shapePtr);		// point to pixel data

				/* CALC BG & OFFSCREEN BUFFER ADDRS TO DRAW */

	destStartPtr = (long *)(gBackgroundLookUpTable[y]+x);
	dest2StartPtr = (long *)(gOffScreenLookUpTable[y]+x);

						/* DO THE DRAW */
	if (!height)									// special check for 0 heights
		height = 1;

	do
	{
		destPtr = destStartPtr;						// get line start ptr
		dest2Ptr = dest2StartPtr;

		col = width;
		do
		{
			*destPtr++ = *dest2Ptr++ = *srcPtr++;
		} while (--col);

		destStartPtr += (OFFSCREEN_WIDTH>>2);			// next row
		dest2StartPtr += (OFFSCREEN_WIDTH>>2);

	} while(--height);

	if (updateFlag)										// see if create update region
		AddUpdateRegion(box,CLIP_REGION_SCREEN);

}


/************************ DRAW FRAME TO SCREEN ********************/
//
// NOTE: draws directly to the screen
//
// INPUT: x/y = LOCAL screen coords, not global!!!
//

void DrawFrameToScreen(long x,long y,long groupNum,long shapeNum,long frameNum)
{
register	long	col,height;
register	long	*destPtr,*srcPtr,*maskPtr;
register	long	*destStartPtr;
long		width;
Ptr			tempPtr,shapePtr;
long		*longPtr,offset;
short		*intPtr;

					/* CALC ADDRESS OF FRAME TO DRAW */

	shapePtr = 	gSHAPE_HEADER_Ptrs[groupNum][shapeNum];		// get ptr to SHAPE_HEADER

	offset = *((long *)(shapePtr+2));				// get offset to FRAME_LIST
	tempPtr = shapePtr+offset;						// get ptr to FRAME_LIST

	longPtr = (long *)(tempPtr+(frameNum<<2)+2);	// point to correct frame offset
	offset = *longPtr;								// get offset to FRAME_HEADER
	tempPtr = shapePtr+offset;						// get ptr to FRAME_HEADER


	intPtr = (short *)tempPtr;						// get height & width of frame
	width = (*intPtr++)>>2;							// word width
	height = *intPtr++;
	x += *intPtr++;									// use position offsets
	y += *intPtr++;

	if ((x < 0) ||									// see if out of bounds
		(x >= VISIBLE_WIDTH) ||
		(y < 0) ||
		(y >= VISIBLE_HEIGHT))
			return;

	longPtr = (long *)intPtr;
	srcPtr = (long *)((*longPtr++)+shapePtr);		// point to pixel data
	maskPtr = (long *)((*longPtr)+shapePtr);		// point to mask data
	destStartPtr = (long *)(gScreenLookUpTable[y]+x);		// point to screen

//	gMMUMode = true32b;									// we must do this in 32bit addressing mode
//	SwapMMUMode(&gMMUMode);


						/* DO THE DRAW */

	do
	{
		destPtr = destStartPtr;							// get line start ptr

		col = width;
		do
		{
			*destPtr++ = (*destPtr & (*maskPtr++)) | (*srcPtr++);
		}while (--col > 0);

		destStartPtr += gScreenRowOffsetLW;			// next row
	}
	while (--height > 0);

//	SwapMMUMode(&gMMUMode);						// Restore addressing mode
}



/************************ DRAW FRAME TO SCREEN: NO MASK  ********************/
//
// NOTE: draws directly to the screen
//
// INPUT: x/y = LOCAL screen coords, not global!!!
//

void DrawFrameToScreen_NoMask(long x,long y,long groupNum,long shapeNum,long frameNum)
{
register	long	col,height;
register	long	*destPtr,*srcPtr;
register	long	*destStartPtr;
long	width;
Ptr		tempPtr,shapePtr;
long	*longPtr,offset;
short		*intPtr;

					/* CALC ADDRESS OF FRAME TO DRAW */

	shapePtr = 	gSHAPE_HEADER_Ptrs[groupNum][shapeNum];		// get ptr to SHAPE_HEADER

	offset = *((long *)(shapePtr+2));				// get offset to FRAME_LIST
	tempPtr = shapePtr+offset;						// get ptr to FRAME_LIST

	longPtr = (long *)(tempPtr+(frameNum<<2)+2);	// point to correct frame offset
	offset = *longPtr;								// get offset to FRAME_HEADER
	tempPtr = shapePtr+offset;						// get ptr to FRAME_HEADER


	intPtr = (short *)tempPtr;						// get height & width of frame
	width = (*intPtr++)>>2;							// word width
	height = *intPtr++;
	x += *intPtr++;									// use position offsets
	y += *intPtr++;

	if ((x < 0) ||									// see if out of bounds
		(x >= VISIBLE_WIDTH) ||
		(y < 0) ||
		(y >= VISIBLE_HEIGHT))
			return;

	longPtr = (long *)intPtr;
	srcPtr = (long *)((*longPtr++)+shapePtr);		// point to pixel data
	destStartPtr = (long *)(gScreenLookUpTable[y]+x);		// point to screen

//	gMMUMode = true32b;									// we must do this in 32bit addressing mode
//	SwapMMUMode(&gMMUMode);


						/* DO THE DRAW */

	do
	{
		destPtr = destStartPtr;							// get line start ptr

		col = width;
		do
		{
			*destPtr++ = *srcPtr++;
		}while (--col > 0);

		destStartPtr += gScreenRowOffsetLW;			// next row
	}
	while (--height > 0);

//	SwapMMUMode(&gMMUMode);						// Restore addressing mode
}


/************************* ZAP SHAPE TABLE ***************************/
//
// groupNum = $ff means zap all
//

void ZapShapeTable(long groupNum)
{
long	i,i2;

	if (groupNum == 0xff)
	{
		i = 0;
		i2 = MAX_SHAPE_GROUPS-1;
	}
	else
	{
		i = groupNum;
		i2 = groupNum;
	}

	for ( ; i<=i2; i++)
	{
		if (gShapeTableHandle[i] != nil)
		{
			DisposeHandle(gShapeTableHandle[i]);
			gShapeTableHandle[i] = nil;
		}
	}
}




/************************** CHECK FOOT PRIORITY ****************************/

Boolean CheckFootPriority(unsigned long x, unsigned long y, long width)
{
register	long	col,row;
register	long	x2;


	if ((y >= gPlayfieldHeight) || (x >= gPlayfieldWidth))		// check for bounds error  (automatically checks for <0)
		return(false);

	row = y >> TILE_SIZE_SH;
	col = x >> TILE_SIZE_SH;
	x2 = (x+width) >> TILE_SIZE_SH;

	for (; col <= x2; col ++)
	{
		if (gPlayfield[row][col] & TILE_PRIORITY_MASK)
			return(true);
	}
	return(false);
}


/************************ DRAW SPRITE AT ********************/
//
// Draws a sprite with given low level parameters
//

void DrawSpriteAt(ObjNode *theNodePtr,long x, long y, Ptr destBufferBaseAddr, long rowBytes)
{
register	long	width;
register	long	col;
register	long	*destPtr,*srcPtr,*maskPtr;
register	long	height;
long		*destStartPtr,*longPtr;
short		*intPtr;
long		frameNum;
long	offset;
long	shapeNum,groupNum;
Ptr		SHAPE_HEADER_Ptr,SHAPE_HEADER_Base;

	groupNum = theNodePtr->SpriteGroupNum;			// get shape group #
	shapeNum = theNodePtr->Type;					// get shape type
	frameNum = theNodePtr->CurrentFrame;			// get frame #

					/* CALC ADDRESS OF FRAME TO DRAW */

	if (shapeNum >= gNumShapesInFile[groupNum])		// see if error
		DoFatalAlert("\pIllegal Shape #");

	SHAPE_HEADER_Ptr = 	SHAPE_HEADER_Base =	theNodePtr->SHAPE_HEADER_Ptr;	// get ptr to SHAPE_HEADER

	offset = 	*((long *)(SHAPE_HEADER_Ptr+2));	// get offset to FRAME_LIST
	intPtr = (short *)(SHAPE_HEADER_Base + offset);	// get ptr to FRAME_LIST
	if (frameNum >= *intPtr++)						// see if error
		DoFatalAlert("\pIllegal Frame #");

	longPtr = (long *)intPtr;
	offset = *(longPtr+frameNum);					// get offset to FRAME_HEADER_n

	intPtr = (short *)(SHAPE_HEADER_Base + offset);	// get ptr to FRAME_HEADER

	width = (*intPtr++)>>2;								// get word width
	height = *intPtr++;									// get height
	x += *intPtr++;										// use position offsets
	y += *intPtr++;

	longPtr = (long *)intPtr;
	offset = *longPtr++;								// get offset to PIXEL_DATA
	srcPtr = (long *)(SHAPE_HEADER_Base + offset);		// get ptr to PIXEL_DATA
	offset = *longPtr++;								// get offset to MASK_DATA
	maskPtr = (long *)(SHAPE_HEADER_Base + offset);		// get ptr to MASK_DATA

	destStartPtr = (long *)((long)destBufferBaseAddr+(rowBytes*y)+x);	// calc draw addr

						/* DO THE DRAW */

//	gMMUMode = true32b;										// we must do this in 32bit addressing mode
//	SwapMMUMode(&gMMUMode);

	do
	{
		destPtr = destStartPtr;								// get line start ptr

		for (col = 0; col < width; col++)
			*destPtr++ = (*destPtr & (*maskPtr++)) | (*srcPtr++);

		destStartPtr += (rowBytes>>2);						// next row
	} while(--height);

//	SwapMMUMode(&gMMUMode);									// Restore addressing mode

}




/************************ DRAW FRAME AT ********************/
//
// Draws a frame with given low level parameters
//

void DrawFrameAt_NoMask(long x,long y,long groupNum,long shapeNum,long frameNum, Ptr destBufferBaseAddr, long rowBytes)
{
register	long		col,height;
register	long	*destPtr,*srcPtr;
register	long	*destStartPtr;
long		width;
Ptr		tempPtr,shapePtr;
long	*longPtr,offset;
short		*intPtr;

					/* CALC ADDRESS OF FRAME TO DRAW */

	shapePtr = 	gSHAPE_HEADER_Ptrs[groupNum][shapeNum];		// get ptr to SHAPE_HEADER

	offset = *((long *)(shapePtr+2));				// get offset to FRAME_LIST
	tempPtr = shapePtr+offset;						// get ptr to FRAME_LIST

	longPtr = (long *)(tempPtr+(frameNum<<2)+2);	// point to correct frame offset
	offset = *longPtr;								// get offset to FRAME_HEADER
	tempPtr = shapePtr+offset;						// get ptr to FRAME_HEADER


	intPtr = (short *)tempPtr;						// get height & width of frame
	width = (*intPtr++)>>2;							// word width
	height = *intPtr++;
	x += *intPtr++;									// use position offsets
	y += *intPtr++;

	longPtr = (long *)intPtr;
	srcPtr = (long *)((*longPtr++)+shapePtr);		// point to pixel data
	destStartPtr = (long *)((long)destBufferBaseAddr+(y*rowBytes)+x);

//	gMMUMode = true32b;									// we must do this in 32bit addressing mode
//	SwapMMUMode(&gMMUMode);

						/* DO THE DRAW */

	do
	{
		destPtr = destStartPtr;							// get line start ptr

		col = width;
		do
		{
			*destPtr++ = *srcPtr++;
		}while (--col > 0);

		destStartPtr += (rowBytes>>2);			// next row
	}
	while (--height > 0);

//	SwapMMUMode(&gMMUMode);						// Restore addressing mode
}


#ifdef __MWERKS__
//=======================================================================================
//                 POWERPC CODE
//=======================================================================================


/************************ DRAW A SPRITE ********************/
//
// Normal draw routine to draw a sprite Object
//

void DrawASprite(ObjNode *theNodePtr)
{
long	width,i;
long	*destPtr,*srcPtr,*maskPtr;
long	height;
long	*destStartPtr,*longPtr;
short	*intPtr;
long	frameNum;
long	x,y,offset;
Rect	oldBox;
long	shapeNum,groupNum;
Ptr		SHAPE_HEADER_Ptr,SHAPE_HEADER_Base;

	if (theNodePtr->PFCoordsFlag)					// see if do special PF Draw code
	{
		DrawPFSprite(theNodePtr);
		return;
	}

	groupNum = theNodePtr->SpriteGroupNum;			// get shape group #
	shapeNum = theNodePtr->Type;					// get shape type
	frameNum = theNodePtr->CurrentFrame;			// get frame #
	x = (theNodePtr->X.Int);						// get short x coord
	y = (theNodePtr->Y.Int);						// get short y coord

					/* CALC ADDRESS OF FRAME TO DRAW */

	if (shapeNum >= gNumShapesInFile[groupNum])		// see if error
		DoFatalAlert("\pIllegal Shape #");

	SHAPE_HEADER_Ptr = 	SHAPE_HEADER_Base =	theNodePtr->SHAPE_HEADER_Ptr;	// get ptr to SHAPE_HEADER

	offset = 	*((long *)(SHAPE_HEADER_Ptr+2));	// get offset to FRAME_LIST
	intPtr = (short *)(SHAPE_HEADER_Base + offset);	// get ptr to FRAME_LIST
	if (frameNum >= *intPtr++)						// see if error
		DoFatalAlert("\pIllegal Frame #");

	longPtr = (long *)intPtr;
	offset = *(longPtr+frameNum);					// get offset to FRAME_HEADER_n

	intPtr = (short *)(SHAPE_HEADER_Base + offset);	// get ptr to FRAME_HEADER

	width = (*intPtr++)>>2;								// get word width
	height = *intPtr++;									// get height
	x += *intPtr++;										// use position offsets
	y += *intPtr++;

	oldBox = theNodePtr->drawBox;						// remember old box

	if ((x < gRegionClipLeft[theNodePtr->ClipNum]) ||		// see if out of bounds
		((x+(width<<2)) >= gRegionClipRight[theNodePtr->ClipNum]) ||
		((y+height) <= gRegionClipTop[theNodePtr->ClipNum]) ||
		(y >= gRegionClipBottom[theNodePtr->ClipNum]))
			goto update;

	if	((y+height) > gRegionClipBottom[theNodePtr->ClipNum])	// see if need to clip height
		height -= (y+height)-gRegionClipBottom[theNodePtr->ClipNum];

	longPtr = (long *)intPtr;
	offset = *longPtr++;								// get offset to PIXEL_DATA
	srcPtr = (long *)(SHAPE_HEADER_Base + offset);		// get ptr to PIXEL_DATA
	offset = *longPtr++;								// get offset to MASK_DATA
	maskPtr = (long *)(SHAPE_HEADER_Base + offset);		// get ptr to MASK_DATA

	if (y < gRegionClipTop[theNodePtr->ClipNum])			// see if need to clip TOP
	{
		offset = gRegionClipTop[theNodePtr->ClipNum]-y;
		y = gRegionClipTop[theNodePtr->ClipNum];
		height -= offset;
		srcPtr += offset*width;
		maskPtr += offset*width;
	}

	if (theNodePtr->UpdateBoxFlag)						// see if using update regions
	{
		theNodePtr->drawBox.left = x;					// set drawn box
		theNodePtr->drawBox.right = x+(width<<2)-1;		// pixel widths
		theNodePtr->drawBox.top = y;
		theNodePtr->drawBox.bottom = y+height;
	}

	destStartPtr = (long *)(gOffScreenLookUpTable[y]+x);	// calc draw addr

						/* DO THE DRAW */
	if (height <= 0)										// special check for illegal heights
		height = 1;

	do
	{
		destPtr = destStartPtr;								// get line start ptr

		for (i=width; i; i--)
			*destPtr++ = (*destPtr & (*maskPtr++)) | (*srcPtr++);

		destStartPtr += (OFFSCREEN_WIDTH>>2);				// next row
	} while(--height);


					/* MAKE AN UPDATE REGION */
update:
	if (theNodePtr->UpdateBoxFlag)							// see if using update regions
	{
		if (oldBox.left > theNodePtr->drawBox.left)			// setup max box
			oldBox.left = theNodePtr->drawBox.left;

		if (oldBox.right < theNodePtr->drawBox.right)
			oldBox.right = theNodePtr->drawBox.right;

		if (oldBox.top > theNodePtr->drawBox.top)
			oldBox.top = theNodePtr->drawBox.top;

		if (oldBox.bottom < theNodePtr->drawBox.bottom)
			oldBox.bottom = theNodePtr->drawBox.bottom;

		AddUpdateRegion(oldBox,theNodePtr->ClipNum);
	}
}


/************************ ERASE A SPRITE ********************/

void EraseASprite(ObjNode *theNodePtr)
{
long	*destPtr,*destStartPtr,*srcPtr,*srcStartPtr;
long	i;
long	width,height;
long	x,y;

	if (theNodePtr->PFCoordsFlag)					// see if do special PF Erase code
	{
		ErasePFSprite(theNodePtr);
		return;
	}

	x = theNodePtr->drawBox.left;					// get x coord
	y = theNodePtr->drawBox.top;					// get y coord

	if ((x < 0) ||									// see if out of bounds
		(x >= OFFSCREEN_WIDTH) ||
		(y < 0) ||
		(y >= OFFSCREEN_HEIGHT))
			return;

	width = (((theNodePtr->drawBox.right)-x)>>2)+2;
	height = (theNodePtr->drawBox.bottom)-y;

	destStartPtr = (long *)(gOffScreenLookUpTable[y]+x);	// calc read/write addrs
	srcStartPtr = (long *)(gBackgroundLookUpTable[y]+x);

						/* DO THE ERASE */

	for (; height > 0; height--)
	{
		destPtr = destStartPtr;						// get line start ptrs
		srcPtr = srcStartPtr;

		for (i=width; i; i--)
			*destPtr++ = *srcPtr++;

		destStartPtr += (OFFSCREEN_WIDTH>>2);		// next row
		srcStartPtr += (OFFSCREEN_WIDTH>>2);
	}
}

/************************ DRAW PLAYFIELD SPRITE ********************/
//
// Draws shape obj into Playfield circular buffer
//

static void DrawPFSprite(ObjNode *theNodePtr)
{
long	width,height,i;
long	drawHeight,y;
Ptr		destPtrB,srcPtrB,maskPtrB;
long	*destPtr,*srcPtr,*maskPtr;
Ptr		tileMaskStartPtr;
long	*tileMaskPtr;
long	*longPtr;
Ptr		destStartPtr,srcStartPtr,originalSrcStartPtr;
Ptr		originalMaskStartPtr,maskStartPtr;
short	*intPtr;
long	frameNum,x;
long	offset;
long	realWidth,originalY,topToClip,leftToClip;
long	drawWidth,footY,shapeNum,groupNum,numHSegs;
Ptr		SHAPE_HEADER_Ptr,SHAPE_HEADER_Base;
Boolean	priorityFlag;
Ptr		tmP;

	groupNum = theNodePtr->SpriteGroupNum;				// get shape group #
	shapeNum = theNodePtr->Type;						// get shape type
	frameNum = theNodePtr->CurrentFrame;				// get frame #
	x = theNodePtr->X.Int;								// get short x coord (world)
	footY = y = theNodePtr->Y.Int;						// get short y coord & add any y adjustment offset
	y += theNodePtr->YOffset.Int;						// add any y adjustment offset

					/* CALC ADDRESS OF FRAME TO DRAW */

	if (shapeNum >= gNumShapesInFile[groupNum])			// see if error
	{
		DoAlert("\pIllegal Shape #");
		ShowSystemErr(shapeNum);
	}

	SHAPE_HEADER_Ptr = 	SHAPE_HEADER_Base =	theNodePtr->SHAPE_HEADER_Ptr;	// get ptr to SHAPE_HEADER

	offset = *((long *)(SHAPE_HEADER_Ptr+2));			// get offset to FRAME_LIST
	intPtr = (short *)(SHAPE_HEADER_Base + offset);		// get ptr to FRAME_LIST
	if (frameNum >= *intPtr++)							// see if error
		DoFatalAlert("\pIllegal Frame #");

	longPtr = (long *)intPtr;
	offset = *(longPtr+frameNum);						// get offset to FRAME_HEADER_n

	intPtr = (short *)(SHAPE_HEADER_Base + offset);		// get ptr to FRAME_HEADER

	drawWidth = realWidth = width = *intPtr++;			// get pixel width
	height = *intPtr++;									// get height
	x += *intPtr++;										// use position offsets (still global coords)
	y += *intPtr++;


				/************************/
				/*  CHECK IF VISIBLE    */
				/************************/

	if (((x+width) < gScrollX) || ((y+height) < gScrollY) ||
		(x >= (gScrollX+PF_BUFFER_WIDTH)) ||
		(y >= (gScrollY+PF_BUFFER_HEIGHT)))
	{
		theNodePtr->drawBox.left = 0;
		theNodePtr->drawBox.right = 0;
		theNodePtr->drawBox.top = 0;
		theNodePtr->drawBox.bottom = 0;
		return;
	}

					/***********************/
					/* CHECK VIEW CLIPPING */
					/***********************/

	if ((y+height) > (gScrollY+PF_BUFFER_HEIGHT))		// check vertical view clipping (bottom)
		height -= (y+height)-(gScrollY+PF_BUFFER_HEIGHT);

	if (y < gScrollY)									// check more vertical view clipping (top)
	{
		topToClip = (gScrollY-y);
		y += topToClip;
		height -= topToClip;
	}
	else
		topToClip = 0;

	if ((x+width) > (gScrollX+PF_BUFFER_WIDTH))			// check horiz view clipping (right)
	{
		width -= (x+width)-(gScrollX+PF_BUFFER_WIDTH);
		drawWidth = width;
	}

	if (x < gScrollX)									// check more horiz view clip (left)
	{
		leftToClip = (gScrollX-x);
		x += leftToClip;
		width -= leftToClip;
		drawWidth = width;
	}
	else
		leftToClip = 0;


	if (theNodePtr->TileMaskFlag)
		priorityFlag = CheckFootPriority(x,footY,drawWidth);	// see if use priority masking
	else
		priorityFlag = false;

	theNodePtr->drawBox.top = y = originalY =  (y % PF_BUFFER_HEIGHT);	// get PF buffer pixel coords to start @
	theNodePtr->drawBox.left = x = (x % PF_BUFFER_WIDTH);
	theNodePtr->drawBox.right = width;							// right actually = width
	theNodePtr->drawBox.bottom = height;

	if ((x+width) > PF_BUFFER_WIDTH)							// check horiz buffer clipping
	{
		width -= ((x+width)-PF_BUFFER_WIDTH);
		numHSegs = 2;											// 2 horiz segments
	}
	else
		numHSegs = 1;

	leftToClip += (topToClip*realWidth);
	longPtr = (long *)intPtr;
	offset = (*longPtr++)+leftToClip;							// get offset to PIXEL_DATA
	originalSrcStartPtr = srcStartPtr = 						// get ptr to PIXEL_DATA
			(Ptr)(SHAPE_HEADER_Base + offset);
	offset = (*longPtr++)+leftToClip;							// get offset to MASK_DATA
	originalMaskStartPtr = maskStartPtr = 						// get ptr to MASK_DATA
			(Ptr)(SHAPE_HEADER_Base + offset);

	destStartPtr = (Ptr)(gPFLookUpTable[y]+x);					// calc draw addr

						/* DO THE DRAW */


	if (!priorityFlag)
	{
		for (; numHSegs > 0; numHSegs--)
		{
			for (drawHeight = 0; drawHeight < height; drawHeight++)
			{
				destPtr = (long *)destStartPtr;					// get line start ptr
				srcPtr = (long *)srcStartPtr;
				maskPtr = (long *)maskStartPtr;

				for (i=(width>>2); i ; i--)					// draw longs
					*destPtr++ = (*destPtr & (*maskPtr++)) | (*srcPtr++);

				destPtrB = (Ptr)destPtr;
				maskPtrB = (Ptr)maskPtr;
				srcPtrB = (Ptr)srcPtr;
				for (i=(width&b11); i; i--)						// draw remaining pixels
					*destPtrB++ = (*destPtrB & (*maskPtrB++)) | (*srcPtrB++);

				srcStartPtr += realWidth;						// next sprite line
				maskStartPtr += realWidth;						// next mask line

				if (++y >=  PF_BUFFER_HEIGHT)					// see if wrap buffer vertically
				{
					destStartPtr = (Ptr)(gPFLookUpTable[0]+x);	// wrap to top
					y = 0;
				}
				else
					destStartPtr += PF_BUFFER_WIDTH;			// next buffer line
			}

			if (numHSegs == 2)
			{
				destStartPtr = (Ptr)gPFLookUpTable[y = originalY];	// set buff addr for segment #2
				x = 0;
				srcStartPtr = originalSrcStartPtr+width;
				maskStartPtr = originalMaskStartPtr+width;
				width = drawWidth-width;
			}
		}
	}
	else
	{
					/**************************/
					/* DRAW IT WITH TILE MASK */
					/**************************/

		tileMaskStartPtr = (Ptr)(gPFMaskLookUpTable[y]+x);			// calc tilemask addr

		for (; numHSegs > 0; numHSegs--)
		{
			for (drawHeight = 0; drawHeight < height; drawHeight++)
			{
				destPtr = (long *)destStartPtr;								// get line start ptr
				srcPtr = (long *)srcStartPtr;
				maskPtr = (long *)maskStartPtr;
				tileMaskPtr = (long *)tileMaskStartPtr;

				for (i=(width>>2); i ; i--)
				{
					*destPtr++ = (*destPtr & (*maskPtr | *tileMaskPtr)) |		// draw longs
								 (*srcPtr & (*tileMaskPtr ^ 0xffffffffL));
					tileMaskPtr++;
					maskPtr++;
					srcPtr++;
				}

				tmP = (Ptr)tileMaskPtr;
				destPtrB = (Ptr)destPtr;
				maskPtrB = (Ptr)maskPtr;
				srcPtrB = (Ptr)srcPtr;
				for (i=(width&b11); i; i--)						// draw remaining pixels
				{
					*destPtrB++ = (*destPtrB & (*maskPtrB | *tmP)) |
								 (*srcPtrB & (*tmP ^ 0xff));
					maskPtrB++;
					srcPtrB++;
					tmP++;
				}

				srcStartPtr += realWidth;						// next sprite line
				maskStartPtr += realWidth;						// next mask line

				if (++y >=  PF_BUFFER_HEIGHT)					// see if wrap buffer vertically
				{
					destStartPtr = (Ptr)(gPFLookUpTable[0]+x);	// wrap to top
					tileMaskStartPtr = (Ptr)(gPFMaskLookUpTable[0]+x);
					y = 0;
				}
				else
				{
					destStartPtr += PF_BUFFER_WIDTH;			// next buffer line
					tileMaskStartPtr += PF_BUFFER_WIDTH;
				}
			}

			if (numHSegs == 2)
			{
				destStartPtr = (Ptr)gPFLookUpTable[originalY];		// set buff addr for segment #2
				tileMaskStartPtr = (Ptr)gPFMaskLookUpTable[y = originalY];
				x = 0;
				srcStartPtr = originalSrcStartPtr+width;
				maskStartPtr = originalMaskStartPtr+width;
				width = drawWidth-width;
			}
		}
	}
}

/************************ ERASE PLAYFIELD SPRITE ********************/

static void ErasePFSprite(ObjNode *theNodePtr)
{
long	width,height,drawWidth,y;
long	i;
double	*destPtr,*srcPtr;
Ptr		destPtrB,srcPtrB;
Ptr		destStartPtr;
Ptr		srcStartPtr,originalSrcStartPtr;
long	x;
long	numHSegs;
long	drawHeight,originalY;

	x = theNodePtr->drawBox.left;					// remember area in the drawbox
	drawWidth = width = theNodePtr->drawBox.right;	// right actually = width
	originalY = y = theNodePtr->drawBox.top;
	height = theNodePtr->drawBox.bottom;

	if ((height <= 0) || (width <= 0))							// see if anything there
		return;

	if ((x+width) >= PF_BUFFER_WIDTH)				// check horiz clipping
	{
		width -= ((x+width)-PF_BUFFER_WIDTH);
		numHSegs = 2;								// 2 horiz segments
	}
	else
		numHSegs = 1;


	destStartPtr = (Ptr)(gPFLookUpTable[y]+x);				// calc draw addr
	originalSrcStartPtr = srcStartPtr = (Ptr)(gPFCopyLookUpTable[y]+x);	// calc source addr

						/* DO THE ERASE */

	for (; numHSegs > 0; numHSegs--)
	{
		for (drawHeight = 0; drawHeight < height; drawHeight++)
		{
			destPtr = (double *)destStartPtr;					// get line start ptr
			srcPtr = (double *)srcStartPtr;

//			for (i=(width>>3); i ; i--)
//				*destPtr++ = *srcPtr++;							// erase doubles

			ILD(destPtr,srcPtr,width>>3)


			destPtrB = (Ptr)destPtr;
			srcPtrB = (Ptr)srcPtr;


			for (i = (width&b111-1); i >= 0; i--)				// do remaining pixels
				destPtrB[i] = srcPtrB[i];

			if (++y >=  PF_BUFFER_HEIGHT)					// see if wrap buffer vertically
			{
				destStartPtr = (Ptr)(gPFLookUpTable[0]+x);	// wrap to top
				srcStartPtr = (Ptr)(gPFCopyLookUpTable[0]+x);
				y = 0;
			}
			else
			{
				destStartPtr += PF_BUFFER_WIDTH;			// next buffer line
				srcStartPtr += PF_BUFFER_WIDTH;
			}
		}

		if (numHSegs == 2)
		{
			destStartPtr = (Ptr)gPFLookUpTable[originalY];		// set buff addr for segment #2
			srcStartPtr = (Ptr)gPFCopyLookUpTable[originalY];
			y = originalY;
			x = 0;
			width = drawWidth-width;
		}
	}
}

#else
//=======================================================================================
//                 68000 CODE
//=======================================================================================


/************************ DRAW A SPRITE ********************/
//
// Normal draw routine to draw a sprite Object
//

void DrawASprite(ObjNode *theNodePtr)
{
register	short	width;
register	long	col;
register	long	*destPtr,*srcPtr,*maskPtr;
register	short	height;
static		long	*destStartPtr,*longPtr;
static		short	*intPtr,frameNum;
static		long	x,y,offset,oldD0;
static		Rect	oldBox;
static		short	shapeNum,groupNum;
static		Ptr		tempPtr,SHAPE_HEADER_Ptr,SHAPE_HEADER_Base;

	if (theNodePtr->PFCoordsFlag)					// see if do special PF Draw code
	{
		DrawPFSprite(theNodePtr);
		return;
	}

	groupNum = theNodePtr->SpriteGroupNum;			// get shape group #
	shapeNum = theNodePtr->Type;					// get shape type
	frameNum = theNodePtr->CurrentFrame;			// get frame #
	x = (theNodePtr->X.Int);						// get short x coord
	y = (theNodePtr->Y.Int);						// get short y coord

					/* CALC ADDRESS OF FRAME TO DRAW */

	if (shapeNum >= gNumShapesInFile[groupNum])		// see if error
	{
		DoAlert("\pIllegal Shape #");
		ShowSystemErr(shapeNum);
	}

	SHAPE_HEADER_Ptr = 	SHAPE_HEADER_Base =	theNodePtr->SHAPE_HEADER_Ptr;	// get ptr to SHAPE_HEADER

	offset = 	*((long *)(SHAPE_HEADER_Ptr+2));	// get offset to FRAME_LIST
	intPtr = (short *)(SHAPE_HEADER_Base + offset);	// get ptr to FRAME_LIST
	if (frameNum >= *intPtr++)						// see if error
		DoFatalAlert("\pIllegal Frame #");

	longPtr = (long *)intPtr;
	offset = *(longPtr+frameNum);					// get offset to FRAME_HEADER_n

	intPtr = (short *)(SHAPE_HEADER_Base + offset);	// get ptr to FRAME_HEADER

	width = (*intPtr++)>>2;							// get word width
	height = *intPtr++;								// get height
	x += *intPtr++;									// use position offsets
	y += *intPtr++;

	oldBox = theNodePtr->drawBox;					// remember old box

	if ((x < gRegionClipLeft[theNodePtr->ClipNum]) ||		// see if out of bounds
		((x+(width<<2)) >= gRegionClipRight[theNodePtr->ClipNum]) ||
		((y+height) <= gRegionClipTop[theNodePtr->ClipNum]) ||
		(y >= gRegionClipBottom[theNodePtr->ClipNum]))
			goto update;

	if	((y+height) > gRegionClipBottom[theNodePtr->ClipNum])	// see if need to clip height
		height -= (y+height)-gRegionClipBottom[theNodePtr->ClipNum];

	longPtr = (long *)intPtr;
	offset = *longPtr++;									// get offset to PIXEL_DATA
	srcPtr = (long *)(SHAPE_HEADER_Base + offset);			// get ptr to PIXEL_DATA
	offset = *longPtr++;									// get offset to MASK_DATA
	maskPtr = (long *)(SHAPE_HEADER_Base + offset);			// get ptr to MASK_DATA

	if (y < gRegionClipTop[theNodePtr->ClipNum])			// see if need to clip TOP
	{
		offset = gRegionClipTop[theNodePtr->ClipNum]-y;
		y = gRegionClipTop[theNodePtr->ClipNum];
		height -= offset;
		srcPtr += offset*width;
		maskPtr += offset*width;
	}

	if (theNodePtr->UpdateBoxFlag)						// see if using update regions
	{
		theNodePtr->drawBox.left = x;					// set drawn box
		theNodePtr->drawBox.right = x+(width<<2)-1;		// pixel widths
		theNodePtr->drawBox.top = y;
		theNodePtr->drawBox.bottom = y+height;
	}


						/* DO THE DRAW */
	if (height <= 0)										// special check for illegal heights
		height = 1;

	destStartPtr = (long *)(gOffScreenLookUpTable[y]+x);	// calc draw addr

	do
	{
		destPtr = destStartPtr;								// get line start ptr

		col = (112-width)<<3;								// 8 bytes/macro
		asm
		{
			move.l	d0,oldD0								// keep old d0 since I'm illegally using it!

			jmp		@inline(col)

			@inline
				MASK_MAC									//0..15 longs
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC

				MASK_MAC								//32..47
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC

				MASK_MAC								//48..63
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC

				MASK_MAC								//64..79
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC

				MASK_MAC								//80..95
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC

				MASK_MAC								//96..111
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC

				MASK_MAC								//112..127
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC
				MASK_MAC

				move.l	oldD0,D0							// restore d0 since I was illegally using it!
		}

		destStartPtr += (OFFSCREEN_WIDTH>>2);				// next row
	} while(--height);


					/* MAKE AN UPDATE REGION */
update:
	if (theNodePtr->UpdateBoxFlag)							// see if using update regions
	{
		if (oldBox.left > theNodePtr->drawBox.left)			// setup max box
			oldBox.left = theNodePtr->drawBox.left;

		if (oldBox.right < theNodePtr->drawBox.right)
			oldBox.right = theNodePtr->drawBox.right;

		if (oldBox.top > theNodePtr->drawBox.top)
			oldBox.top = theNodePtr->drawBox.top;

		if (oldBox.bottom < theNodePtr->drawBox.bottom)
			oldBox.bottom = theNodePtr->drawBox.bottom;

		AddUpdateRegion(oldBox,theNodePtr->ClipNum);
	}
}


/************************ ERASE A SPRITE ********************/

void EraseASprite(ObjNode *theNodePtr)
{
register	long	*destPtr,*destStartPtr,*srcPtr,*srcStartPtr;
register	short	col;
register	short	width,height;
static		long	x,y;
static		Rect	box;

	if (theNodePtr->PFCoordsFlag)					// see if do special PF Erase code
	{
		ErasePFSprite(theNodePtr);
		return;
	}

	x = theNodePtr->drawBox.left;					// get x coord
	y = theNodePtr->drawBox.top;					// get y coord

	if ((x < 0) ||									// see if out of bounds
		(x >= OFFSCREEN_WIDTH) ||
		(y < 0) ||
		(y >= OFFSCREEN_HEIGHT))
			return;

	width = (((theNodePtr->drawBox.right)-x)>>2)+2;
	height = (theNodePtr->drawBox.bottom)-y;

	destStartPtr = (long *)(gOffScreenLookUpTable[y]+x);	// calc read/write addrs
	srcStartPtr = (long *)(gBackgroundLookUpTable[y]+x);

						/* DO THE ERASE */

	for (; height > 0; height--)
	{
		destPtr = destStartPtr;						// get line start ptrs
		srcPtr = srcStartPtr;

		col = (112-width)<<1;

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
			}

		destStartPtr += (OFFSCREEN_WIDTH>>2);		// next row
		srcStartPtr += (OFFSCREEN_WIDTH>>2);
	}

}


/************************ DRAW PLAYFIELD SPRITE: INTERLACED ********************/
//
// Draws shape obj into Playfield circular buffer
//

static void DrawPFSpriteInterlaced(ObjNode *theNodePtr)
{
register	short		width,height;
register	short		drawHeight,y;
register	long	col;
register	Ptr		destPtr,srcPtr,maskPtr;
static		Ptr		tileMaskStartPtr;
static		long	*tileMaskPtr;
static		long	*longPtr;
static		Ptr		destStartPtr,srcStartPtr,originalSrcStartPtr;
static		Ptr		originalMaskStartPtr,maskStartPtr;
static		short		*intPtr,frameNum,x;
static		long	offset,oldD0;
static		Byte	pixel;
static		short		realWidth,originalY,topToClip,leftToClip;
static		short		drawWidth,footY,shapeNum,groupNum,numHSegs;
static		Ptr		SHAPE_HEADER_Ptr,SHAPE_HEADER_Base;
static		Boolean	priorityFlag;
static		Ptr		tmP;


	groupNum = theNodePtr->SpriteGroupNum;				// get shape group #
	shapeNum = theNodePtr->Type;						// get shape type
	frameNum = theNodePtr->CurrentFrame;				// get frame #
	x = theNodePtr->X.Int;								// get short x coord (world)
	footY = y = theNodePtr->Y.Int;								// get short y coord & add any y adjustment offset
	y += theNodePtr->YOffset.Int;						// add any y adjustment offset

					/* CALC ADDRESS OF FRAME TO DRAW */

	if (shapeNum >= gNumShapesInFile[groupNum])			// see if error
	{
		DoAlert("\pIllegal Shape #");
		ShowSystemErr(shapeNum);
	}

	SHAPE_HEADER_Ptr = 	SHAPE_HEADER_Base =	theNodePtr->SHAPE_HEADER_Ptr;	// get ptr to SHAPE_HEADER

	offset = *((long *)(SHAPE_HEADER_Ptr+2));			// get offset to FRAME_LIST
	intPtr = (short *)(SHAPE_HEADER_Base + offset);		// get ptr to FRAME_LIST
	if (frameNum >= *intPtr++)							// see if error
		DoFatalAlert("\pIllegal Frame #");

	longPtr = (long *)intPtr;
	offset = *(longPtr+frameNum);						// get offset to FRAME_HEADER_n

	intPtr = (short *)(SHAPE_HEADER_Base + offset);		// get ptr to FRAME_HEADER

	drawWidth = realWidth = width = *intPtr++;			// get pixel width
	height = *intPtr++;									// get height
	x += *intPtr++;										// use position offsets (still global coords)
	y += *intPtr++;


				/************************/
				/*  CHECK IF VISIBLE    */
				/************************/

	if (((x+width) < gScrollX) || ((y+height) < gScrollY) ||
		(x >= (gScrollX+PF_BUFFER_WIDTH)) ||
		(y >= (gScrollY+PF_BUFFER_HEIGHT)))
	{
		theNodePtr->drawBox.left = 0;
		theNodePtr->drawBox.right = 0;
		theNodePtr->drawBox.top = 0;
		theNodePtr->drawBox.bottom = 0;
		return;
	}

					/***********************/
					/* CHECK VIEW CLIPPING */
					/***********************/

	if ((y+height) > (gScrollY+PF_BUFFER_HEIGHT))		// check vertical view clipping (bottom)
		height -= (y+height)-(gScrollY+PF_BUFFER_HEIGHT);

	if (y < gScrollY)									// check more vertical view clipping (top)
	{
		topToClip = (gScrollY-y);
		y += topToClip;
		height -= topToClip;
	}
	else
		topToClip = 0;

	if ((x+width) > (gScrollX+PF_BUFFER_WIDTH))			// check horiz view clipping (right)
	{
		width -= (x+width)-(gScrollX+PF_BUFFER_WIDTH);
		drawWidth = width;
	}

	if (x < gScrollX)									// check more horiz view clip (left)
	{
		leftToClip = (gScrollX-x);
		x += leftToClip;
		width -= leftToClip;
		drawWidth = width;
	}
	else
		leftToClip = 0;


	if (theNodePtr->TileMaskFlag)
		priorityFlag = CheckFootPriority(x,footY,drawWidth);	// see if use priority masking
	else
		priorityFlag = false;

	theNodePtr->drawBox.top = y = originalY =  (y % PF_BUFFER_HEIGHT);	// get PF buffer pixel coords to start @
	theNodePtr->drawBox.left = x = (x % PF_BUFFER_WIDTH);
	theNodePtr->drawBox.right = width;							// right actually = width
	theNodePtr->drawBox.bottom = height;

	if ((x+width) > PF_BUFFER_WIDTH)							// check horiz buffer clipping
	{
		width -= ((x+width)-PF_BUFFER_WIDTH);
		numHSegs = 2;											// 2 horiz segments
	}
	else
		numHSegs = 1;


	longPtr = (long *)intPtr;
	offset = (*longPtr++)+(topToClip*realWidth)+leftToClip;						// get offset to PIXEL_DATA
	originalSrcStartPtr = srcStartPtr = (Ptr)(SHAPE_HEADER_Base + offset);		// get ptr to PIXEL_DATA
	offset = (*longPtr++)+(topToClip*realWidth)+leftToClip;						// get offset to MASK_DATA
	originalMaskStartPtr = maskStartPtr = (Ptr)(SHAPE_HEADER_Base + offset);	// get ptr to MASK_DATA

	destStartPtr = (Ptr)(gPFLookUpTable[y]+x);					// calc draw addr

						/* DO THE DRAW */

	if (!priorityFlag)
	{

		for (; numHSegs > 0; numHSegs--)
		{
			for (drawHeight = 0; drawHeight < height; drawHeight++)
			{
				if (!(y&1))										// see if skip this line
				{
					destPtr = destStartPtr;							// get line start ptr
					srcPtr = srcStartPtr;
					maskPtr = maskStartPtr;

					col = (112-(width>>2))<<3;							// 8 bytes/macro
					asm
					{
						move.l	d0,oldD0								// keep old d0 since I'm illegally using it!

						jmp		@inline(col)

						@inline
							MASK_MAC									//0..15 longs
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC

							MASK_MAC								//32..47
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC

							MASK_MAC								//48..63
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC

							MASK_MAC								//64..79
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC

							MASK_MAC								//80..95
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC

							MASK_MAC								//96..111
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC

							MASK_MAC								//112..127
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC
							MASK_MAC

							move.l	oldD0,D0							// restore d0 since I was illegally using it!
					}

					for (col=(width&b11); col; col--)				// draw remaining pixels
					{
						*destPtr++ = (*destPtr & (*maskPtr++)) | (*srcPtr++);
					}
				} // end if skip

				srcStartPtr += realWidth;						// next sprite line
				maskStartPtr += realWidth;						// next mask line

				if (++y >=  PF_BUFFER_HEIGHT)					// see if wrap buffer vertically
				{
					destStartPtr = (Ptr)(gPFLookUpTable[0]+x);	// wrap to top
					y = 0;
				}
				else
					destStartPtr += PF_BUFFER_WIDTH;			// next buffer line
			}

			if (numHSegs == 2)
			{
				destStartPtr = (Ptr)gPFLookUpTable[originalY];		// set buff addr for segment #2
				y = originalY;
				x = 0;
				srcStartPtr = originalSrcStartPtr+width;
				maskStartPtr = originalMaskStartPtr+width;
				width = drawWidth-width;
			}
		}
	}
	else
	{
					/**************************/
					/* DRAW IT WITH TILE MASK */
					/**************************/

		tileMaskStartPtr = (Ptr)(gPFMaskLookUpTable[y]+x);			// calc tilemask addr

		for (; numHSegs > 0; numHSegs--)
		{
			for (drawHeight = 0; drawHeight < height; drawHeight++)
			{
				if (!(y&1))										// see if skip this line
				{
					destPtr = destStartPtr;								// get line start ptr
					srcPtr = srcStartPtr;
					maskPtr = maskStartPtr;
					tileMaskPtr = (long *)tileMaskStartPtr;

					col = (112-(width>>2))<<4;							// 16 bytes/macro
					asm
					{
						movem	d0/d1/a0,-(sp)
						move.l	tileMaskPtr,a0							// put this in a register

						jmp		@inline2(col)

						@inline2
							MASK_MAC2									//0..15 longs
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2

							MASK_MAC2								//32..47
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2

							MASK_MAC2								//48..63
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2

							MASK_MAC2								//64..79
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2

							MASK_MAC2								//80..95
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2

							MASK_MAC2								//96..111
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2

							MASK_MAC2								//112..127
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2
							MASK_MAC2

							move.l	a0,tileMaskPtr
							movem	(sp)+,d0/d1/a0
					}

					tmP = (Ptr)tileMaskPtr;
					for (col=(width&b11); col; col--)				// draw remaining pixels
					{

						*destPtr++ = (*destPtr & (*maskPtr++ | *tmP)) |
									 (*srcPtr++ & (*tmP++ ^ 0xff));
					}
				} // end if skip

				srcStartPtr += realWidth;						// next sprite line
				maskStartPtr += realWidth;						// next mask line

				if (++y >=  PF_BUFFER_HEIGHT)					// see if wrap buffer vertically
				{
					destStartPtr = (Ptr)(gPFLookUpTable[0]+x);	// wrap to top
					tileMaskStartPtr = (Ptr)(gPFMaskLookUpTable[0]+x);
					y = 0;
				}
				else
				{
					destStartPtr += PF_BUFFER_WIDTH;			// next buffer line
					tileMaskStartPtr += PF_BUFFER_WIDTH;
				}
			}

			if (numHSegs == 2)
			{
				destStartPtr = (Ptr)gPFLookUpTable[originalY];		// set buff addr for segment #2
				tileMaskStartPtr = (Ptr)gPFMaskLookUpTable[originalY];
				y = originalY;
				x = 0;
				srcStartPtr = originalSrcStartPtr+width;
				maskStartPtr = originalMaskStartPtr+width;
				width = drawWidth-width;
			}
		}

	} // end else
}


/************************ ERASE PLAYFIELD SPRITE ********************/

static void ErasePFSprite(ObjNode *theNodePtr)
{
register	short	width,height,drawWidth,y;
register	long	col;
register	Ptr		destPtr,srcPtr,destStartPtr;
static		Ptr		srcStartPtr,originalSrcStartPtr;
static		long	x,colCopy;
static		short	numHSegs;
static		short	drawHeight,originalY;

	if (gInterlaceMode)								// see if erase it interlaced
	{
		ErasePFSpriteInterlaced(theNodePtr);
		return;
	}

	x = theNodePtr->drawBox.left;					// remember area in the drawbox
	drawWidth = width = theNodePtr->drawBox.right;	// right actually = width
	originalY = y = theNodePtr->drawBox.top;
	height = theNodePtr->drawBox.bottom;

	if ((height <= 0) || (width <= 0))				// see if anything there
		return;

	if ((x+width) >= PF_BUFFER_WIDTH)				// check horiz clipping
	{
		width -= ((x+width)-PF_BUFFER_WIDTH);
		numHSegs = 2;								// 2 horiz segments
	}
	else
		numHSegs = 1;


	destStartPtr = (Ptr)(gPFLookUpTable[y]+x);		// calc draw addr
	originalSrcStartPtr = srcStartPtr = (Ptr)(gPFCopyLookUpTable[y]+x);	// calc source addr

						/* DO THE ERASE */

	colCopy = (112-(width>>2))<<1;					// max size = 112*4 = 448

	for (; numHSegs > 0; numHSegs--)
	{
		for (drawHeight = 0; drawHeight < height; drawHeight++)
		{
			destPtr = destStartPtr;					// get line start ptr
			srcPtr = srcStartPtr;

			col = colCopy;							// assume didnt change

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
				}

			for (col = (width&b11); col > 0; col--)		// do remaining pixels
				asm
				{
					move.b	(srcPtr)+,(destPtr)+
				}


			if (++y >=  PF_BUFFER_HEIGHT)					// see if wrap buffer vertically
			{
				destStartPtr = (Ptr)(gPFLookUpTable[0]+x);	// wrap to top
				srcStartPtr = (Ptr)(gPFCopyLookUpTable[0]+x);
				y = 0;
			}
			else
			{
				destStartPtr += PF_BUFFER_WIDTH;		// next buffer line
				srcStartPtr += PF_BUFFER_WIDTH;
			}
		}

		if (numHSegs == 2)
		{
			destStartPtr = (Ptr)gPFLookUpTable[originalY];		// set buff addr for segment #2
			srcStartPtr = (Ptr)gPFCopyLookUpTable[originalY];
			y = originalY;
			x = 0;
			width = drawWidth-width;
			colCopy = (112-(width>>2))<<1;						// need to re-calc this
		}
	}
}


/************************ ERASE PLAYFIELD SPRITE: INTERLACED ********************/

static void ErasePFSpriteInterlaced(ObjNode *theNodePtr)
{
register	short		width,height,drawWidth,y;
register	long	col;
register	Ptr		destPtr,srcPtr,destStartPtr;
static		Ptr		srcStartPtr,originalSrcStartPtr;
static		long	x;
static		short	numHSegs;
static		short		drawHeight,originalY;

	x = theNodePtr->drawBox.left;					// remember area in the drawbox
	drawWidth = width = theNodePtr->drawBox.right;	// right actually = width
	originalY = y = theNodePtr->drawBox.top;
	height = theNodePtr->drawBox.bottom;

	if ((height <= 0) || (width<=0))				// see if anything there
		return;

	if ((x+width) >= PF_BUFFER_WIDTH)				// check horiz clipping
	{
		width -= ((x+width)-PF_BUFFER_WIDTH);
		numHSegs = 2;								// 2 horiz segments
	}
	else
		numHSegs = 1;


	destStartPtr = (Ptr)(gPFLookUpTable[y]+x);				// calc draw addr
	originalSrcStartPtr = srcStartPtr = (Ptr)(gPFCopyLookUpTable[y]+x);	// calc source addr

						/* DO THE ERASE */

	for (; numHSegs > 0; numHSegs--)
	{
		for (drawHeight = 0; drawHeight < height; drawHeight++)
		{
			if (!(y&1))										// see if skip this line
			{

				destPtr = destStartPtr;							// get line start ptr
				srcPtr = srcStartPtr;

				col = (112-(width>>2))<<1;						// max size = 112*4 = 448

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
					}

				for (col = (width&b11); col > 0; col--)		// do remaining pixels
				{
					asm
					{
						move.b	(srcPtr)+,(destPtr)+
					}
				}
			} // end if skip

			if (++y >=  PF_BUFFER_HEIGHT)					// see if wrap buffer vertically
			{
				destStartPtr = (Ptr)(gPFLookUpTable[0]+x);	// wrap to top
				srcStartPtr = (Ptr)(gPFCopyLookUpTable[0]+x);
				y = 0;
			}
			else
			{
				destStartPtr += PF_BUFFER_WIDTH;			// next buffer line
				srcStartPtr += PF_BUFFER_WIDTH;
			}
		}

		if (numHSegs == 2)
		{
			destStartPtr = (Ptr)gPFLookUpTable[originalY];		// set buff addr for segment #2
			srcStartPtr = (Ptr)gPFCopyLookUpTable[originalY];
			y = originalY;
			x = 0;
			width = drawWidth-width;
		}
	}
}

/************************ DRAW PLAYFIELD SPRITE ********************/
//
// Draws shape obj into Playfield circular buffer
//

static void DrawPFSprite(ObjNode *theNodePtr)
{
register	int		width,height;
register	int		drawHeight,y;
register	long	col;
register	Ptr		destPtr,srcPtr,maskPtr;
static		Ptr		tileMaskStartPtr;
static		long	*tileMaskPtr;
static		long	*longPtr;
static		Ptr		destStartPtr,srcStartPtr,originalSrcStartPtr;
static		Ptr		originalMaskStartPtr,maskStartPtr;
static		int		*intPtr,frameNum,x;
static		long	offset,oldD0;
static		Byte	pixel;
static		int		realWidth,originalY,topToClip,leftToClip;
static		int		drawWidth,footY,shapeNum,groupNum,numHSegs;
static		Ptr		SHAPE_HEADER_Ptr,SHAPE_HEADER_Base;
static		Boolean	priorityFlag;
static		Ptr		tmP;
static		long	colCopy;

	if (gInterlaceMode)									// see if draw it interlaced
	{
		DrawPFSpriteInterlaced(theNodePtr);
		return;
	}

	groupNum = theNodePtr->SpriteGroupNum;				// get shape group #
	shapeNum = theNodePtr->Type;						// get shape type
	frameNum = theNodePtr->CurrentFrame;				// get frame #
	x = theNodePtr->X.Int;								// get int x coord (world)
	footY = y = theNodePtr->Y.Int;								// get int y coord & add any y adjustment offset
	y += theNodePtr->YOffset.Int;						// add any y adjustment offset

					/* CALC ADDRESS OF FRAME TO DRAW */

	if (shapeNum >= gNumShapesInFile[groupNum])			// see if error
	{
		DoAlert("\pIllegal Shape #");
		ShowSystemErr(shapeNum);
	}

	SHAPE_HEADER_Ptr = 	SHAPE_HEADER_Base =	theNodePtr->SHAPE_HEADER_Ptr;	// get ptr to SHAPE_HEADER

	offset = *((long *)(SHAPE_HEADER_Ptr+2));			// get offset to FRAME_LIST
	intPtr = (int *)(SHAPE_HEADER_Base + offset);		// get ptr to FRAME_LIST
	if (frameNum >= *intPtr++)							// see if error
		DoFatalAlert("\pIllegal Frame #");

	longPtr = (long *)intPtr;
	offset = *(longPtr+frameNum);						// get offset to FRAME_HEADER_n

	intPtr = (int *)(SHAPE_HEADER_Base + offset);		// get ptr to FRAME_HEADER

	drawWidth = realWidth = width = *intPtr++;			// get pixel width
	height = *intPtr++;									// get height
	x += *intPtr++;										// use position offsets (still global coords)
	y += *intPtr++;


				/************************/
				/*  CHECK IF VISIBLE    */
				/************************/

	if (((x+width) < gScrollX) || ((y+height) < gScrollY) ||
		(x >= (gScrollX+PF_BUFFER_WIDTH)) ||
		(y >= (gScrollY+PF_BUFFER_HEIGHT)))
	{
		theNodePtr->drawBox.left = 0;
		theNodePtr->drawBox.right = 0;
		theNodePtr->drawBox.top = 0;
		theNodePtr->drawBox.bottom = 0;
		return;
	}

					/***********************/
					/* CHECK VIEW CLIPPING */
					/***********************/

	if ((y+height) > (gScrollY+PF_BUFFER_HEIGHT))		// check vertical view clipping (bottom)
		height -= (y+height)-(gScrollY+PF_BUFFER_HEIGHT);

	if (y < gScrollY)									// check more vertical view clipping (top)
	{
		topToClip = (gScrollY-y);
		y += topToClip;
		height -= topToClip;
	}
	else
		topToClip = 0;

	if ((x+width) > (gScrollX+PF_BUFFER_WIDTH))			// check horiz view clipping (right)
	{
		width -= (x+width)-(gScrollX+PF_BUFFER_WIDTH);
		drawWidth = width;
	}

	if (x < gScrollX)									// check more horiz view clip (left)
	{
		leftToClip = (gScrollX-x);
		x += leftToClip;
		width -= leftToClip;
		drawWidth = width;
	}
	else
		leftToClip = 0;


	if (theNodePtr->TileMaskFlag)
		priorityFlag = CheckFootPriority(x,footY,drawWidth);	// see if use priority masking
	else
		priorityFlag = false;

	theNodePtr->drawBox.top = y = originalY =  (y % PF_BUFFER_HEIGHT);	// get PF buffer pixel coords to start @
	theNodePtr->drawBox.left = x = (x % PF_BUFFER_WIDTH);
	theNodePtr->drawBox.right = width;							// right actually = width
	theNodePtr->drawBox.bottom = height;

	if ((x+width) > PF_BUFFER_WIDTH)							// check horiz buffer clipping
	{
		width -= ((x+width)-PF_BUFFER_WIDTH);
		numHSegs = 2;											// 2 horiz segments
	}
	else
		numHSegs = 1;

	leftToClip += (topToClip*realWidth);
	longPtr = (long *)intPtr;
	offset = (*longPtr++)+leftToClip;							// get offset to PIXEL_DATA
	originalSrcStartPtr = srcStartPtr = 						// get ptr to PIXEL_DATA
			(Ptr)(SHAPE_HEADER_Base + offset);
	offset = (*longPtr++)+leftToClip;							// get offset to MASK_DATA
	originalMaskStartPtr = maskStartPtr = 						// get ptr to MASK_DATA
			(Ptr)(SHAPE_HEADER_Base + offset);

	destStartPtr = (Ptr)(gPFLookUpTable[y]+x);					// calc draw addr

						/* DO THE DRAW */


	if (!priorityFlag)
	{
		colCopy = (112-(width>>2))<<3;							// 8 bytes/macro

		for (; numHSegs > 0; numHSegs--)
		{
			for (drawHeight = 0; drawHeight < height; drawHeight++)
			{
				destPtr = destStartPtr;							// get line start ptr
				srcPtr = srcStartPtr;
				maskPtr = maskStartPtr;

				col = colCopy;									// get value
				asm
				{
					move.l	d0,oldD0								// keep old d0 since I'm illegally using it!

					jmp		@inline(col)

					@inline
						MASK_MAC									//0..15 longs
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC

						MASK_MAC								//32..47
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC

						MASK_MAC								//48..63
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC

						MASK_MAC								//64..79
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC

						MASK_MAC								//80..95
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC

						MASK_MAC								//96..111
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC

						MASK_MAC								//112..127
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC
						MASK_MAC

						move.l	oldD0,D0							// restore d0 since I was illegally using it!
				}

				for (col=(width&b11); col; col--)				// draw remaining pixels
				{
					*destPtr++ = (*destPtr & (*maskPtr++)) | (*srcPtr++);
				}

				srcStartPtr += realWidth;						// next sprite line
				maskStartPtr += realWidth;						// next mask line

				if (++y >=  PF_BUFFER_HEIGHT)					// see if wrap buffer vertically
				{
					destStartPtr = (Ptr)(gPFLookUpTable[0]+x);	// wrap to top
					y = 0;
				}
				else
					destStartPtr += PF_BUFFER_WIDTH;			// next buffer line
			}

			if (numHSegs == 2)
			{
				destStartPtr = (Ptr)gPFLookUpTable[y = originalY];	// set buff addr for segment #2
				x = 0;
				srcStartPtr = originalSrcStartPtr+width;
				maskStartPtr = originalMaskStartPtr+width;
				width = drawWidth-width;
				colCopy = (112-(width>>2))<<3;					// need to re-calc this
			}
		}
	}
	else
	{
					/**************************/
					/* DRAW IT WITH TILE MASK */
					/**************************/

		colCopy = (112-(width>>2))<<4;							// 16 bytes/macro

		tileMaskStartPtr = (Ptr)(gPFMaskLookUpTable[y]+x);			// calc tilemask addr

		for (; numHSegs > 0; numHSegs--)
		{
			for (drawHeight = 0; drawHeight < height; drawHeight++)
			{
				destPtr = destStartPtr;								// get line start ptr
				srcPtr = srcStartPtr;
				maskPtr = maskStartPtr;
				tileMaskPtr = (long *)tileMaskStartPtr;

				col = colCopy;							// 16 bytes/macro
				asm
				{
					movem	d0/d1/a0,-(sp)
					move.l	tileMaskPtr,a0							// put this in a register

					jmp		@inline2(col)

					@inline2
						MASK_MAC2									//0..15 longs
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2

						MASK_MAC2								//32..47
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2

						MASK_MAC2								//48..63
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2

						MASK_MAC2								//64..79
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2

						MASK_MAC2								//80..95
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2

						MASK_MAC2								//96..111
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2

						MASK_MAC2								//112..127
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2
						MASK_MAC2

						move.l	a0,tileMaskPtr
						movem	(sp)+,d0/d1/a0
				}

				tmP = (Ptr)tileMaskPtr;
				for (col=(width&b11); col; col--)				// draw remaining pixels
				{

					*destPtr++ = (*destPtr & (*maskPtr++ | *tmP)) |
								 (*srcPtr++ & (*tmP++ ^ 0xff));
				}

				srcStartPtr += realWidth;						// next sprite line
				maskStartPtr += realWidth;						// next mask line

				if (++y >=  PF_BUFFER_HEIGHT)					// see if wrap buffer vertically
				{
					destStartPtr = (Ptr)(gPFLookUpTable[0]+x);	// wrap to top
					tileMaskStartPtr = (Ptr)(gPFMaskLookUpTable[0]+x);
					y = 0;
				}
				else
				{
					destStartPtr += PF_BUFFER_WIDTH;			// next buffer line
					tileMaskStartPtr += PF_BUFFER_WIDTH;
				}
			}

			if (numHSegs == 2)
			{
				destStartPtr = (Ptr)gPFLookUpTable[originalY];		// set buff addr for segment #2
				tileMaskStartPtr = (Ptr)gPFMaskLookUpTable[y = originalY];
				x = 0;
				srcStartPtr = originalSrcStartPtr+width;
				maskStartPtr = originalMaskStartPtr+width;
				width = drawWidth-width;
				colCopy = (112-(width>>2))<<4;						// need to recalc this
			}
		}

	} // end else
}


#endif



