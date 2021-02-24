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


extern	long		gScore;
extern	Byte	gStartingScene;
extern	Boolean	gPPCFullScreenFlag;

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

static KeyMap gKeyMap,gOldKeyMap;

struct KeyRec
{
	short		keyNum;						// mapped keyboard key #
	char		ascii;						// ascii char for that key
};
typedef struct KeyRec KeyRec;

static KeyRec	KeyList[NUM_KEYS] =
		{
			KEY_A,'A',
			KEY_B,'B',
			KEY_C,'C',
			KEY_D,'D',
			KEY_E,'E',
			KEY_F,'F',
			KEY_G,'G',
			KEY_H,'H',
			KEY_I,'I',
			KEY_J,'J',
			KEY_K,'K',
			KEY_L,'L',
			KEY_M,'M',
			KEY_N,'N',
			KEY_O,'O',
			KEY_P,'P',
			KEY_Q,'Q',
			KEY_R,'R',
			KEY_S,'S',
			KEY_T,'T',
			KEY_U,'U',
			KEY_V,'V',
			KEY_W,'W',
			KEY_X,'X',
			KEY_Y,'Y',
			KEY_Z,'Z',
			KEY_0,'0',
			KEY_1,'1',
			KEY_2,'2',
			KEY_3,'3',
			KEY_4,'4',
			KEY_5,'5',
			KEY_6,'6',
			KEY_7,'7',
			KEY_8,'8',
			KEY_9,'9',
			KEY_PERIOD,'.',
			KEY_Z,'Z',
			KEY_SPACE,' ',
			KEY_QMARK,'?',
			KEY_LEFT,CHAR_LEFT,
			KEY_DELETE,CHAR_DELETE,
			KEY_RETURN,CHAR_RETURN
		};


static Boolean		gUpButtonDownFlag,gDownButtonDownFlag;
Boolean		gSpaceButtonDownFlag;
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
	gAbortDemoFlag = false;
	gGameIsDemoFlag = false;

	gDemoDataHandle = AllocHandle(MAX_DEMO_SIZE);			// alloc memory for it
	if (gDemoDataHandle == nil)
		DoFatalAlert("Cant allocate memory for record buffer.");
	HLockHi(gDemoDataHandle);
	gDemoDataPtr = gPrevDemoDataPtr = *gDemoDataHandle;
	gDemoSize = 0L;											// init size to 0

	gOldKeyMap[0] = 0xffffffffL;							// init old keymap
	gOldKeyMap[1] = 0xffffffffL;
	gOldKeyMap[2] = 0xffffffffL;
	gOldKeyMap[3] = 0xffffffffL;

	gDemoMode = DEMO_MODE_RECORD;
	SetMyRandomSeed(DEMO_SEED);								// always use same seed!

	gDemoKeyIterations = 0;
}

/***************** SAVE DEMO DATA *************************/
//
// Save keyboard data for demo playback later
//

void SaveDemoData(void)
{
Handle		resHandle;

	if (gDemoMode != DEMO_MODE_RECORD)					// be sure we were recording
		return;

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
}


/********************* INIT DEMO PLAYBACK ****************/

void InitDemoPlayback(void)
{
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
}


/**************** READ KEYBOARD *************/

void ReadKeyboard(void)
{
short	*intPtr;
KeyMap tempKeys;

					/* DEMO PLAYBACK */

	if (gDemoMode == DEMO_MODE_PLAYBACK)				// see if read from demo file
	{
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
	}
	else
	{
		MyGetKeys(&gKeyMap);										// READ THE REAL KEYBOARD
//		GetKeys(gKeyMap);								// READ THE REAL KEYBOARD
    }

					/* RECORD DEMO */

	if (gDemoMode == DEMO_MODE_RECORD)					// see if record keyboard
	{

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
	}

				/* SEE IF QUIT GAME */

	if (GetKeyState_Real(KEY_Q) && GetKeyState_Real(KEY_APPLE))			// see if key quit
		CleanQuit();
}


/******************* STOP DEMO ******************/

void StopDemo(void)
{
	DisposeHandle(gDemoDataHandle);
	gDemoMode = DEMO_MODE_OFF;			// set back to OFF
	gAbortDemoFlag = true;
	ReadKeyboard();						// read keyboard to reset it all
}


/****************** GET KEY STATE ***********/
// NOTE: KEYS ARE NOT ASCII VALUES!!!
//

Boolean GetKeyState(unsigned short key)
{
unsigned char *keyMap;

	keyMap = (unsigned char *)&gKeyMap;
	return ( ( keyMap[key>>3] >> (key & 7) ) & 1);
}


/****************** GET KEY STATE 2 ***********/
// NOTE: KEYS ARE NOT ASCII VALUES!!!
// v2 Automatically calls ReadKeyboard

Boolean GetKeyState2(unsigned short key)
{
unsigned char *keyMap;

	ReadKeyboard();

	keyMap = (unsigned char *)&gKeyMap;
	return ( ( keyMap[key>>3] >> (key & 7) ) & 1);
}


/********************** CHECK KEY *********************/

char CheckKey(void)
{
static		oldKey = 0;
short		i;
char		theChar;

again:
	ReadKeyboard();
	for (i=0; i<NUM_KEYS; i++)
	{

		if (GetKeyState(KeyList[i].keyNum))
		{
			theChar = KeyList[i].ascii;
			goto	gotit;
		}

	}
	oldKey = 0;											// no key found
	return('\xBD');

					/* GOT SOMETHING */
gotit:
	if ((theChar == '1') && GetKeyState(KEY_SHIFT))		// check for !
		theChar = '!';
	else
	if ((theChar == '8') && GetKeyState(KEY_SHIFT))		// check for *
		theChar = '*';

	if (theChar == oldKey)								// see if still down
		goto	again;

	oldKey = theChar;


	return(theChar);
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

/*************** PRINT NUMBER 2 ****************/
//
// This version prints to offscreen buffer
//
// INPUT: htab = coord or leftmost digit
//

void PrintNum2(long	num, short numDigits, short htab, short vtab)
{
register short	i,digit;

	for (i=0; i<numDigits; i++)
	{
		digit = num-(num/10*10);
		DrawOnBackground_NoMask(htab,vtab,GroupNum_ScoreNumbers,ObjType_ScoreNumbers,digit,true);
		htab -= 14;
		num = num/10;
	}
}


/*************** PRINT NUMBER AT ****************/
//
// This version prints to offscreen buffer
//
// INPUT: htab = coord or leftmost digit
//

void PrintNumAt(long	num, short numDigits, short htab, short vtab, Ptr where, long wid)
{
register short	i,digit;

	for (i=0; i<numDigits; i++)
	{
		digit = num-(num/10*10);
		DrawFrameAt_NoMask(htab,vtab,GroupNum_ScoreNumbers,ObjType_ScoreNumbers,
							digit,where,wid);
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


/****************** WAIT KEY UP *******************/

void WaitKeyUp(Byte key)
{
	while(GetKeyState2(key));
}


/*************** CHECK NEW KEY DOWN ****************/
//
// checks if key is down and flagPtr points to KeyAlreadyDown Flag
//

Boolean CheckNewKeyDown(Byte key1, Byte key2, Boolean *flagPtr)
{
	if (GetKeyState2(key1) || (GetKeyState2(key2)))
	{
		if (*flagPtr)
			return(false);
		else
		{
			*flagPtr = true;
			return(true);
		}
	}
	else
	{
		*flagPtr = false;
		return(false);
	}
}

/*************** CHECK NEW KEY DOWN 2 ****************/
//
// only 1 key
//

Boolean CheckNewKeyDown2(Byte key1, Boolean *flagPtr)
{
	if (GetKeyState2(key1))
	{
		if (*flagPtr)
			return(false);
		else
		{
			*flagPtr = true;
			return(true);
		}
	}
	else
	{
		*flagPtr = false;
		return(false);
	}
}


/******************* ASCII TO BIG FONT **********************/
//
// Converts ASCII char to BigFont subtype #
//

Byte ASCIIToBigFont(char c)
{
	if ((c >= '0') && (c <= '9'))				// see if #
		return(c-'0'+68);

	if ((c >= 'A') && (c <= 'Z'))				// see if A..Z
		return(c-'A'+11);

	if (c == '!')
		return(80);

	if (c == '?')
		return(83);

	if (c=='.')
		return(38);

	if (c==':')
		return(79);

	if (c=='@')
		return(78);

	if (c==',')
		return(82);

	if ((c >= 'a') && (c <= 'z'))				// see if a..z
		return(c-'a'+42);

	return(0);

}





