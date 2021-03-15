/****************************/
/*    	MISC ANIMS          */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/****************************/
/*    EXTERNALS             */
/****************************/
#include "myglobals.h"
#include "object.h"
#include "miscanims.h"
#include "misc.h"
#include "myguy.h"
#include "shape.h"
#include "sound2.h"
#include "objecttypes.h"
#include "cinema.h"
#include "externs.h"

/****************************/
/*    CONSTANTS             */
/****************************/

#define PLAYER_SIGNAL_X		230
#define PLAYER_SIGNAL_Y		200

/**********************/
/*     VARIABLES      */
/**********************/


/***************** MAKE SHADOW ******************/

ObjNode *MakeShadow(ObjNode *parentObj,Byte shadowSize)
{
register ObjNode *newObj;

	newObj = MakeNewShape(GroupNum_Shadow,ObjType_Shadow,shadowSize,
				parentObj->X.Int,parentObj->Y.Int,FARTHEST_Z,MoveShadow,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(nil);

	newObj->ShadowIndex = parentObj;			// remember ptr to parent of shadow

	return(newObj);
}


/**************** MOVE SHADOW *******************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveShadow(void)
{
ObjNode *o;

	o = (ObjNode *)gThisNodePtr->ShadowIndex;

	GAME_ASSERT_MESSAGE(o, "Shadow node lost its owner!");

	gThisNodePtr->X.Int = o->X.Int;		// follow owner
	gThisNodePtr->Y.Int = o->Y.Int;
	
	gThisNodePtr->DX = o->DX;			// for movement extrapolation
	gThisNodePtr->DY = o->DY;

}

//============================================================================================


/***************** MAKE MIKE MESSAGE ******************/
//
// Puts message balloon above me
//

void MakeMikeMessage(short messageNum)
{
ObjNode *newObj;
static	int	messageDurations[] = {
							GAME_FPS*2,		// dont worry, ill save you
							GAME_FPS*2,		// come here rodent
							GAME_FPS*2,		// take that you fiend
							GAME_FPS*2,		// eat my dust
							GAME_FPS,		// food
							GAME_FPS*2,		// free dude
							GAME_FPS*4/3,	// ouch!
							GAME_FPS*2,		// no more nice guy
							GAME_FPS*2		// fire in the hole
							};

static	short	messageSounds[] = {
				SOUND_ILLSAVE,
				SOUND_COMEHERERODENT,
				SOUND_TAKETHAT,
				SOUND_EATMYDUST,
				SOUND_FOOD,
				SOUND_FREEDUDE,
				SOUND_MIKEHURT,
				SOUND_NICEGUY,
				SOUND_FIREHOLE
				};


	if (gMyNodePtr->SubType == MY_ANIMBASE_LIFTOFF)		// cant do messages while flying away
		return;

	if (gMyNodePtr->OwnerToMessageNode != nil)			// see if there's already a message
		return;

						/* MAKE MESSAGE SHAPE */

	newObj = MakeNewShape(GroupNum_Message,ObjType_Message,messageNum,
				gMyNodePtr->X.Int,gMyNodePtr->Y.Int,gMyNodePtr->Z,MoveMessage,PLAYFIELD_RELATIVE);

	if (newObj == nil)
		return;

	newObj->MessageTimer = messageDurations[messageNum];			// set sprite timer
	newObj->TileMaskFlag = false;								// wont be tile masked
	newObj->MessageToOwnerNode = gMyNodePtr;					// point to Mike
	gMyNodePtr->OwnerToMessageNode = newObj;					// point to message

	PlaySound(messageSounds[messageNum]);
}


/**************** MOVE MESSAGE *******************/
//
// INPUT: gThisNodePtr = Pointer to current working node
//

void MoveMessage(void)
{
	GAME_ASSERT_MESSAGE(gThisNodePtr->MessageToOwnerNode, "Message node lost its owner!");

	if (--gThisNodePtr->MessageTimer <= 0)							// see if delete
	{
		gThisNodePtr->MessageToOwnerNode->OwnerToMessageNode = nil;		// make sure owner knows its gone
		DeleteObject(gThisNodePtr);
		return;
	}

	gThisNodePtr->X.Int = gThisNodePtr->MessageToOwnerNode->X.Int;	// align with owner
	gThisNodePtr->Y.Int = gThisNodePtr->MessageToOwnerNode->Y.Int;

	gThisNodePtr->DX = gThisNodePtr->MessageToOwnerNode->DX;		// for movement extrapolation
	gThisNodePtr->DY = gThisNodePtr->MessageToOwnerNode->DY;
}



//=============================================================================================




/****************** PUT PLAYER SIGNAL ******************/
//
// Puts the PLAYER 1 / PLAYER 2 signal on screen
//
// INPUT: 0=overhead map, 1 = game
//

void PutPlayerSignal(short	mode)
{
register	ObjNode			*newObj;

	if (!gPlayerMode)							// if 1 player mode, then dont bother
		return;

	if (mode == 1)
	{
		newObj = MakeNewShape(GroupNum_PlayerSignal,ObjType_PlayerSignal,gCurrentPlayer,
				PLAYER_SIGNAL_X+gScrollX,PLAYER_SIGNAL_Y+gScrollY,NEAREST_Z,MovePlayerSignal,PLAYFIELD_RELATIVE);
		if (newObj == nil)
			return;

		newObj->TileMaskFlag = false;						// doesnt use tile masks
		newObj->Special1 = GAME_FPS*3;
	}
	else
	{
		newObj = MakeNewShape(GroupNum_OHMPlayerSignal,ObjType_OHMPlayerSignal,gCurrentPlayer,
					320,240,NEAREST_Z,MovePlayerSignalOHM,SCREEN_RELATIVE);
	}
}


/*************** MOVE PLAYER SIGNAL ****************/
//
// For playfield
//

void MovePlayerSignal(void)
{
	gThisNodePtr->X.Int = gScrollX+PLAYER_SIGNAL_X;		// keep aligned with screen
	gThisNodePtr->Y.Int = gScrollY+PLAYER_SIGNAL_Y;

	if ((gFrames&b11) == 0)									// make flash
		gThisNodePtr->DrawFlag = !gThisNodePtr->DrawFlag;

	if (gThisNodePtr->Special1-- < 0)						// see if done
		DeleteObject(gThisNodePtr);
}


/*************** MOVE PLAYER SIGNAL OHM ****************/
//
// For overhead map
//

void MovePlayerSignalOHM(void)
{
	if ((gFrames&b111) == 0)									// make flash
	{
		if (gThisNodePtr->DrawFlag)
			DeactivateObjectDraw(gThisNodePtr);
		else
			gThisNodePtr->DrawFlag = true;
	}
}


/***************** MAKE SPLASH ******************/

void MakeSplash(short x,short y,short z)
{
	MakeNewShape(GroupNum_Splash,ObjType_Splash,0,x,y,z,nil,PLAYFIELD_RELATIVE);
	PlaySound(SOUND_SPLASH);
}



/*************** ADD KEY COLOR ******************/

Boolean AddKeyColor(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

//	if (gDifficultySetting == DIFFICULTY_EASY)				// no doors/keys in easy mode
//		return(false);

	newObj = MakeNewShape(GroupNum_KeyColor,ObjType_KeyColor,itemPtr->parm[0],
			itemPtr->x,itemPtr->y,FARTHEST_Z,SimpleObjectMove,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;					// remember where this came from

	return(true);									// was added
}

