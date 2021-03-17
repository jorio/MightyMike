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


void	MakeCoins(short, short, short, short);
void	MoveCoin(void);
Boolean	AddBunny(ObjectEntryType *);
void	MoveBunny(void);
void	DeleteBunny(ObjNode *);
void	CountBunnies(void);
void	DecBunnyCount(void);
void	DisplayBunnyRadar(void);
Boolean	AddHealthPOW(ObjectEntryType *);
Boolean	AddKey(ObjectEntryType *);
void	MoveKey(void);
void	PutBonusPOW(short, short);
Boolean	AddMiscPowerup(ObjectEntryType *);
void	GetMiscPOW(short);
void	MovePOW(void);
void	StartShield(void);
void	MoveShield(void);
void	FireRingShot(void);
void	MoveFireRing(void);
void	StartNuke(void);
void	MoveNukeObject(void);
void	MoveNuke(void);
Boolean	AddShipPOW(ObjectEntryType *);
