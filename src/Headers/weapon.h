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

void	InitWeaponsList(void);
void	InitBullets(void);
Boolean	AddWeaponPowerup(ObjectEntryType *);
void	GetAWeapon(short);
void	CheckFireWeapon(void);
void	SelectNextWeapon(void);
void	RemoveCurrentWeaponFromInventory(void);
void	WeaponHitEnemy(ObjNode *);
void	DeleteWeapon(ObjNode *);
Boolean	CalcWeaponStartCoords(long, long, short *, short *, short *, Byte);
void	MoveBasicBullet(void);
void	MoveBasicRico(void);
Boolean	ShootSuctionCup(void);
Boolean	ShootCake(void);
void	MoveCake(void);
Boolean	ShootOozie(void);
Boolean	ShootRBand(void);
Boolean	ShootToothpaste(void);
void	MoveToothpaste(void);

// WEAPON2

Boolean	ThrowRock(void);
void	MoveRock(void);
Boolean	ShootTracer(void);
Boolean	ShootFlamethrower(void);
void	MoveFlamethrower(void);
Boolean	ShootElephantGun(void);
Boolean	ThrowPie(void);
void	MovePie(void);
void	ExplodePie(ObjNode *);
Boolean	ShootDoubleShot(void);
Boolean	ShootTripleShot(void);
Boolean	ShootRocketGun(void);
void	MoveRocketGun(void);
Boolean	ShootHeatSeek(void);
void	FindHeatSeekTarget(ObjNode *);
void	MoveHeatSeek(void);
Boolean	ShootPixieDust(void);
void	MovePixieDust(void);
