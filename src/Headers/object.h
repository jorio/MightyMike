//
// Object.h
//

#define SPRITE_GENRE	0
#define BG_GENRE		1

#define	CLIP_REGION_PLAYFIELD	0			// clip region # reserved for playfield view
#define	CLIP_REGION_SCREEN		1			// clip region # reserved for entire screen

#define	FARTHEST_Z				0xfff0		// $fff0-$ffff are Z's which will NOT sort
#define NEAREST_Z				0x000f		// $0000-$000f are Z's which will NOT sort

extern void	InitObjectManager(void);
extern ObjNode	*MakeNewObject(Byte, short, short, unsigned short, void *);
extern void	MoveObjects(void);
extern void	EraseObjects(void);
extern void	DrawObjects(void);
extern void	InitRegionList(void);
extern void	AddUpdateRegion(Rect, Byte);
extern void	DumpUpdateRegions(void);
extern void	GetObjectInfo(void);
extern void	UpdateObject(void);
extern void	CalcObjectBox(void);
extern void	CalcObjectBox2(ObjNode *);
extern void	DeleteAllObjects(void);
extern void	DeleteObject(ObjNode *);
extern void	MoveObject(void);
extern void	DeactivateObjectDraw(ObjNode *);
extern void	SortObjectsByY(void);
extern void	SimpleObjectMove(void);
