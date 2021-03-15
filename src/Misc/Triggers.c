/****************************/
/*    	TRIGGERS            */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/****************************/
/*    EXTERNALS             */
/****************************/
#include "myglobals.h"
#include "window.h"
#include "object.h"
#include "triggers.h"
#include "playfield.h"
#include "objecttypes.h"
#include "shape.h"
#include "misc.h"
#include "myguy.h"
#include "infobar.h"
#include "collision.h"
#include "sound2.h"
#include "bonus.h"
#include "externs.h"

/****************************/
/*    CONSTANTS             */
/****************************/

#define StoreNum		Special1
#define	TeleportNum		Special1
#define	KeyNeeded		Special1

#define	FairyDoorBoomFlag	Flag0

#define	TRUCK_SPEED		0x40000L

/**********************/
/*     VARIABLES      */
/**********************/



static	ObjNode		*gTriggerNode;						// pointer to current trigger
static	Byte		gTriggerSides;						// side bits of current trigger collision (which of MyGuy's sides hit trigger?)
Boolean		gTeleportingFlag = false;


										// TRIGGER HANDLER TABLE
										//========================

static	Boolean	(*gTriggerTable[])(void) = {
					DoTrig_Teleport,
					DoTrig_Door,
					DoTrig_FairyDoor,
					DoTrig_BargainDoor
					};


/******************** HANDLE TRIGGER ***************************/
//
// INPUT: triggerNode = ptr to trigger's node
//		  side = side bits from collision.  Which side hit the trigger
//
// OUTPUT: true if we want to handle the trigger as a solid object
//

Boolean HandleTrigger(ObjNode *triggerNode, Byte side)
{
	gTriggerNode = triggerNode;
	gTriggerSides = side;

	if (side & SIDE_BITS_TOP)
	{
		if (triggerNode->TriggerSides & SIDE_BITS_BOTTOM)		// if my top hit, then must be bottom-triggerable
		{
			return(gTriggerTable[triggerNode->TriggerType]());	// call trigger's handler routine
		}
		else
			return(true);
	}
	else
	if (side & SIDE_BITS_BOTTOM)
	{
		if (triggerNode->TriggerSides & SIDE_BITS_TOP)			// if my bottom hit, then must be top-triggerable
		{
			return(gTriggerTable[triggerNode->TriggerType]());	// call trigger's handler routine
		}
		else
			return(true);
	}
	else
	if (side & SIDE_BITS_LEFT)
	{
		if (triggerNode->TriggerSides & SIDE_BITS_RIGHT)		// if my left hit, then must be right-triggerable
		{
			return(gTriggerTable[triggerNode->TriggerType]());	// call trigger's handler routine
		}
		else
			return(true);
	}
	else
	if (side & SIDE_BITS_RIGHT)
	{
		if (triggerNode->TriggerSides & SIDE_BITS_LEFT)			// if my right hit, then must be left-triggerable
		{
			return(gTriggerTable[triggerNode->TriggerType]());	// call trigger's handler routine
		}
		else
			return(true);
	}
	else
		return(true);											// assume it can be solid since didnt trigger
}

/*======================================== TELEPORT =============================================*/


/************************ ADD TELEPORT ********************/
//
// INPUT:   parm[0] :	0 = source
//						1 = destination
//
//			parm[1]	:	match #
//
//			parm[2,3] :	width,height
//

Boolean AddTeleport(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	if	( itemPtr->parm[0] == 1)							// destinations are never added
		return(false);

	newObj = MakeNewObject(BG_GENRE,itemPtr->x,itemPtr->y,NEAREST_Z,SimpleObjectMove);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;							// remember where this came from

	newObj->CType = CTYPE_TRIGGER;							// set collision info
	newObj->CBits = CBITS_ALLSOLID;

	newObj->TriggerSides = ALL_SOLID_SIDES;					// set trigger info
	newObj->TriggerType = TRIGTYPE_TELEPORT;

	if (itemPtr->parm[2] || itemPtr->parm[3])				// see if specified dimensions
	{
		newObj->TopOff = -itemPtr->parm[3];					// get box
		newObj->BottomOff = itemPtr->parm[3];
		newObj->LeftOff = -itemPtr->parm[2];
		newObj->RightOff = itemPtr->parm[2];
	}
	else
	{
		newObj->TopOff = -15;								// set default box
		newObj->BottomOff = 14;
		newObj->LeftOff = -15;
		newObj->RightOff = 14;
	}
	CalcObjectBox2(newObj);

	newObj->TeleportNum = itemPtr->parm[1];					// set teleport match #

	return(true);											// was added flag
}




/**************** DO TRIG: TELEPORT *****************/
//
// INPUT: gTriggerNode = ptr to trigger's node
//
// OUTPUT: true if want to handle trigger as a solid object
//

Boolean DoTrig_Teleport(void)
{
short		matchNum,i;

	matchNum = gTriggerNode->TeleportNum;							// get destination to find

	for (i = 0; i < gNumItems; i++)
	{
		if (gMasterItemList[i].type == PF_OBJ_NUM_TELEPORT)			// look for Teleporter
		{
			if (gMasterItemList[i].parm[1] == matchNum)				// see if matching teleporter
			{
				if (gMasterItemList[i].parm[0] == 1)				// make sure its the destination
				{
					gX.Int = gMasterItemList[i].x;					// move me there
					gY.Int = gMasterItemList[i].y;
					gTeleportingFlag = true;
					StartShield();
					SetMyStandAnim();
					goto okay;
				}
			}
		}
	}
okay:
	if (!gTeleportingFlag)											// see if nothing was found
		DoAlert("No destination for teleporter was found.");

	return(false);
}


/*======================================== CLOWNDOOR =============================================*/


/************************ ADD CLOWNDOOR ********************/

Boolean AddClowndoor(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	if (gDifficultySetting == DIFFICULTY_EASY)				// no doors in easy mode
		return(false);

	if (itemPtr->type & ITEM_MEMORY)						// see if door is already OPEN
		return(false);

	newObj = MakeNewShape(GroupNum_ClownDoor,ObjType_ClownDoor,0,itemPtr->x,
						itemPtr->y,50,SimpleObjectMove,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;							// remember where this came from

	newObj->CType = CTYPE_TRIGGER|CTYPE_MISC;				// set collision info
	newObj->CBits = CBITS_ALLSOLID;

	newObj->TriggerSides = SIDE_BITS_BOTTOM|SIDE_BITS_TOP;	// set trigger info
	newObj->TriggerType = TRIGTYPE_DOOR;

	newObj->TopOff = -34;									// set box
	newObj->BottomOff = 6;
	newObj->LeftOff = -6;
	newObj->RightOff = 66;
	CalcObjectBox2(newObj);

	newObj->KeyNeeded = itemPtr->parm[0];					// remember which key is needed to open

	return(true);											// was added flag
}


/*======================================== CANDYDOOR =============================================*/


/************************ ADD CANDYDOOR ********************/

Boolean AddCandyDoor(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	if (gDifficultySetting == DIFFICULTY_EASY)				// no doors in easy mode
		return(false);

	if (itemPtr->type & ITEM_MEMORY)						// see if door is already OPEN
		return(false);

	newObj = MakeNewShape(GroupNum_CandyDoor,ObjType_CandyDoor,0,itemPtr->x,
						itemPtr->y,50,SimpleObjectMove,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;							// remember where this came from

	newObj->CType = CTYPE_TRIGGER|CTYPE_MISC;			// set collision info
	newObj->CBits = CBITS_ALLSOLID;

	newObj->TriggerSides = SIDE_BITS_BOTTOM|SIDE_BITS_TOP;	// set trigger info
	newObj->TriggerType = TRIGTYPE_DOOR;

	newObj->TopOff = -42;									// set box
	newObj->BottomOff = 9;
	newObj->LeftOff = 15;
	newObj->RightOff = 130;
	CalcObjectBox2(newObj);

	newObj->KeyNeeded = itemPtr->parm[0];					// remember which key is needed to open

	return(true);											// was added flag
}


/*======================================== JURASSIC DOOR =============================================*/


/************************ ADD JURASSIC DOOR ********************/

Boolean AddJurassicDoor(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	if (gDifficultySetting == DIFFICULTY_EASY)				// no doors in easy mode
		return(false);

	if (itemPtr->type & ITEM_MEMORY)						// see if door is already OPEN
		return(false);

	newObj = MakeNewShape(GroupNum_JurassicDoor,ObjType_JurassicDoor,0,itemPtr->x,
						itemPtr->y,50,SimpleObjectMove,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;							// remember where this came from

	newObj->CType = CTYPE_TRIGGER|CTYPE_MISC;				// set collision info
	newObj->CBits = CBITS_ALLSOLID;

	newObj->TriggerSides = SIDE_BITS_BOTTOM|SIDE_BITS_TOP;	// set trigger info
	newObj->TriggerType = TRIGTYPE_DOOR;

	newObj->TopOff = -36;									// set box
	newObj->BottomOff = 3;
	newObj->LeftOff = -3;
	newObj->RightOff = 93;
	CalcObjectBox2(newObj);

	newObj->KeyNeeded = itemPtr->parm[0];					// remember which key is needed to open

	return(true);											// was added flag
}

/*======================================== BARGAIN DOOR =============================================*/


/************************ ADD BARGAIN DOOR ********************/

Boolean AddBargainDoor(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	if (gDifficultySetting == DIFFICULTY_EASY)				// no doors in easy mode
		return(false);

	if (itemPtr->type & ITEM_MEMORY)						// see if door is already OPEN
		return(false);

	newObj = MakeNewShape(GroupNum_BargainDoor,ObjType_BargainDoor,0,itemPtr->x,
						itemPtr->y,50,MoveBargainDoor,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;							// remember where this came from

	newObj->CType = CTYPE_TRIGGER|CTYPE_MISC;				// set collision info
	newObj->CBits = CBITS_ALLSOLID;

	newObj->TriggerSides = SIDE_BITS_BOTTOM|SIDE_BITS_TOP;	// set trigger info
	newObj->TriggerType = TRIGTYPE_BARGAINDOOR;

	newObj->TopOff = -68;									// set box
	newObj->BottomOff = 5;
	newObj->LeftOff = -40;
	newObj->RightOff = 44;
	CalcObjectBox2(newObj);

	newObj->KeyNeeded = itemPtr->parm[0];					// remember which key is needed to open

	return(true);											// was added flag
}


/******************* MOVE BARGAIN DOOR **********************/

void MoveBargainDoor(void)
{
	if (TrackItem())								// see if scrolled off screen
	{
		DeleteObject(gThisNodePtr);
		return;
	}

	if (gThisNodePtr->SubType)						// see if truck is moving
	{
		GetObjectInfo();
		gThisNodePtr->CType = 0;					// not solid when opening

					/* MOVE X */

		gDX = TRUCK_SPEED;
		gX.L += gDX;

					/* COLLISION DETECT */

		gSumDX = gDX;
		gSumDY = gDY;
		CalcObjectBox();
		HandleCollisions(CTYPE_BGROUND|CTYPE_MISC);
		CalcObjectBox();
		UpdateObject();
	}
}



/*======================================== FAIRY DOOR =============================================*/


/************************ ADD FAIRY DOOR ********************/
//
// NOTE: Fairy Door is different than other doors in that
//		the solid attributes are not nuked until AFTER the door
// 		has disintegrated.
//

Boolean AddFairyDoor(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;

	if (gDifficultySetting == DIFFICULTY_EASY)				// no doors in easy mode
		return(false);

	if (itemPtr->type & ITEM_MEMORY)						// see if door is already OPEN
		return(false);

	newObj = MakeNewShape(GroupNum_FairyDoor,ObjType_FairyDoor,0,itemPtr->x,
						itemPtr->y,50,MoveFairyDoor,PLAYFIELD_RELATIVE);
	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;							// remember where this came from

	newObj->CType = CTYPE_TRIGGER|CTYPE_MISC;			// set collision info
	newObj->CBits = CBITS_ALLSOLID;
	newObj->FairyDoorBoomFlag = false;					// hasnt exploded yet

	newObj->TriggerSides = SIDE_BITS_BOTTOM|SIDE_BITS_TOP;	// set trigger info
	newObj->TriggerType = TRIGTYPE_FAIRYDOOR;

	newObj->TopOff = -42;									// set box
	newObj->BottomOff = 10;
	newObj->LeftOff = -6;
	newObj->RightOff = 84;
	CalcObjectBox2(newObj);

	newObj->KeyNeeded = itemPtr->parm[0];					// remember which key is needed to open

	return(true);											// was added flag
}

/******************* MOVE FAIRY DOOR **********************/

void MoveFairyDoor(void)
{
	if (gThisNodePtr->FairyDoorBoomFlag)					// see if door has exploded
	{
		gThisNodePtr->FairyDoorBoomFlag = false;
		gThisNodePtr->CType = 0;
		PlaySound(gSoundNum_DoorOpen);
	}

	if (TrackItem())
	{
		DeleteObject(gThisNodePtr);
	}
}

/**************** DO TRIG: FAIRY DOOR *****************/
//
// Special door trigger for fairy doors.
//
// INPUT: gTriggerNode = ptr to trigger's node
//
// OUTPUT: true if want to handle trigger as a solid object
//

Boolean DoTrig_FairyDoor(void)
{
	if (gMyKeys[gTriggerNode->KeyNeeded])					// see if I've got the key
	{
		SwitchAnim(gTriggerNode,1);							// open the door
		gTriggerNode->ItemIndex->type |= ITEM_MEMORY;		// set memory bits to remember that door is open
		gMyKeys[gTriggerNode->KeyNeeded] = false;			// lose key
		ShowKeys();											// update keys on screen
	}

	return(true);
}



/**************** DO TRIG: DOOR *****************/
//
// INPUT: gTriggerNode = ptr to trigger's node
//
// OUTPUT: true if want to handle trigger as a solid object
//

Boolean DoTrig_Door(void)
{
	if (gMyKeys[gTriggerNode->KeyNeeded])					// see if I've got the key
	{
		SwitchAnim(gTriggerNode,1);							// open the door
		gTriggerNode->CType = 0;
		gTriggerNode->ItemIndex->type |= ITEM_MEMORY;		// set memory bits to remember that door is open
		gMyKeys[gTriggerNode->KeyNeeded] = false;			// lose key
		ShowKeys();											// update keys on screen

		switch(gSceneNum)									// play door sound
		{
			case	SCENE_JURASSIC:
			case	SCENE_CLOWN:
					PlaySound(gSoundNum_DoorOpen);
					break;

		}
	}

	return(true);
}


/**************** DO TRIG: BARGAIN DOOR *****************/
//
// Special door trigger for bargain doors.
//
// INPUT: gTriggerNode = ptr to trigger's node
//
// OUTPUT: true if want to handle trigger as a solid object
//

Boolean DoTrig_BargainDoor(void)
{
	if (gMyKeys[gTriggerNode->KeyNeeded])					// see if I've got the key
	{
		SwitchAnim(gTriggerNode,1);							// open the door
		gTriggerNode->ItemIndex->type |= ITEM_MEMORY;		// set memory bits to remember that door is open
		gMyKeys[gTriggerNode->KeyNeeded] = false;			// lose key
		ShowKeys();											// update keys on screen
		PlaySound(gSoundNum_DoorOpen);
	}

	return(true);
}



