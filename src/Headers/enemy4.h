//
// Enemy4.h
//

			/* GIANT */

extern Boolean	AddEnemy_Giant(ObjectEntryType *);
extern void	MoveGiant(void);
extern void	MoveGiant_Stand(void);
extern void	MoveGiant_Hop(void);
extern void	MoveGiant_Land(void);
extern void	MakeGiantDeathRing(void);
extern void	MoveGiantDeathRing(void);


			/* DRAGON */

extern Boolean	AddEnemy_Dragon(ObjectEntryType *);
extern void	MoveDragon(void);
extern void	UpdateDragon(void);
extern void	DoDragonMove(void);

			/* WITCH */

extern Boolean	AddEnemy_Witch(ObjectEntryType *);
extern void	MoveWitch(void);
extern void	UpdateWitch(void);
extern void	DoWitchMove(void);
extern void	DoWitchHaha(void);


			/* BB WOLF */

extern Boolean	AddEnemy_BBWolf(ObjectEntryType *);
extern void	MoveBBWolf(void);
extern void	UpdateBBWolf(void);
extern void	DoBBWolfMove(void);


		/* SOLDIER */

extern Boolean	AddEnemy_Soldier(ObjectEntryType *);
extern void	MoveSoldier(void);
extern void	UpdateSoldier(void);
extern void	DoSoldierMove(void);

			/* SPIDER */

extern Boolean	AddEnemy_Spider(ObjectEntryType *);
extern void	MoveSpider(void);
