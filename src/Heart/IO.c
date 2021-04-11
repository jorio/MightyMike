/****************************/
/*        IO                */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/
#include "myglobals.h"
#include "object.h"
#include "objecttypes.h"
#include "shape.h"
#include "io.h"
#include "misc.h"
#include "infobar.h"
#include "input.h"
#include "externs.h"

/****************************/
/*    CONSTANTS             */
/****************************/


#define	NUM_KEYS		43


#define	MAX_DEMO_SIZE		80000L
#define	END_DEMO_MARK		0x7fff
#define	DEMO_SEED			1234L


/**********************/
/*     VARIABLES      */
/**********************/

short	gDemoMode = DEMO_MODE_OFF;
Handle	gDemoDataHandle;
Ptr		gDemoDataPtr,gPrevDemoDataPtr;
long	gDemoSize;
short	gDemoKeyIterations;

short	gHtab=0,gVtab=0;

//static KeyMap gKeyMap,gOldKeyMap;

Boolean		gAbortDemoFlag,gGameIsDemoFlag;


/*************** START RECORDING DEMO *****************/
//
// Begins to record key commands for playback later
//
// The demo data is in the following format:
//     short	# iterations of following KeyMap
//	   KeyMap	the KeyMap in question
//

void StartRecordingDemo(void)
{
	DoFatalAlert("TODO: Implement StartRecordingDemo");
#if 0
	gAbortDemoFlag = false;
	gGameIsDemoFlag = false;

	gDemoDataHandle = AllocHandle(MAX_DEMO_SIZE);			// alloc memory for it
	if (gDemoDataHandle == nil)
		DoFatalAlert("Cant allocate memory for record buffer.");
	HLockHi(gDemoDataHandle);
	gDemoDataPtr = gPrevDemoDataPtr = *gDemoDataHandle;
	gDemoSize = 0L;											// init size to 0

	DoFatalAlert("TODO: Reimplement StartRecordingDemo");
#if 0
	gOldKeyMap[0] = 0xffffffffL;							// init old keymap
	gOldKeyMap[1] = 0xffffffffL;
	gOldKeyMap[2] = 0xffffffffL;
	gOldKeyMap[3] = 0xffffffffL;
#endif

	gDemoMode = DEMO_MODE_RECORD;
	SetMyRandomSeed(DEMO_SEED);								// always use same seed!

	gDemoKeyIterations = 0;
#endif
}

/***************** SAVE DEMO DATA *************************/
//
// Save keyboard data for demo playback later
//

void SaveDemoData(void)
{
	if (gDemoMode != DEMO_MODE_RECORD)					// be sure we were recording
		return;

	DoFatalAlert("TODO: Implement SaveDemoData");
#if 0
Handle		resHandle;

	*(short *)gDemoDataPtr = END_DEMO_MARK;						// put end mark @ end of file
	gDemoSize += sizeof(short);

					/* SAVE AS RESOURCE */

#ifdef __powerc
	resHandle = GetResource('dEmo',1000+gStartingScene+(1000*gPPCFullScreenFlag));		// read the resource
#else
	resHandle = GetResource('dEmo',4000+gStartingScene);
#endif
	RemoveResource(resHandle);									// nuke it
	ReleaseResource(resHandle);

	PtrToHand(*gDemoDataHandle, &resHandle, gDemoSize);			// make a handle with data
#ifdef __powerc
	AddResource( resHandle, 'dEmo', 1000+gStartingScene+(1000*gPPCFullScreenFlag), "Demo Data" );		// add resource
#else
	AddResource( resHandle, 'dEmo', 4000+gStartingScene, "Demo Data" );		// add resource
#endif
	if ( ResError() )
		DoFatalAlert("Couldnt Add Demo Resource!");
	WriteResource( resHandle );									// update it
	ReleaseResource(resHandle);									// nuke resource

	DisposeHandle(gDemoDataHandle);								// delete demo data
	gDemoDataHandle = nil;
	gDemoMode = DEMO_MODE_OFF;
#endif
}


/********************* INIT DEMO PLAYBACK ****************/

void InitDemoPlayback(void)
{
	DoFatalAlert("TODO: Implement InitDemoPlayback");
#if 0
	gStartingScene = RandomRange(0,2);						// get random demo (dont do 3 & 4)

	gAbortDemoFlag = false;
	gGameIsDemoFlag = true;
	gDemoMode = DEMO_MODE_PLAYBACK;
	SetMyRandomSeed(DEMO_SEED);								// always use same seed!

				/* LOAD DEMO DATA */

#ifdef __powerc
	gDemoDataHandle = GetResource('dEmo',1000+gStartingScene+(1000*gPPCFullScreenFlag));	// read the resource
#else
	gDemoDataHandle = GetResource('dEmo',4000+gStartingScene);	// read the resource
#endif

	if (gDemoDataHandle == nil)
		DoFatalAlert("Error reading Demo Resource!");

	DetachResource(gDemoDataHandle);						// detach resource
	HLockHi(gDemoDataHandle);
	gDemoDataPtr = *gDemoDataHandle;

	gDemoKeyIterations = 0;									// no iterations present
#endif
}


/**************** READ KEYBOARD *************/

void ReadKeyboard(void)
{
#if 0
short	*intPtr;
KeyMap tempKeys;
#endif

					/* DEMO PLAYBACK */

	if (gDemoMode == DEMO_MODE_PLAYBACK)				// see if read from demo file
	{
		DoFatalAlert("TODO: Reimplement demo playback");
#if 0
		if (gDemoKeyIterations == 0)					// see if need to get next keymap
		{
			intPtr = (short *)gDemoDataPtr;				// get new iteration count
			gDemoKeyIterations = *intPtr++;
			if (gDemoKeyIterations == END_DEMO_MARK)	// see if end of file
			{
				StopDemo();
				return;
			}

			BlockMove((Ptr)intPtr,&gKeyMap,sizeof(KeyMap));
			gDemoDataPtr += sizeof(KeyMap) + sizeof(short);
		}
		else
			gDemoKeyIterations--;						// decrement iteration counter

		MyGetKeys(&tempKeys);												// read real keyboard only to check for abort
//		GetKeys(tempKeys);								// read real keyboard only to check for abort
		if (tempKeys[0] || tempKeys[1] || tempKeys[2] || tempKeys[3])		// any key aborts
		{
			StopDemo();
		}
#endif
	}
	else
	{
//		MyGetKeys(&gKeyMap);										// READ THE REAL KEYBOARD
//		GetKeys(gKeyMap);								// READ THE REAL KEYBOARD
		UpdateInput();
	}

					/* RECORD DEMO */

	if (gDemoMode == DEMO_MODE_RECORD)					// see if record keyboard
	{
		DoFatalAlert("TODO: Reimplement demo recording");
#if 0
		if ((gKeyMap[0] == gOldKeyMap[0]) &&					// see if same as last occurrence
			(gKeyMap[1] == gOldKeyMap[1]) &&
			(gKeyMap[2] == gOldKeyMap[2]) &&
			(gKeyMap[3] == gOldKeyMap[3]))
		{
			intPtr = (short *)gPrevDemoDataPtr;					// increment iteration count
			*intPtr = *intPtr+1;
		}
		else													// new KeyMap, so reset and add
		{
			gPrevDemoDataPtr = gDemoDataPtr;
			intPtr = (short *)gDemoDataPtr;						// init iteration count
			*intPtr++ = 0;
			BlockMove(&gKeyMap,(Ptr)intPtr,sizeof(KeyMap));		// copy keymap to file
			BlockMove(&gKeyMap,&gOldKeyMap,sizeof(KeyMap));		// copy keymap to oldkeymap
			gDemoDataPtr += sizeof(KeyMap) + sizeof(short);
			gDemoSize += sizeof(KeyMap) + sizeof(short);

			gScore = MAX_DEMO_SIZE-gDemoSize;					// show remaining memory in score
			ShowScore();
		}

		if (gDemoSize >= (MAX_DEMO_SIZE - sizeof(KeyMap)))		// see if overflowed
			DoFatalAlert("Demo Record Buffer Overflow!");
#endif
	}
}


/******************* STOP DEMO ******************/

void StopDemo(void)
{
	DisposeHandle(gDemoDataHandle);
	gDemoMode = DEMO_MODE_OFF;			// set back to OFF
	gAbortDemoFlag = true;
	ReadKeyboard();						// read keyboard to reset it all
}

/*************** PRINT NUMBER ****************/
//
// This version prints directly to the screen
//
// INPUT: htab = coord or leftmost digit
//

void PrintNum(long	num, short numDigits, short htab, short vtab)
{
register short	i,digit;

	for (i=0; i<numDigits; i++)
	{
		digit = num-(num/10*10);
		DrawFrameToScreen_NoMask(htab,vtab,GroupNum_ScoreNumbers,ObjType_ScoreNumbers,digit);
		htab -= 14;
		num = num/10;
	}
}


/****************** PRINT BIG NUM **********************/

void PrintBigNum(long num, short numDigits)
{
short			i;
short		digit;
long		divis[] = {1,10,100,1000,10000,100000,1000000,10000000,100000000};
long		workNum;

	workNum = num;

	if (numDigits > 9)
		DoFatalAlert("Attempted to print a number longer than 9 digits!");

	for (i=numDigits-1; i>=0; i--)
	{
		digit = workNum/divis[i];
		PrintBigChar('0'+digit);

		workNum -= divis[i]*digit;

	}
}


/****************** WRITELN *******************/

void WriteLn(char *cString)
{
	while (*cString)
	{
		PrintBigChar(*cString);
		cString++;
	}

	DumpUpdateRegions();
}


/******************* PRINT BIG CHAR *****************/

void PrintBigChar(char ch)
{
	if ((ch >= '0') && (ch <= '9'))							// see if number
		DrawFrameToScreen(gHtab,gVtab,GroupNum_BigFont,ObjType_BigFont,ch-'0');
	else
	if ((ch >= 'A') && (ch <= 'Z'))							// see if letter
		DrawFrameToScreen(gHtab,gVtab,GroupNum_BigFont,ObjType_BigFont,ch-'A'+10);
	else
	if (ch == '*')											// see if *
		DrawFrameToScreen(gHtab,gVtab,GroupNum_BigFont,ObjType_BigFont,36);
	else
	if (ch == '.')											// see if .
		DrawFrameToScreen(gHtab,gVtab,GroupNum_BigFont,ObjType_BigFont,37);
	else
	if (ch == '!')											// see if !
		DrawFrameToScreen(gHtab,gVtab,GroupNum_BigFont,ObjType_BigFont,38);
	else
	if (ch == '?')											// see if ?
		DrawFrameToScreen(gHtab,gVtab,GroupNum_BigFont,ObjType_BigFont,39);


	gHtab += FONT_WIDTH;

}

