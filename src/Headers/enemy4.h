//
// Enemy4.h
//

			/* GIANT */

Boolean	AddEnemy_Giant(ObjectEntryType *);
void	MoveGiant(void);
void	MoveGiant_Stand(void);
void	MoveGiant_Hop(void);
void	MoveGiant_Land(void);
void	MakeGiantDeathRing(void);
void	MoveGiantDeathRing(void);


			/* DRAGON */

Boolean	AddEnemy_Dragon(ObjectEntryType *);
void	MoveDragon(void);
void	UpdateDragon(void);
void	DoDragonMove(void);

			/* WITCH */

Boolean	AddEnemy_Witch(ObjectEntryType *);
void	MoveWitch(void);
void	UpdateWitch(void);
void	DoWitchMove(void);
void	DoWitchHaha(void);


			/* BB WOLF */

Boolean	AddEnemy_BBWolf(ObjectEntryType *);
void	MoveBBWolf(void);
void	UpdateBBWolf(void);
void	DoBBWolfMove(void);


		/* SOLDIER */

Boolean	AddEnemy_Soldier(ObjectEntryType *);
void	MoveSoldier(void);
void	UpdateSoldier(void);
void	DoSoldierMove(void);

			/* SPIDER */

Boolean	AddEnemy_Spider(ObjectEntryType *);
void	MoveSpider(void);
