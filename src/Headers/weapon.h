//
// weapon.h
//

#pragma once

#include "objecttypes.h"

#define	MAX_WEAPONS		50				// max weapons allowed in weapon list

struct WeaponType
{
	Byte		type;
	short		life;
};
typedef struct WeaponType WeaponType;


enum
{
	WEAPON_TYPE_SUCTIONCUP,
	WEAPON_TYPE_CAKE,
	WEAPON_TYPE_OOZIE,
	WEAPON_TYPE_RBAND,
	WEAPON_TYPE_TOOTHPASTE,
	WEAPON_TYPE_TRACER,
	WEAPON_TYPE_PIXIEDUST,
	WEAPON_TYPE_ROCK,
	WEAPON_TYPE_FIREHOSE,
	WEAPON_TYPE_ELEPHANTGUN,
	WEAPON_TYPE_PIE,
	WEAPON_TYPE_DOUBLESHOT,
	WEAPON_TYPE_TRIPLESHOT,
	WEAPON_TYPE_FLAMETHROWER,
	WEAPON_TYPE_ROCKETGUN
};


#define		WeaponPower		Misc1

#define		NO_WEAPON		0xff

#define	WS_SHOOT		0
#define	WS_THROW		1

							// WEAPON OBJECT TYPE LIST
							//====================================
enum
{
	ObjType_SuctionCup,
	ObjType_Cake,
	ObjType_Oozie,
	ObjType_RBand,
	ObjType_Toothpaste,
	ObjType_Rock,
	ObjType_Tracer,
	ObjType_HeatSeek,
	ObjType_ElephantGun,
	ObjType_Pie,
	ObjType_WeaponPOWs,
	ObjType_DoubleShot,
	ObjType_TripleShot,
	ObjType_Flamethrower,
	ObjType_MiscPOWs,
	ObjType_MyShieldEffect,
	ObjType_RocketGun,
	ObjType_Nuke,
	ObjType_PixieDust
};

enum
{
	GroupNum_SuctionCup = GROUP_WEAPONS,
	GroupNum_Cake = GROUP_WEAPONS,
	GroupNum_Oozie = GROUP_WEAPONS,
	GroupNum_RBand = GROUP_WEAPONS,
	GroupNum_Toothpaste = GROUP_WEAPONS,
	GroupNum_Rock = GROUP_WEAPONS,
	GroupNum_Tracer = GROUP_WEAPONS,
	GroupNum_HeatSeek = GROUP_WEAPONS,
	GroupNum_ElephantGun = GROUP_WEAPONS,
	GroupNum_Pie = GROUP_WEAPONS,
	GroupNum_WeaponPOWs = GROUP_WEAPONS,
	GroupNum_DoubleShot = GROUP_WEAPONS,
	GroupNum_TripleShot = GROUP_WEAPONS,
	GroupNum_Flamethrower = GROUP_WEAPONS,
	GroupNum_MiscPOWs = GROUP_WEAPONS,
	GroupNum_MyShieldEffect = GROUP_WEAPONS,
	GroupNum_RocketGun = GROUP_WEAPONS,
	GroupNum_Nuke = GROUP_WEAPONS,
	GroupNum_PixieDust = GROUP_WEAPONS
};


		/* WEAPON1 */

extern void	InitWeaponsList(void);
extern void	InitBullets(void);
extern Boolean	AddWeaponPowerup(ObjectEntryType *);
extern void	GetAWeapon(short);
extern void	CheckFireWeapon(void);
extern void	SelectNextWeapon(void);
extern void	RemoveCurrentWeaponFromInventory(void);
extern void	WeaponHitEnemy(ObjNode *);
extern void	DeleteWeapon(ObjNode *);
extern Boolean	CalcWeaponStartCoords(long, long, short *, short *, short *, Byte);
extern void	MoveBasicBullet(void);
extern void	MoveBasicRico(void);
extern Boolean	ShootSuctionCup(void);
extern Boolean	ShootCake(void);
extern void	MoveCake(void);
extern Boolean	ShootOozie(void);
extern Boolean	ShootRBand(void);
extern Boolean	ShootToothpaste(void);
extern void	MoveToothpaste(void);

// WEAPON2

extern Boolean	ThrowRock(void);
extern void	MoveRock(void);
extern Boolean	ShootTracer(void);
extern Boolean	ShootFlamethrower(void);
extern void	MoveFlamethrower(void);
extern Boolean	ShootElephantGun(void);
extern Boolean	ThrowPie(void);
extern void	MovePie(void);
extern void	ExplodePie(ObjNode *);
extern Boolean	ShootDoubleShot(void);
extern Boolean	ShootTripleShot(void);
extern Boolean	ShootRocketGun(void);
extern void	MoveRocketGun(void);
extern Boolean	ShootHeatSeek(void);
extern void	FindHeatSeekTarget(ObjNode *);
extern void	MoveHeatSeek(void);
extern Boolean	ShootPixieDust(void);
extern void	MovePixieDust(void);
