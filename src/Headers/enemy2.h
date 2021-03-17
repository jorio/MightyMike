//
// Enemy2.h
//


// ENEMY: CLOWN CAR

Boolean	AddEnemy_ClownCar(ObjectEntryType *);
void	MoveClownCar_Driving(void);
void	MoveClownCar_Stopped(void);
void	MakeCarClown(void);
void	MoveCarClown(void);
void	DoCarClownMove(void);
void	UpdateCarClown(void);

// ENEMY: CLOWN

Boolean	AddEnemy_Clown(ObjectEntryType *);
void	MoveClown_Walker(void);
void	MoveClown_PieThrower(void);
void	MoveClown_PieThrower2(void);
void	UpdateClown(void);
void	DoClownMove(void);
void	ThrowClownPie(void);
void	MoveClownPie(void);
void	DoClownLaugh(void);

// ENEMY: HAT BUNNY

Boolean	AddMagicHat(ObjectEntryType *);
void	MoveMagicHat(void);
void	MakeHatBunny(void);
void	MoveHatBunny(void);
void	UpdateHatBunny(void);
void	DoHatBunnyMove(void);

// ENEMY: FLOWER CLOWN

Boolean	AddEnemy_FlowerClown(ObjectEntryType *);
void	MoveFlowerClown(void);
void	MoveFlowerClown_Walk(void);
void	MoveFlowerClown_Squirt(void);
void	UpdateFlowerClown(void);
void	DoFlowerClownMove(void);
void	ThrowFlowerClownSquirt(void);
void	MoveFlowerClownSquirt(void);
