//
// TileAnim.h
//

struct TileAnimDefType
{
	short		speed;						// speed of anim
	unsigned short	baseTile;				// base tile # to animate
	short		numFrames;					// # frames in sequence
	unsigned short	tileNums[];				// the tile sequence
};
typedef struct TileAnimDefType TileAnimDefType;


struct TileAnimEntryType
{
	short	count;						// current speed count
	Byte	index;							// index into sequence
	TileAnimDefType	*defPtr;				// pointer to definition
};
typedef struct TileAnimEntryType TileAnimEntryType;




extern void	PrepareTileAnims(short *);
extern void	Add1TileAnim(TileAnimEntryType *);
extern void	UpdateTileAnimation(void);
