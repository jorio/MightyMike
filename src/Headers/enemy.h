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


void	InitEnemies(void);
Boolean	DoEnemyCollisionDetect(unsigned long);
void	UpdateEnemy(void);
void	CalcEnemyScatterOffset(ObjNode *);
Boolean	EnemyLoseHealth(ObjNode *, short);
void	KillEnemy(ObjNode *);
void	MoveEnemySplat(void);
void	DeleteEnemy(ObjNode *);
Boolean	TrackEnemy(void);
Boolean	TrackEnemy2(void);
void	MoveFrozenEnemy(void);


// ENEMY: CAVEMAN

Boolean	AddEnemy_Caveman(ObjectEntryType *);
void	MoveCaveman_Walker(void);
void	MoveCaveman_Thrower(void);
void	MoveCaveman_Thrower2(void);
void	MoveCaveman_Roller(void);
void	UpdateCaveman(void);
void	DoCavemanMove(void);
void	RollWheel(void);
void	MoveStoneWheel(void);
void	ThrowABone(void);
void	MoveBone(void);
void	DoUngaBunga(void);
void	MoveUngaBunga(void);


// ENEMY: TRICERATOPS

Boolean	AddEnemy_Triceratops(ObjectEntryType *);
void	MoveTriceratops(void);
void	MoveTri_Waiting(void);
void	MoveTri_Charging(void);


// ENEMY: DINO EGG

Boolean	AddDinoEgg(ObjectEntryType *);
void	MoveDinoEgg(void);
void	MakeHatchling(short, short);
void	MoveHatchling(void);


// ENEMY: BABY DINO

Boolean	AddEnemy_BabyDino(ObjectEntryType *);
void	MoveBabyDino(void);
void	MoveBaby_Stand(void);
void	MoveBaby_Hop(void);
void	MoveBaby_Land(void);

// ENEMY: REX

Boolean	AddEnemy_Rex(ObjectEntryType *);
void	MoveRex(void);
void	UpdateRex(void);
void	DoRexMove(void);

// ENEMY: TURTLE

Boolean	AddEnemy_Turtle(ObjectEntryType *);
void	MoveTurtle(void);

