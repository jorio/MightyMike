/****************************/
/*   	  INPUT.C	   	    */
/* (c)1997-99 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/

#include <stdlib.h>
#include <DrawSprocket.h>
#include <InputSprocket.h>
#include <CursorDevices.h>
#include <Traps.h>
#include <TextUtils.h>
#include <FixMath.h>
#include "myglobals.h"
#include "misc.h"
#include "input.h"
#include "windows.h"

extern	unsigned long 		gOriginalSystemVolume;
extern	short				gMainAppRezFile;
extern	Byte				gDemoMode;
extern	Boolean				gAbortedFlag,gGameOverFlag,gAbortDemoFlag;
extern	DSpContextReference gDisplayContext;

/**********************/
/*     PROTOTYPES     */
/**********************/

static void InitMouseDevice(void);


/****************************/
/*    CONSTANTS             */
/****************************/

#define	USE_ISP		1


/**********************/
/*     VARIABLES      */
/**********************/


KeyMap gKeyMap,gNewKeys,gOldKeys,gKeyMap_Real,gNewKeys_Real,gOldKeys_Real;

Boolean	gReadFromInputSprockets = false;
Boolean	gISpActive 				= false;
Boolean	gPlayerUsingKeyControl 	= false;
Boolean	gISPInitialized			= false;


		/* CONTORL NEEDS */

#define	NEED_NUM_MOUSEMOTION	4
#define	NUM_CONTROL_NEEDS		10

static ISpNeed	gControlNeeds[NUM_CONTROL_NEEDS] =
{
	{													// 0
		"Fire Weapon",
		131,
		0,
		0,
		kISpElementKind_Button,
		kISpElementLabel_Btn_Fire,
		0,
		0,
		0,
		0
	},

	{													// 1
		"Select Weapon",
		136,
		0,
		0,
		kISpElementKind_Button,
		kISpElementLabel_Btn_Select,
		0,
		0,
		0,
		0
	},

	{													// 2
		"Movement",
		146,
		0,
		0,
		kISpElementKind_DPad,
		kISpElementLabel_Pad_Move_Horiz,
		0,
		0,
		0,
		0
	},

	{													// 3
		"Bunny Radar",
		147,
		0,
		0,
		kISpElementKind_Button,
		kISpElementLabel_Btn_Look,
		0,
		0,
		0,
		0
	},

	{													// 4
		"Pause",
		137,
		0,
		0,
		kISpElementKind_Button,
		kISpElementLabel_Btn_StartPause,
		0,
		0,
		0,
		0
	},

	{													// 5
		"Toggle Music",
		129,
		0,
		0,
		kISpElementKind_Button,
		kISpElementLabel_None,
		0,
		0,
		0,
		0
	},

	{													// 6
		"Toggle Sound Effects",
		129,
		0,
		0,
		kISpElementKind_Button,
		kISpElementLabel_None,
		0,
		0,
		0,
		0
	},

	{													// 7
		"Raise Volume",
		145,
		0,
		0,
		kISpElementKind_Button,
		kISpElementLabel_None,
		0,
		0,
		0,
		0
	},

	{													// 8
		"Lower Volume",
		139,
		0,
		0,
		kISpElementKind_Button,
		kISpElementLabel_None,
		0,
		0,
		0,
		0
	},

	{													// 9
		"Quit Application",
		202,
		0,
		0,
		kISpElementKind_Button,
		kISpElementLabel_Btn_Quit,
		0,
		0,
		0,
		0
	}
};


ISpElementReference	gVirtualElements[NUM_CONTROL_NEEDS];


short	gNeedToKey[NUM_CONTROL_NEEDS] =				// table to convert need # into key equate value
{
	kKey_Attack,
	kKey_SelectWeapon,

	0,							// dpad movement

	kKey_Radar,

	kKey_Pause,

	kKey_ToggleMusic,
	kKey_ToggleEffects,
	kKey_RaiseVolume,
	kKey_LowerVolume,

	kKey_Quit
};





/************************* INIT INPUT *********************************/

void InitInput(void)
{
#if USE_ISP
OSErr				iErr;
ISpDeviceReference	dev[10];
UInt32				count = 0;

        /* SEE IF ISP EXISTS */

    if ((void *)ISpStartup == (void *)kUnresolvedCFragSymbolAddress)
		DoFatalAlert("You do not have Input Sprocket installed.  This game requires Input Sprocket to function.  To install Apple's Game Sprockets, go to www.pangeasoft.net/downloads.html");

	ISpStartup();
	gISPInitialized = true;

				/* CREATE NEW NEEDS */

	iErr = ISpElement_NewVirtualFromNeeds(NUM_CONTROL_NEEDS, gControlNeeds, gVirtualElements, 0);
	if (iErr)
	{
		DoAlert("InitInput: ISpElement_NewVirtualFromNeeds failed!");
		ShowSystemErr(iErr);
	}

	iErr = ISpInit(NUM_CONTROL_NEEDS, gControlNeeds, gVirtualElements, 'MMik','Mik6', 0, 1000, 0);
	if (iErr)
	{
		DoAlert("InitInput: ISpInit failed!");
		ShowSystemErr(iErr);
	}


			/* ACTIVATE ALL DEVICES */

	if (ISpDevices_Extract(10,&count,dev) != noErr)
		DoFatalAlert("InitInput: ISpDevices_Extract failed!");

	if (ISpDevices_Activate(count, dev) != noErr)
		DoFatalAlert("InitInput: ISpDevices_Activate failed!");

	gISpActive = true;

	TurnOffISp();

#else
	gISpActive = false;
	gReadFromInputSprockets = false;
#endif
}



/**************** READ KEYBOARD_REAL *************/
//
// This just does a simple read of the REAL keyboard (regardless of Input Sprockets)
//

void ReadKeyboard_Real(void)
{


	GetKeys(gKeyMap_Real);

			/* CALC WHICH KEYS ARE NEW THIS TIME */

	gNewKeys_Real[0] = (gOldKeys_Real[0] ^ gKeyMap_Real[0]) & gKeyMap_Real[0];
	gNewKeys_Real[1] = (gOldKeys_Real[1] ^ gKeyMap_Real[1]) & gKeyMap_Real[1];
	gNewKeys_Real[2] = (gOldKeys_Real[2] ^ gKeyMap_Real[2]) & gKeyMap_Real[2];
	gNewKeys_Real[3] = (gOldKeys_Real[3] ^ gKeyMap_Real[3]) & gKeyMap_Real[3];


			/* REMEMBER AS OLD MAP */

	gOldKeys_Real[0] = gKeyMap_Real[0];
	gOldKeys_Real[1] = gKeyMap_Real[1];
	gOldKeys_Real[2] = gKeyMap_Real[2];
	gOldKeys_Real[3] = gKeyMap_Real[3];
}


/****************** GET KEY STATE: REAL ***********/
//
// for data from ReadKeyboard_Real
//

Boolean GetKeyState_Real(unsigned short key)
{
unsigned char *keyMap;

	keyMap = (unsigned char *)&gKeyMap_Real;
	return ( ( keyMap[key>>3] >> (key & 7) ) & 1);
}

/****************** GET NEW KEY STATE: REAL ***********/
//
// for data from ReadKeyboard_Real
//

Boolean GetNewKeyState_Real(unsigned short key)
{
unsigned char *keyMap;

	keyMap = (unsigned char *)&gNewKeys_Real;
	return ( ( keyMap[key>>3] >> (key & 7) ) & 1);
}




/****************** GET NEW KEY STATE ***********/
//
// NOTE: Assumes that ReadKeyboard has already been called!!
//

Boolean GetNewKeyState(unsigned short key)
{
unsigned char *keyMap;

	keyMap = (unsigned char *)&gNewKeys;
	return ( ( keyMap[key>>3] >> (key & 7) ) & 1);
}

/********************** MY GET KEYS ******************************/
//
// Depending on mode, will either read key map from GetKeys or
// will "fake" a keymap using Input Sprockets.
//

void MyGetKeys(KeyMap *keyMap)
{
short	i,key,j,q;
UInt32	keyState;
unsigned char *keyBytes;

	ReadKeyboard_Real();												// always read real keyboard anyway

	if (!gReadFromInputSprockets)
	{
		GetKeys(*keyMap);
	}
	else
	{
#if USE_ISP

		keyBytes = (unsigned char *)keyMap;
		(*keyMap)[0] = (*keyMap)[1] = (*keyMap)[2] = (*keyMap)[3] = 0;		// clear out keymap

			/***********************************/
			/* POLL NEEDS FROM INPUT SPROCKETS */
			/***********************************/

		for (i = 0; i < NUM_CONTROL_NEEDS; i++)
		{
			switch(gControlNeeds[i].theKind)
			{
					/* SIMPLE KEY BUTTON */

				case	kISpElementKind_Button:
						ISpElement_GetSimpleState(gVirtualElements[i],&keyState);		// get state of this one
						if (keyState == kISpButtonDown)
						{
							key = gNeedToKey[i];										// get keymap value for this "need"
							j = key>>3;
							q = (1<<(key&7));
							keyBytes[j] |= q;											// set correct bit in keymap
						}
						break;

					/* DIRECTIONAL PAD */

				case	kISpElementKind_DPad:
						ISpElement_GetSimpleState(gVirtualElements[i],&keyState);		// get state of this need

						gPlayerUsingKeyControl = true;									// assume player using key control

						switch(keyState)
						{
							case	kISpPadLeft:
									j = kKey_Left>>3;
									q = (1<<(kKey_Left&7));
									keyBytes[j] |= q;
									break;

							case	kISpPadUpLeft:
									j = kKey_Left>>3;
									q = (1<<(kKey_Left&7));
									keyBytes[j] |= q;
									j = kKey_Forward>>3;
									q = (1<<(kKey_Forward&7));
									keyBytes[j] |= q;
									break;

							case	kISpPadUp:
									j = kKey_Forward>>3;
									q = (1<<(kKey_Forward&7));
									keyBytes[j] |= q;
									break;

							case	kISpPadUpRight:
									j = kKey_Right>>3;
									q = (1<<(kKey_Right&7));
									keyBytes[j] |= q;
									j = kKey_Forward>>3;
									q = (1<<(kKey_Forward&7));
									keyBytes[j] |= q;
									break;

							case	kISpPadRight:
									j = kKey_Right>>3;
									q = (1<<(kKey_Right&7));
									keyBytes[j] |= q;
									break;

							case	kISpPadDownRight:
									j = kKey_Right>>3;
									q = (1<<(kKey_Right&7));
									keyBytes[j] |= q;
									j = kKey_Backward>>3;
									q = (1<<(kKey_Backward&7));
									keyBytes[j] |= q;
									break;

							case	kISpPadDown:
									j = kKey_Backward>>3;
									q = (1<<(kKey_Backward&7));
									keyBytes[j] |= q;
									break;

							case	kISpPadDownLeft:
									j = kKey_Left>>3;
									q = (1<<(kKey_Left&7));
									keyBytes[j] |= q;
									j = kKey_Backward>>3;
									q = (1<<(kKey_Backward&7));
									keyBytes[j] |= q;
									break;

							default:
									gPlayerUsingKeyControl = false;
						}
						break;
					}
		}
#endif
	}
}




/***************** GET MOUSE DELTA *****************/

void GetMouseDelta(float *dx, float *dy)
{
#if USE_ISP
ISpDeltaData	deltaX,deltaY;
OSStatus		err;

	if (!gISpActive)				// make sure DSp active
	{
		*dy = *dx = 0;
		return;
	}

		/* SEE IF OVERRIDE MOUSE WITH DPAD MOVEMENT */

	if (gPlayerUsingKeyControl)
	{
		if (GetKeyState(kKey_Left))
			*dx = -75;
		else
		if (GetKeyState(kKey_Right))
			*dx = 75;
		else
			*dx = 0;

		if (GetKeyState(kKey_Forward))
			*dy = -75;
		else
		if (GetKeyState(kKey_Backward))
			*dy = 75;
		else
			*dy = 0;

		return;
	}

				/******************************/
				/* READ MOUSE DELTAS FROM ISP */
				/******************************/

					/* DX */

	err = ISpElement_GetComplexState(gVirtualElements[NEED_NUM_MOUSEMOTION],
									sizeof(ISpDeltaData),
									&deltaX);

	if (err)
	{
		DoAlert("GetMouseDelta: ISpElement_GetComplexState failed!");
		ShowSystemErr(err);
	}

					/* DY */

	err = ISpElement_GetComplexState(gVirtualElements[NEED_NUM_MOUSEMOTION+1],
									sizeof(ISpDeltaData),
									&deltaY);

	if (err)
	{
		DoAlert("GetMouseDelta: ISpElement_GetComplexState failed!");
		ShowSystemErr(err);
	}

	*dx = (float)deltaX * (1.0f/120.0f); 			// convert to a number that works for us
	*dy = -(float)deltaY * (1.0f/120.0f);
#else
		*dy = *dx = 0;
#endif
}


#pragma mark -

/******************** TURN ON ISP *********************/

void TurnOnISp(void)
{
#if USE_ISP
ISpDeviceReference	dev[10];
UInt32		count = 0;
OSErr		iErr;

	if (!gISpActive)
	{
		gReadFromInputSprockets = true;								// player control uses input sprockets
		ISpResume();
		gISpActive = true;

				/* ACTIVATE ALL DEVICES */

		iErr = ISpDevices_Extract(10,&count,dev);
		if (iErr)
			DoFatalAlert("TurnOnISp: ISpDevices_Extract failed!");
		iErr = ISpDevices_Activate(count, dev);
		if (iErr)
			DoFatalAlert("TurnOnISp: ISpDevices_Activate failed!");

			/* DEACTIVATE JUST THE MOUSE SINCE WE DONT NEED THAT */

//		ISpDevices_ExtractByClass(kISpDeviceClass_Mouse,10,&count,dev);
//		ISpDevices_Deactivate(count, dev);
	}
#endif
}

/******************** TURN OFF ISP *********************/

void TurnOffISp(void)
{
#if USE_ISP
ISpDeviceReference	dev[10];
UInt32		count = 0;

	if (gISpActive)
	{
				/* DEACTIVATE ALL DEVICES */

		ISpDevices_Extract(10,&count,dev);
		ISpDevices_Deactivate(count, dev);
		ISpSuspend();

		gISpActive = false;
		gReadFromInputSprockets = false;
	}
#endif
}



/************************ DO KEY CONFIG DIALOG ***************************/

void DoKeyConfigDialog(void)
{
	FlushEvents (everyEvent, REMOVE_ALL_EVENTS);

				/* DO ISP CONFIG DIALOG */

	InitCursor;
	TurnOnISp();
	ISpConfigure(nil);
	TurnOffISp();
	HideCursor();
}






