//
// Enemy.h
//


#define	EnemyTargetXOff		Special1			// for moving towards me
#define	EnemyTargetYOff		Special2

#define	MAX_ENEMIES			6

#define	FULL_ENEMY_COLLISION	(CTYPE_BGROUND|CTYPE_MYBULLET|CTYPE_HURTENEMY|CTYPE_MISC|CTYPE_BONUS)
#define	ENEMY_NO_BG_COLLISION	(CTYPE_MYBULLET|CTYPE_HURTENEMY|CTYPE_MISC|CTYPE_BONUS)
#define	ENEMY_NO_BULLET_COLLISION	(CTYPE_BGROUND|CTYPE_MISC|CTYPE_BONUS)


// ENEMY


extern void	InitEnemies(void);
extern Boolean	DoEnemyCollisionDetect(unsigned long);
extern void	UpdateEnemy(void);
extern void	CalcEnemyScatterOffset(ObjNode *);
extern Boolean	EnemyLoseHealth(ObjNode *, short);
extern void	KillEnemy(ObjNode *);
extern void	MoveEnemySplat(void);
extern void	DeleteEnemy(ObjNode *);
extern Boolean	TrackEnemy(void);
extern Boolean	TrackEnemy2(void);
extern void	MoveFrozenEnemy(void);


// ENEMY: CAVEMAN

extern Boolean	AddEnemy_Caveman(ObjectEntryType *);
extern void	MoveCaveman_Walker(void);
extern void	MoveCaveman_Thrower(void);
extern void	MoveCaveman_Thrower2(void);
extern void	MoveCaveman_Roller(void);
extern void	UpdateCaveman(void);
extern void	DoCavemanMove(void);
extern void	RollWheel(void);
extern void	MoveStoneWheel(void);
extern void	ThrowABone(void);
extern void	MoveBone(void);
extern void	DoUngaBunga(void);
extern void	MoveUngaBunga(void);


// ENEMY: TRICERATOPS

extern Boolean	AddEnemy_Triceratops(ObjectEntryType *);
extern void	MoveTriceratops(void);
extern void	MoveTri_Waiting(void);
extern void	MoveTri_Charging(void);


// ENEMY: DINO EGG

extern Boolean	AddDinoEgg(ObjectEntryType *);
extern void	MoveDinoEgg(void);
extern void	MakeHatchling(short, short);
extern void	MoveHatchling(void);


// ENEMY: BABY DINO

extern Boolean	AddEnemy_BabyDino(ObjectEntryType *);
extern void	MoveBabyDino(void);
extern void	MoveBaby_Stand(void);
extern void	MoveBaby_Hop(void);
extern void	MoveBaby_Land(void);

// ENEMY: REX

extern Boolean	AddEnemy_Rex(ObjectEntryType *);
extern void	MoveRex(void);
extern void	UpdateRex(void);
extern void	DoRexMove(void);

// ENEMY: TURTLE

extern Boolean	AddEnemy_Turtle(ObjectEntryType *);
extern void	MoveTurtle(void);

