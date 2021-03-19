//
// Object.h
//

#define SPRITE_GENRE	0
#define BG_GENRE		1

#define	CLIP_REGION_PLAYFIELD	0			// clip region # reserved for playfield view
#define	CLIP_REGION_SCREEN		1			// clip region # reserved for entire screen

#define	FARTHEST_Z				0xfff0		// $fff0-$ffff are Z's which will NOT sort
#define NEAREST_Z				0x000f		// $0000-$000f are Z's which will NOT sort

void	InitObjectManager(void);
ObjNode	*MakeNewObject(Byte genre, short x, short y, unsigned short z, void (*moveCall)(void));
void	MoveObjects(void);
void	EraseObjects(void);
void	DrawObjects(void);
void	InitRegionList(void);
void	AddUpdateRegion(Rect, Byte);
void	DumpUpdateRegions_DontPresentFramebuffer(void);
void	DumpUpdateRegions(void);
void	GetObjectInfo(void);
void	UpdateObject(void);
void	CalcObjectBox(void);
void	CalcObjectBox2(ObjNode *);
void	DeleteAllObjects(void);
void	DeleteObject(ObjNode *);
void	MoveObject(void);
void	StopObjectMovement(ObjNode *);
void	DeactivateObjectDraw(ObjNode *);
void	SortObjectsByY(void);
void	SimpleObjectMove(void);
void	TweenObjectPosition(ObjNode* node, int32_t factor, int32_t* x, int32_t* y);
