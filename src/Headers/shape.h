//
// shapes.h
//

#define	PLAYFIELD_RELATIVE	true
#define	SCREEN_RELATIVE		false


extern ObjNode	*MakeNewShape(long, long, long, short, short, short, void *, Boolean);
extern void	LoadShapeTable(Str255, long, Boolean);
extern void	OptimizeShapeTables(void);
extern void	DrawOnBackground(long, long, long, long, long);
extern void	DrawOnBackground_NoMask(long, long, long, long, long, Boolean);
extern void	DrawFrameToScreen(long, long, long, long, long);
extern void	DrawFrameToScreen_NoMask(long, long, long, long, long);
extern void	ZapShapeTable(long);
extern Boolean	CheckFootPriority(unsigned long, unsigned long, long);
extern void	DrawSpriteAt(ObjNode *, long, long, Ptr, long);
extern void	DrawFrameAt_NoMask(long, long, long, long, long, Ptr, long);
extern void	DrawASprite(ObjNode *);
extern void	EraseASprite(ObjNode *);
