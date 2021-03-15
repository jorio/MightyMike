/****************************/
/*     INFOBAR ROUTINES     */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/
#include "myglobals.h"
#include "objecttypes.h"
#include "window.h"
#include "infobar.h"
#include "weapon.h"
#include "shape.h"
#include "io.h"
#include "sound2.h"
#include "miscanims.h"
#include "misc.h"
#include "main.h"
#include "input.h"
#include "structures.h"
#include "externs.h"

/****************************/
/*    CONSTANTS             */
/****************************/


		/* FOR NORMAL SCREEN */

#define	WEAPON_ICON_X	503
#define	WEAPON_ICON_Y	142
#define	WEAPON_LIFE_X	(WEAPON_ICON_X+120)
#define	WEAPON_LIFE_Y	(WEAPON_ICON_Y+47)
#define	WEAPON_ICON_SIZE	76L


#define	HEALTH_X	130
#define	HEALTH_Y	20

#define	SCORE_X		582
#define SCORE_Y		67

#define	COINS_X		560
#define	COINS_Y		426

#define	BUNNIES_X	585
#define	BUNNIES_Y	322

#define	KEYS_X		360
#define	KEYS_Y		35
#define	KEYS_SPACING	45

#define	LIVES_X		504
#define	LIVES_Y		324

		/* FOR FULL SCREEN */

#define	WEAPON_ICON_Xf	256
#define	WEAPON_ICON_Yf	424
#define	WEAPON_LIFE_Xf	(WEAPON_ICON_Xf+93)
#define	WEAPON_LIFE_Yf	(WEAPON_ICON_Yf+35)

#define	KEYS_Xf		169
#define	KEYS_Yf		477
#define	KEYS_SPACINGf 32

#define	SCORE_Xf	84
#define SCORE_Yf	470

#define	BUNNIES_Xf	602
#define	BUNNIES_Yf	467

#define	COINS_Xf	445
#define	COINS_Yf	465

#define	HEALTH_Xf	106
#define	HEALTH_Yf	434

#define	LIVES_Xf	533
#define	LIVES_Yf	467


#define	NUM_DUDES		3

#define	FREE_DUDE_POINTS 45000L


#define	MAX_KEYS				6
#define	MAX_KEYS_DISPLAYABLE	3			// not actual max, just max that can be seen on screen

#define	DEFAULT_HEALTH		4				// default to .. hits

#define	COINS_FOR_HEALTH	400

/**********************/
/*     VARIABLES      */
/**********************/

static	Boolean		gUpdateScore,gUpdateCoins,gUpdateLives;
long		gScore;

short		gNumLives;
long		gNumCoins;

short		gMyHealth,gMyMaxHealth;

static	int			gInfobarXAdjust = (VISIBLE_WIDTH - 640) / 2;
static	int			gInfobarYAdjust = (VISIBLE_HEIGHT - 480);


/**************** INIT HEALTH ********************/

void InitHealth(void)
{
	if (gDifficultySetting == DIFFICULTY_EASY)		// more health in easy mode
	{
		gMyHealth = DEFAULT_HEALTH+1;
		gMyMaxHealth = DEFAULT_HEALTH+1;
	}
	else
	{
		gMyHealth = DEFAULT_HEALTH;
		gMyMaxHealth = DEFAULT_HEALTH;
	}
}


/**************** SHOW HEALTH ********************/

void ShowHealth(void)
{
long	i,x,y;

	if (gMyHealth < 0)						// if negative, then I'm dead
		return;

	if (gPPCFullScreenFlag)
	{
		x = HEALTH_Xf + gInfobarXAdjust;
		y = HEALTH_Yf + gInfobarYAdjust;
	}
	else
	{
		x = HEALTH_X + gInfobarXAdjust;
		y = HEALTH_Y + gInfobarYAdjust;
	}

	for (i=0; i < gMyMaxHealth; i++)
	{
		if (gMyHealth <= i)
		{
			DrawFrameToScreen(x,y,GroupNum_HealthHearts,			// empty heart
						ObjType_HealthHearts,0);
		}
		else
		{
			DrawFrameToScreen(x,y,GroupNum_HealthHearts,			// full heart
						ObjType_HealthHearts,2);
		}

		x += 17;
	}
}


/***************** GIVE ME HEALTH *****************/

void GiveMeHealth(void)
{
	if (gMyHealth >= gMyMaxHealth)				// see if @ max
		return;

	gMyHealth++;

	ShowHealth();
}


/*********************** UPDATE INFOBAR ****************/

void UpdateInfoBar(void)
{
	if (gUpdateScore)
		ShowScore();
	if (gUpdateCoins)
		ShowCoins();
	if (gUpdateLives)
		ShowLives();
}


/******************** INIT SCORE ******************/

void InitScore(void)
{
	gUpdateScore = false;
	gScore = 0L;
}


/******************* SHOW SCORE *******************/

void ShowScore(void)
{
long	i,digit,htab;
long	num,x,y;

	if (gPPCFullScreenFlag)
	{
		x = SCORE_Xf + gInfobarXAdjust;
		y = SCORE_Yf + gInfobarYAdjust;
	}
	else
	{
		x = SCORE_X+ gInfobarXAdjust;
		y = SCORE_Y+ gInfobarYAdjust;
	}

	num = gScore;
	htab = x;				// draw right to left
	for (i=0; i<6; i++)
	{
		digit = num-(num/10*10);
		DrawFrameToScreen_NoMask(htab,y,GroupNum_ScoreNumbers,ObjType_ScoreNumbers,digit);
		htab -= 14;
		num = num/10;
	}

	gUpdateScore = false;
}


/******************** GET POINTS *****************/

void GetPoints(long amount)
{
long	pre;

	pre = gScore / FREE_DUDE_POINTS;		// how many free dudes before adding amount

	gScore += amount;
	gUpdateScore = true;

	if (gDemoMode == DEMO_MODE_OFF)						// no free dudes in demo modes since scores are not good
	{
		if ((gScore / FREE_DUDE_POINTS) != pre)			// if # dudes changed, then get one
		{
			GetFreeDude();
		}
	}
}


/**************** GET FREE DUDE *****************/

void GetFreeDude(void)
{
	gNumLives++;
	MakeMikeMessage(MESSAGE_NUM_FREEDUDE);					// put message
	gUpdateLives = true;
}


/************************** INIT FREE LIVES **********************/

void InitFreeLives(void)
{
	gNumLives = NUM_DUDES;
}


/***************** SHOW LIVES *******************/

void ShowLives(void)
{
	if (gPPCFullScreenFlag)
		PrintNum(gNumLives-1,2,LIVES_Xf+ gInfobarXAdjust,LIVES_Yf+ gInfobarYAdjust);
	else
		PrintNum(gNumLives-1,2,LIVES_X+ gInfobarXAdjust,LIVES_Y+ gInfobarYAdjust);

	gUpdateLives = false;
}


/******************* SHOW WEAPON ICON ********************/

void ShowWeaponIcon(void)
{
	if (gCurrentWeaponType == NO_WEAPON)					// see if blank
	{
		EraseWeaponIcon();
	}
	else
	{
		if (gPPCFullScreenFlag)
			DrawFrameToScreen_NoMask(
					WEAPON_ICON_Xf + gInfobarXAdjust,
					WEAPON_ICON_Yf + gInfobarYAdjust,
					GroupNum_WeaponIcon, ObjType_WeaponIcon, gCurrentWeaponType);
		else
			DrawFrameToScreen_NoMask(
					WEAPON_ICON_X + gInfobarXAdjust,
					WEAPON_ICON_Y + gInfobarYAdjust,
					GroupNum_WeaponIcon, ObjType_WeaponIcon, gCurrentWeaponType);

		ShowWeaponLife();
	}
}

/************** ERASE WEAPON ICON ******************/

void EraseWeaponIcon(void)
{
Rect	r;

	r.top = WEAPON_ICON_Y + gInfobarYAdjust;							// erase icon
	r.bottom = r.top + WEAPON_ICON_SIZE;
	r.left = WEAPON_ICON_X + gInfobarXAdjust;
	r.right = r.left + WEAPON_ICON_SIZE;
	BlankScreenArea(r);

	r.top = WEAPON_LIFE_Y-19 + gInfobarYAdjust;							// erase counter
	r.bottom = r.top + 22;
	r.left = WEAPON_LIFE_X-33 + gInfobarXAdjust;
	r.right = r.left + 42;
	BlankScreenArea(r);
}


/***************** SHOW WEAPON LIFE *******************/
//
// Display life of the current weapon
//

void ShowWeaponLife(void)
{
	if (gPPCFullScreenFlag)
		PrintNum(gMyWeapons[gCurrentWeaponIndex].life,
				 3,WEAPON_LIFE_Xf+gInfobarXAdjust,WEAPON_LIFE_Yf+gInfobarYAdjust);
	else
		PrintNum(gMyWeapons[gCurrentWeaponIndex].life,
				 3,WEAPON_LIFE_X+gInfobarXAdjust,WEAPON_LIFE_Y+gInfobarYAdjust);
}


/******************** INIT COINS *******************/

void InitCoins(void)
{
	gNumCoins = 0;
}


/**************** SHOW COINS *******************/

void ShowCoins(void)
{
long	i,digit,htab;
long	num,x,y;

	if (gPPCFullScreenFlag)
	{
		 x = COINS_Xf + gInfobarXAdjust;
		 y = COINS_Yf + gInfobarYAdjust;
	}
	else
	{
		 x = COINS_X + gInfobarXAdjust;
		 y = COINS_Y + gInfobarYAdjust;
	}



	num = gNumCoins;
	htab = x;				// draw right to left
	for (i=0; i<3; i++)
	{
		digit = num-(num/10*10);
		DrawFrameToScreen_NoMask(htab,y,GroupNum_ScoreNumbers,ObjType_ScoreNumbers,digit);
		htab -= 14;
		num = num/10;
	}

	gUpdateCoins = false;
}


/******************** GET COINS *****************/

void GetCoins(short amount)
{
	gNumCoins += amount;
	gUpdateCoins = true;

//	if ((gNumCoins >= COINS_FOR_HEALTH) && (gMyMaxHealth < MAX_HEARTS))	// see if increase health max
//	{
//		gNumCoins = 0;
//		gMyMaxHealth++;
//		ShowHealth();
//		PlaySound(SOUND_HEALTHDING);
//	}
}


/****************** SHOW NUM BUNNIES **********************/

void ShowNumBunnies(void)
{
long	i,digit,htab;
long	num,x,y;


	if (gPPCFullScreenFlag)
	{
		 x = BUNNIES_Xf + gInfobarXAdjust;
		 y = BUNNIES_Yf + gInfobarYAdjust;
	}
	else
	{
		 x = BUNNIES_X + gInfobarXAdjust;
		 y = BUNNIES_Y + gInfobarYAdjust;
	}


	num = gNumBunnies;
	htab = x;				// draw right to left
	for (i=0; i<2; i++)
	{
		digit = num-(num/10*10);
		DrawFrameToScreen_NoMask(htab,y,GroupNum_ScoreNumbers,ObjType_ScoreNumbers,digit);
		htab -= 14;
		num = num/10;
	}

}


/******************* INIT KEYS *********************/

void InitKeys(void)
{
	gMyKeys[0] = gMyKeys[1] = gMyKeys[2] = gMyKeys[3] = gMyKeys[4] = gMyKeys[5] = false;
}



/************** SHOW KEYS ******************/

void ShowKeys(void)
{
long	type,group,i,s,n,x,y;
Boolean	diffSubs;
long	spacing;

	if (gPPCFullScreenFlag)
		spacing = KEYS_SPACINGf;
	else
		spacing = KEYS_SPACING;

	switch(gSceneNum)
	{
		case	SCENE_JURASSIC:
				group = GroupNum_JurassicKeys;
				type = ObjType_JurassicKeys;
				diffSubs = true;
				break;

		case	SCENE_CANDY:
				group = GroupNum_CandyKeys;
				type = ObjType_CandyKeys;
				diffSubs = true;
				break;

		case	SCENE_CLOWN:
				group = GroupNum_ClownKeys;
				type = ObjType_ClownKeys;
				diffSubs = true;
				break;

		case	SCENE_FAIRY:
				group = GroupNum_FairyKeys;
				type = ObjType_FairyKeys;
				diffSubs = true;
				break;

		case	SCENE_BARGAIN:
				group = GroupNum_BargainKeys;
				type = ObjType_BargainKeys;
				diffSubs = true;
				break;
	}

	s = 0;															// start on anim #0
	n = 0;															// init # done

	if (gPPCFullScreenFlag)
	{
		x = KEYS_Xf + gInfobarXAdjust;
		y = KEYS_Yf + gInfobarYAdjust;
	}
	else
	{
		x = KEYS_X + gInfobarXAdjust;
		y = KEYS_Y + gInfobarYAdjust;
	}

	for (i=0; i < MAX_KEYS; i++)
	{
		if (gMyKeys[i])
		{
			DrawFrameToScreen(x+(n*spacing),y-16,GroupNum_KeyCover, // erase frame
							ObjType_KeyCover,0);
			DrawFrameToScreen(x+(n*spacing),y,group,type,s);			// draw frame
			n++;
		}
		if (diffSubs)												// see if uses different frames
			s++;
	}

	for (; n < MAX_KEYS_DISPLAYABLE; n++)							// erase empty slots
	{
		DrawFrameToScreen(x+(n*spacing),y-16,GroupNum_KeyCover,
						ObjType_KeyCover,0);
	}
}


/******************** SHOW PAUSED ******************/

void ShowPaused(void)
{
	if (!GetSDLKeyState(SDL_SCANCODE_LCTRL) && !GetSDLKeyState(SDL_SCANCODE_RCTRL))			// ctrl key eliminates text for screen grabs!
	{
		if (gPPCFullScreenFlag)
			DrawFrameToScreen_NoMask(320,220,GroupNum_Paused,ObjType_Paused,0);
		else
			DrawFrameToScreen_NoMask(240,220,GroupNum_Paused,ObjType_Paused,0);
	}

	while (GetNeedState(kNeed_UIPause));
	while (!GetNeedState(kNeed_UIPause));
	while (GetNeedState(kNeed_UIPause));
	EraseStore();
}


/*************** ASK IF QUIT *********************/
//
// OUTPUT: True = yes, quit
//

#define	QUIT_X		240
#define	QUIT_Y		220

Boolean	AskIfQuit(void)
{
long	tick;
Boolean	mode;
short	selection;

	if (gDemoMode != DEMO_MODE_OFF)
		return(true);

	tick = 0;
	mode = true;
	selection = 1;		// start on "resume"

	UpdateInput();

	while (!GetNewNeedState(kNeed_UIConfirm))
	{
		UpdateInput();
		
		RegulateSpeed2(1);
		if (!mode)
			DrawFrameToScreen_NoMask(QUIT_X,QUIT_Y,GroupNum_Quit,ObjType_Quit,selection);
		else
			DrawFrameToScreen_NoMask(QUIT_X,QUIT_Y,GroupNum_Quit,ObjType_Quit,2);

		PresentIndexedFramebuffer();

		if (++tick > 10)
		{
			tick = 0;
			mode = !mode;
		}

					/* SEE IF RESUME */

		if (GetNewNeedState(kNeed_UIBack) || GetNewNeedState(kNeed_UIPause))		// see if resume via ESC
		{
			EraseStore();
			return false;
		}

					/* SEE IF RESUME */

		if (GetNewNeedState(kNeed_UIRight) && (selection != 1))
		{
			PlaySound(SOUND_SELECTCHIME);
			selection = 1;
		}
					/* SEE IF QUIT */
		else
		if (GetNewNeedState(kNeed_UILeft) && (selection != 0))
		{
			PlaySound(SOUND_SELECTCHIME);
			selection = 0;
		}
	}
	EraseStore();

	return(!selection);
}



