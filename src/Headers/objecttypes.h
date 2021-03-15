//
// ObjectTypes.h
//

#pragma once

							// SHAPE TABLE MASTER GROUPS
							//===========================
enum
{
	GROUP_MAIN,
	GROUP_INFOBAR,
	GROUP_AREA_SPECIFIC,
	GROUP_AREA_SPECIFIC2,
	GROUP_WEAPONS,
	GROUP_OVERHEAD,
	GROUP_WIN
};


							// MAIN GAME OBJECT TYPE LIST
							//====================================
enum
{
	ObjType_MyGuy,
	ObjType_Coin,
	ObjType_Shadow,
	ObjType_Splat,
	ObjType_Bunny,
	ObjType_Message,
	ObjType_PlayerSignal,
	ObjType_RadarBlip,
	ObjType_Splash,
	ObjType_KeyColor
};

enum
{
	GroupNum_MyGuy = GROUP_MAIN,
	GroupNum_Coin = GROUP_MAIN,
	GroupNum_Shadow = GROUP_MAIN,
	GroupNum_Splat = GROUP_MAIN,
	GroupNum_Bunny = GROUP_MAIN,
	GroupNum_Message = GROUP_MAIN,
	GroupNum_PlayerSignal = GROUP_MAIN,
	GroupNum_RadarBlip = GROUP_MAIN,
	GroupNum_Splash = GROUP_MAIN,
	GroupNum_KeyColor = GROUP_MAIN
};



							// JURASSIC OBJECT TYPE LIST
							//================================

enum
{
	ObjType_Caveman = 0,
	ObjType_Triceratops,
	ObjType_StoneWheel,
	ObjType_Bone,
	ObjType_Turtle,
	ObjType_ManEatingPlant = 0,
	ObjType_DinoEgg,
	ObjType_BabyDino,
	ObjType_Rex,
	ObjType_JurassicHealth,
	ObjType_JurassicKeys,
	ObjType_JurassicDoor
};

enum
{
	GroupNum_Caveman = GROUP_AREA_SPECIFIC,
	GroupNum_Triceratops = GROUP_AREA_SPECIFIC,
	GroupNum_StoneWheel = GROUP_AREA_SPECIFIC,
	GroupNum_Bone = GROUP_AREA_SPECIFIC,
	GroupNum_Turtle = GROUP_AREA_SPECIFIC,

	GroupNum_ManEatingPlant = GROUP_AREA_SPECIFIC2,
	GroupNum_DinoEgg = GROUP_AREA_SPECIFIC2,
	GroupNum_BabyDino = GROUP_AREA_SPECIFIC2,
	GroupNum_Rex = GROUP_AREA_SPECIFIC2,
	GroupNum_JurassicHealth = GROUP_AREA_SPECIFIC2,
	GroupNum_JurassicKeys = GROUP_AREA_SPECIFIC2,
	GroupNum_JurassicDoor = GROUP_AREA_SPECIFIC2
};

							// CLOWN OBJECT TYPE LIST
							//=======================

enum
{
	ObjType_Clown = 0,
	ObjType_MagicHat,

	ObjType_JackInTheBox = 0,
	ObjType_FlowerClown,
	ObjType_ClownDoor,
	ObjType_Star,
	ObjType_ClownHealth,
	ObjType_ClownKeys,
	ObjType_ClownCar
};

enum
{
	GroupNum_Clown = GROUP_AREA_SPECIFIC,
	GroupNum_MagicHat = GROUP_AREA_SPECIFIC,

	GroupNum_JackInTheBox = GROUP_AREA_SPECIFIC2,
	GroupNum_FlowerClown = GROUP_AREA_SPECIFIC2,
	GroupNum_ClownDoor = GROUP_AREA_SPECIFIC2,
	GroupNum_Star = GROUP_AREA_SPECIFIC2,
	GroupNum_ClownHealth = GROUP_AREA_SPECIFIC2,
	GroupNum_ClownKeys = GROUP_AREA_SPECIFIC2,
	GroupNum_ClownCar = GROUP_AREA_SPECIFIC2
};

							// CANDY OBJECT TYPE LIST
							//=======================

enum
{
	ObjType_CandyMPlatform = 0,
	ObjType_CandyDoor,
	ObjType_ChocBunny,
	ObjType_GBread,
	ObjType_RedGummy = 0,
	ObjType_CandyKeys,
	ObjType_Carmel,
	ObjType_GumBall,
	ObjType_LemonDrop,
	ObjType_CandyHealth,
	ObjType_Mint
};

enum
{
	GroupNum_CandyMPlatform = GROUP_AREA_SPECIFIC,
	GroupNum_CandyDoor = GROUP_AREA_SPECIFIC,
	GroupNum_ChocBunny = GROUP_AREA_SPECIFIC,
	GroupNum_GBread = GROUP_AREA_SPECIFIC,

	GroupNum_RedGummy = GROUP_AREA_SPECIFIC2,
	GroupNum_CandyKeys = GROUP_AREA_SPECIFIC2,
	GroupNum_Carmel = GROUP_AREA_SPECIFIC2,
	GroupNum_GumBall = GROUP_AREA_SPECIFIC2,
	GroupNum_LemonDrop = GROUP_AREA_SPECIFIC2,
	GroupNum_CandyHealth = GROUP_AREA_SPECIFIC2,
	GroupNum_Mint = GROUP_AREA_SPECIFIC2
};



							// FAIRY OBJECT TYPE LIST
							//=======================

enum
{
	ObjType_Witch = 0,
	ObjType_Dragon,
	ObjType_Giant,
	ObjType_BBWolf,
	ObjType_Muffit,
	ObjType_Spider = 0,
	ObjType_FairyHealth,
	ObjType_MeFrog,
	ObjType_Soldier,
	ObjType_FairyDoor,
	ObjType_FairyKeys,
	ObjType_FrogPoof
};

enum
{
	GroupNum_Witch = GROUP_AREA_SPECIFIC,
	GroupNum_Dragon = GROUP_AREA_SPECIFIC,
	GroupNum_Giant = GROUP_AREA_SPECIFIC,
	GroupNum_BBWolf = GROUP_AREA_SPECIFIC,
	GroupNum_Muffit = GROUP_AREA_SPECIFIC,

	GroupNum_Spider = GROUP_AREA_SPECIFIC2,
	GroupNum_FairyHealth = GROUP_AREA_SPECIFIC2,
	GroupNum_MeFrog = GROUP_AREA_SPECIFIC2,
	GroupNum_Soldier = GROUP_AREA_SPECIFIC2,
	GroupNum_FairyDoor = GROUP_AREA_SPECIFIC2,
	GroupNum_FairyKeys = GROUP_AREA_SPECIFIC2,
	GroupNum_FrogPoof = GROUP_AREA_SPECIFIC2
};


							// BARGAIN OBJECT TYPE LIST
							//=======================

enum
{
	ObjType_BadBattery,
	ObjType_BargainHealth,
	ObjType_Slinky,
	ObjType_8Ball,
	ObjType_SpaceShip,
	ObjType_Robot = 0,
	ObjType_Doggy,
	ObjType_BargainDoor,
	ObjType_BargainKeys,
	ObjType_Top,
	ObjType_FireHydrant,
	ObjType_RaceCar
};


enum
{
	GroupNum_BadBattery = GROUP_AREA_SPECIFIC,
	GroupNum_BargainHealth = GROUP_AREA_SPECIFIC,
	GroupNum_Slinky = GROUP_AREA_SPECIFIC,
	GroupNum_8Ball = GROUP_AREA_SPECIFIC,
	GroupNum_SpaceShip = GROUP_AREA_SPECIFIC,
	GroupNum_Robot = GROUP_AREA_SPECIFIC2,
	GroupNum_Doggy = GROUP_AREA_SPECIFIC2,
	GroupNum_BargainDoor = GROUP_AREA_SPECIFIC2,
	GroupNum_BargainKeys = GROUP_AREA_SPECIFIC2,
	GroupNum_Top = GROUP_AREA_SPECIFIC2,
	GroupNum_FireHydrant = GROUP_AREA_SPECIFIC2,
	GroupNum_RaceCar = GROUP_AREA_SPECIFIC2
};

						// HIGH SCORES OBJECT TYPE LIST
						//======================================
enum
{
	ObjType_BigFont
};

enum
{
	GroupNum_BigFont = GROUP_WIN
};


//==============================================================================

											// BG MAP ITEM #'S
											//================
enum
{
	PF_OBJ_NUM_TELEPORT = 17
};






