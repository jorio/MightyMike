//
// Enemy2.h
//


// ENEMY: CLOWN CAR

extern Boolean	AddEnemy_ClownCar(ObjectEntryType *);
extern void	MoveClownCar_Driving(void);
extern void	MoveClownCar_Stopped(void);
extern void	MakeCarClown(void);
extern void	MoveCarClown(void);
extern void	DoCarClownMove(void);
extern void	UpdateCarClown(void);

// ENEMY: CLOWN

extern Boolean	AddEnemy_Clown(ObjectEntryType *);
extern void	MoveClown_Walker(void);
extern void	MoveClown_PieThrower(void);
extern void	MoveClown_PieThrower2(void);
extern void	UpdateClown(void);
extern void	DoClownMove(void);
extern void	ThrowClownPie(void);
extern void	MoveClownPie(void);
extern void	DoClownLaugh(void);

// ENEMY: HAT BUNNY

extern Boolean	AddMagicHat(ObjectEntryType *);
extern void	MoveMagicHat(void);
extern void	MakeHatBunny(void);
extern void	MoveHatBunny(void);
extern void	UpdateHatBunny(void);
extern void	DoHatBunnyMove(void);

// ENEMY: FLOWER CLOWN

extern Boolean	AddEnemy_FlowerClown(ObjectEntryType *);
extern void	MoveFlowerClown(void);
extern void	MoveFlowerClown_Walk(void);
extern void	MoveFlowerClown_Squirt(void);
extern void	UpdateFlowerClown(void);
extern void	DoFlowerClownMove(void);
extern void	ThrowFlowerClownSquirt(void);
extern void	MoveFlowerClownSquirt(void);
