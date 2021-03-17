

//
// COLLISION.h
//

extern	 void CollisionDetect(ObjNode *baseNode,unsigned long CType);
Byte	HandleCollisions(unsigned long);
void	DoSimpleCollision(unsigned long);
Boolean	DoPointCollision(unsigned short, unsigned short, unsigned long);
void	AddBGCollisions(ObjNode *);

