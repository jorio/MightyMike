//
// Bonus.h
//

enum
{
	MISC_POW_TYPE_NUKE,
	MISC_POW_TYPE_FREEZE,
	MISC_POW_TYPE_SHIELD,
	MISC_POW_TYPE_RINGSHOT,
	MISC_POW_TYPE_SPEED,
	MISC_POW_TYPE_FREEDUDE
};


extern void	MakeCoins(short, short, short, short);
extern void	MoveCoin(void);
extern Boolean	AddBunny(ObjectEntryType *);
extern void	MoveBunny(void);
extern void	KillBunny(ObjNode *);
extern void	DeleteBunny(ObjNode *);
extern void	CountBunnies(void);
extern void	DecBunnyCount(void);
extern void	DisplayBunnyRadar(void);
extern Boolean	AddHealthPOW(ObjectEntryType *);
extern Boolean	AddKey(ObjectEntryType *);
extern void	MoveKey(void);
extern void	PutBonusPOW(short, short);
extern Boolean	AddMiscPowerup(ObjectEntryType *);
extern void	GetMiscPOW(short);
extern void	MovePOW(void);
extern void	StartShield(void);
extern void	MoveShield(void);
extern void	FireRingShot(void);
extern void	MoveFireRing(void);
extern void	StartNuke(void);
extern void	MoveNukeObject(void);
extern void	MoveNuke(void);
extern Boolean	AddShipPOW(ObjectEntryType *);
