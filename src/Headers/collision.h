

//
// COLLISION.h
//

extern	 void CollisionDetect(ObjNode *baseNode,unsigned long CType);
extern Byte	HandleCollisions(unsigned long);
extern void	DoSimpleCollision(unsigned long);
extern Boolean	DoPointCollision(unsigned short, unsigned short, unsigned long);
extern void	AddBGCollisions(ObjNode *);

