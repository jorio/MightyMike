//
// Playfield.h
//

#pragma once

#define TILENUM_MASK		0x07ff			// b0000011111111111 = mask to filter out tile num from map coords
#define	TILE_PRIORITY_MASK	0x8000			// b1000000000000000 = mask to filter out tile's priority bit (for total tile quick mask)
#define	TILE_PRIORITY_MASK2	0x4000			// b0100000000000000 = mask to filter out tile's priority bit (for pixel masking)

#define	TILE_SIZE			32
#define	TILE_SIZE_SH		5								// for <<32

#define	PF_BUFFER_HEIGHT	(PF_TILE_HEIGHT*TILE_SIZE)
#define	PF_BUFFER_WIDTH		(PF_TILE_WIDTH*TILE_SIZE)
#define	PF_WINDOW_HEIGHT	(PF_BUFFER_HEIGHT-TILE_SIZE)	// dimensions of visible playfield area IN OFFSCREEN BUFFER
#define	PF_WINDOW_WIDTH		(PF_BUFFER_WIDTH-TILE_SIZE)

#define	ITEM_IN_USE			0x8000			// bit 15 = in use flag
#define	ITEM_MEMORY			0x6000			// bits 14..13 = special memory bits
#define	ITEM_NUM			0x0fff			// bits 11..0 = item #

enum								// ALTERNATE TILE TYPES
{
	ALT_TILE_NONE,
	ALT_TILE_DIR_UP,
	ALT_TILE_DIR_UP_RIGHT,
	ALT_TILE_DIR_RIGHT,
	ALT_TILE_DIR_DOWN_RIGHT,
	ALT_TILE_DIR_DOWN,
	ALT_TILE_DIR_DOWN_LEFT,
	ALT_TILE_DIR_LEFT,
	ALT_TILE_DIR_LEFT_UP,
	ALT_TILE_DIR_STOP,
	ALT_TILE_DIR_LOOP
};

enum
{
	TILE_ATTRIB_TOPSOLID = 1,
	TILE_ATTRIB_BOTTOMSOLID = 1<<1,
	TILE_ATTRIB_LEFTSOLID = 1<<2,
	TILE_ATTRIB_RIGHTSOLID = 1<<3,
	TILE_ATTRIB_DEATH = 1<<4,
	TILE_ATTRIB_HURT = 1<<5,
	TILE_ATTRIB_WATER = 1<<7,
	TILE_ATTRIB_WIND = 1<<8,
	TILE_ATTRIB_BULLETGOESTHRU = 1<<9,
	TILE_ATTRIB_STAIRS = 1<<10,
	TILE_ATTRIB_FRICTION = 1<<11,
	TILE_ATTRIB_ICE = 1<<12,
	TILE_ATTRIB_TRACK = 1<<15
};

#define	TILE_ATTRIB_ALLSOLID	(TILE_ATTRIB_TOPSOLID|TILE_ATTRIB_BOTTOMSOLID|TILE_ATTRIB_LEFTSOLID|TILE_ATTRIB_RIGHTSOLID)


struct TileAttribType
{
	uint16_t		bits;
	int16_t			parm0;
	Byte			parm1,parm2;
	Byte			padding[2];					// used to make data stuct even 8 bytes
};
typedef struct TileAttribType TileAttribType;


struct TileAnimDefType
{
	int16_t		speed;					// speed of anim
	uint16_t	baseTile;				// base tile # to animate
	int16_t		numFrames;				// # frames in sequence
	uint16_t 	tileNums[];				// the tile sequence
};
typedef struct TileAnimDefType TileAnimDefType;


struct TileAnimEntryType
{
	int16_t		count;					// current speed count
	uint8_t		index;					// index into sequence
	TileAnimDefType	*defPtr;			// pointer to definition
};
typedef struct TileAnimEntryType TileAnimEntryType;


void	OnChangePlayfieldSize(void);
extern void	ClearTileColorMasks(void);
void LoadTileSet(const char* filename);
extern void	DisposeCurrentMapData(void);
void LoadPlayfield(const char* filename);
extern void	DrawATile(unsigned short, short, short, Boolean);
extern void	DrawATile_Simple(unsigned short, short, short);
extern void	InitPlayfield(void);
extern void	BuildItemList(void);
extern void	ScrollPlayfield(void);
void StopScrollingPlayfield(void);
extern void	SetItemDeleteWindow(void);
extern void	ScrollPlayfield_Down(void);
extern void	ScrollPlayfield_Up(void);
extern void	ScrollPlayfield_Right(void);
extern void	ScrollPlayfield_Left(void);
void ScanForPlayfieldItems(long top, long bottom, long left, long right);
extern void	DoMyScreenScroll(void);
extern void	UpdateViewWindow(void);
extern Boolean	TestCoordinateRange(void);
extern Boolean	TrackItem(void);
extern void	DisplayPlayfield(void);
extern void	DisplayPlayfieldInterlaced(void);
extern Byte	GetAlternateTileInfo(unsigned short, unsigned short);
extern unsigned short	GetMapTileAttribs(unsigned short, unsigned short);
extern TileAttribType	*GetFullMapTileAttribs(unsigned short, unsigned short);
extern void	StartShakeyScreen(short);
extern short	MoveOnPath(long, Boolean);
extern Boolean	NilAdd(ObjectEntryType *);
extern void	CreatePlayfieldPermanentMemory(void);
extern void	UpdateTileAnimation(void);

