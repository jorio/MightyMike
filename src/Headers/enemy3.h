//
// Enemy3.h
//

		/* CHOC BUNNY */

extern Boolean	AddEnemy_ChocBunny(ObjectEntryType *);
extern void	MoveChocBunny(void);
extern void	MoveChoc_Stand(void);
extern void	MoveChoc_Hop(void);
extern void	MoveChoc_Land(void);

		/* GBREAD */

extern Boolean	AddEnemy_GBread(ObjectEntryType *);
extern void	MoveGBread(void);
extern void	MoveGBread_Walk(void);
extern void	MoveGBread_Pop(void);
extern void	UpdateGBread(void);
extern void	DoGBreadMove(void);
extern void	PopAButton(void);
extern void	MoveButton(void);

			/* MINT */

extern Boolean	AddEnemy_Mint(ObjectEntryType *);
extern void	MoveMint(void);

		/* GUMMY BEAR */

extern Boolean	AddEnemy_GBear(ObjectEntryType *);
extern void	MoveGBear(void);
extern void	UpdateGBear(void);
extern void	DoGBearMove(void);
extern void	ExplodeGummy(void);
extern void	MoveTinyGummy(void);
extern void	UpdateTinyGummy(void);
extern void	DoGummyHaha(void);

			/* CARMEL */

extern Boolean	AddEnemy_Carmel(ObjectEntryType *);
extern void	MoveCarmel(void);
extern void	MoveCarmel_Appear(void);
extern void	MoveCarmel_Walk(void);
extern void	MoveCarmel_Shoot(void);
extern void	MoveCarmelDrop(void);

			/* LEMONDROP */

extern Boolean	AddEnemy_LemonDrop(ObjectEntryType *);
extern void	MoveLemon(void);
extern void	SqueezeLemon(void);
extern void	MoveLemonDrop(void);
