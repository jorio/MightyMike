//
// myguy.h
//

#define MY_ANIMBASE_WALK		0
#define MY_ANIMBASE_STAND		4
#define MY_ANIMBASE_HURT		8
#define MY_ANIMBASE_DIE			12
#define MY_ANIMBASE_LIFTOFF		13
#define MY_ANIMBASE_FLAME		14
#define MY_ANIMBASE_SHOOTWALK	15
#define MY_ANIMBASE_SHOOTSTAND	23
#define MY_ANIMBASE_THROWWALK	31
#define MY_ANIMBASE_THROWSTAND	39
#define MY_ANIMBASE_SWIM		47
#define	MY_ANIMBASE_WATERDIE	51
#define MY_ANIMBASE_WATERHURT	52			//52..55

enum
{
	MY_MODE_BASICWALK,
	MY_MODE_SHOOTWALK,
	MY_MODE_BASICSTAND,
	MY_MODE_SHOOTSTAND,
	MY_MODE_HURT,
	MY_MODE_SWIMMING,
	MY_MODE_FROG,
	MY_MODE_SPACESHIP
};

#define	NO_KEY		(-1)

#define	MY_WALK_SPEED		0x68000L
#define	MAX_ICE_SPEED		0xd8000L
#define	ICE_FRICTION		49/50
#define	GROUND_FRICTION		2/3
#define	WATER_FRICTION		39/40
#define	MY_NORMAL_ACCELERATION		0x10000L


extern void	InitMe(void);
extern void	MoveMe(void);
extern void	MoveMe_Walking(void);
extern void	MoveMe_Swimming(void);
extern void	MoveMe_ShootWalking(void);
extern void	MoveMe_Standing(void);
extern void	MoveMe_ShootStanding(void);
extern void	MoveMe_Hurt(void);
extern void	MoveMe_Die(void);
extern void	MoveMe_Liftoff(void);
extern void	DoMyMove(void);
extern void	MoveMyController(void);
extern void	ControlMeByKeyboard(Boolean *, Boolean *);
extern void	PinMyDeltas(void);
extern Boolean	DoMyCollisionDetect(void);
extern void	HandleMyCenter(void);
extern void	UpdateMe(void);
extern void	UpdateMe_NoBlinkie(void);
extern Boolean	CalcMyAim(void);
extern void	SetMyWalkAnim(void);
extern void	SetMyStandAnim(void);
extern void	SetMySwimAnim(void);
extern void	MeHitBonusObject(ObjNode *);
extern void	MeHitEnemyObject(ObjNode *);
extern void	IGotHurt(void);
extern void	ReviveMe(void);
extern void	DrawMyGun(void);
extern void	StartMyThrow(void);
extern void	CheckIfMeOnMPlatform(void);
extern void	FindMyInitCoords(void);
extern void	MakeFeetSmoke(void);
extern void	StartMyLiftoff(void);
extern void	MoveMyFlame(void);
