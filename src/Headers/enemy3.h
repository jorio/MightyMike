//
// Enemy3.h
//

		/* CHOC BUNNY */

Boolean	AddEnemy_ChocBunny(ObjectEntryType *);
void	MoveChocBunny(void);
void	MoveChoc_Stand(void);
void	MoveChoc_Hop(void);
void	MoveChoc_Land(void);

		/* GBREAD */

Boolean	AddEnemy_GBread(ObjectEntryType *);
void	MoveGBread(void);
void	MoveGBread_Walk(void);
void	MoveGBread_Pop(void);
void	UpdateGBread(void);
void	DoGBreadMove(void);
void	PopAButton(void);
void	MoveButton(void);

			/* MINT */

Boolean	AddEnemy_Mint(ObjectEntryType *);
void	MoveMint(void);

		/* GUMMY BEAR */

Boolean	AddEnemy_GBear(ObjectEntryType *);
void	MoveGBear(void);
void	UpdateGBear(void);
void	DoGBearMove(void);
void	ExplodeGummy(void);
void	MoveTinyGummy(void);
void	UpdateTinyGummy(void);
void	DoGummyHaha(void);

			/* CARMEL */

Boolean	AddEnemy_Carmel(ObjectEntryType *);
void	MoveCarmel(void);
void	MoveCarmel_Appear(void);
void	MoveCarmel_Walk(void);
void	MoveCarmel_Shoot(void);
void	MoveCarmelDrop(void);

			/* LEMONDROP */

Boolean	AddEnemy_LemonDrop(ObjectEntryType *);
void	MoveLemon(void);
void	SqueezeLemon(void);
void	MoveLemonDrop(void);
