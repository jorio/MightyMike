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


void	InitMe(void);
void	MoveMe(void);
void	MoveMe_Walking(void);
void	MoveMe_Swimming(void);
void	MoveMe_ShootWalking(void);
void	MoveMe_Standing(void);
void	MoveMe_ShootStanding(void);
void	MoveMe_Hurt(void);
void	MoveMe_Die(void);
void	MoveMe_Liftoff(void);
void	DoMyMove(void);
void	MoveMyController(void);
void	ControlMeByKeyboard(Boolean *, Boolean *);
void	PinMyDeltas(void);
Boolean	DoMyCollisionDetect(void);
void	HandleMyCenter(void);
void	UpdateMe(void);
void	UpdateMe_NoBlinkie(void);
Boolean	CalcMyAim(void);
void	SetMyWalkAnim(void);
void	SetMyStandAnim(void);
void	SetMySwimAnim(void);
void	MeHitBonusObject(ObjNode *);
void	MeHitEnemyObject(ObjNode *);
void	IGotHurt(void);
void	ReviveMe(void);
void	DrawMyGun(void);
void	StartMyThrow(void);
void	CheckIfMeOnMPlatform(void);
void	FindMyInitCoords(void);
void	MakeFeetSmoke(void);
void	StartMyLiftoff(void);
void	MoveMyFlame(void);
