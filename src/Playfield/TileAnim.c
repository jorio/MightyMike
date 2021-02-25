/****************************/
/*  TILE ANIMATION ROUTINES */
/* (c)1994 Pangea Software  */
/*   By Brian Greenstone    */
/****************************/


/***************/
/* EXTERNALS   */
/***************/
#include "myglobals.h"
#include "tileanim.h"
#include "playfield.h"

extern	long			gScrollRow,gScrollCol,gOldScrollRow,gOldScrollCol;
extern	unsigned short	**gPlayfield;
extern	Byte			gCurrentWeaponType,gSceneNum;
extern	short			gPlayfieldTileWidth,gPlayfieldTileHeight;
#if __USE_PF_VARS
extern long	PF_TILE_HEIGHT;
extern long	PF_TILE_WIDTH;
extern long	PF_WINDOW_TOP;
extern long	PF_WINDOW_LEFT;
#endif


/****************************/
/*    CONSTANTS             */
/****************************/


#define	MAX_TILE_ANIMS	50						// max # of tile anims

#define	SEQ_ENDMARK		0xffff



/**********************/
/*     VARIABLES      */
/**********************/

static	short				gNumTileAnims;
static	TileAnimEntryType	gTileAnims[MAX_TILE_ANIMS];


/***************** PREPARE TILE ANIMS ***********************/
//
// INPUT: pointer to TILE_ANIM_LIST in tileset file
//

void PrepareTileAnims(int16_t *intPtr)
{
short	i,num,size;

	gNumTileAnims = 0;								// start @ 0

	num = Byteswap16Signed(intPtr++);				// get # tile anims

	for (i=0; i < num; i++)
	{
		intPtr += (16/2);							// skip over animName (16 byte string)
		Add1TileAnim((TileAnimEntryType	*)intPtr);
		intPtr += 2;
		size = Byteswap16Signed(intPtr++);			// get #frames
		intPtr += size;								// skip over to next anim
	}
}


/***************** ADD 1 TILE ANIM *******************/
//
// Adds 1 tile anim to the list
//

void Add1TileAnim(TileAnimEntryType	*intPtr)
{
	gTileAnims[gNumTileAnims].count = 0;
	gTileAnims[gNumTileAnims].index = 0;
	gTileAnims[gNumTileAnims].defPtr = (TileAnimDefType *)intPtr;
	gNumTileAnims++;
}


/**************** UPDATE TILE ANIMATION **********************/

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

