/****************************/
/*      MISC ROUTINES       */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/

/***************/
/* EXTERNALS   */
/***************/

#include "myglobals.h"
#include <folders.h>
#include <pictutils.h>
#include <palettes.h>
#include <osutils.h>
#include <timer.h>
#include <inputsprocket.h>
#include <textutils.h>
#include "windows.h"
#include "io.h"
#include "object.h"
#include "bitio.h"
#include "arith-n.h"
#include "lzw.h"
#include "misc.h"
#include "sound.h"
#include "sound2.h"
#include "picture.h"
#include "input.h"
#include "objecttypes.h"
#include "cinema.h"

extern	WindowPtr		gGameWindow;
extern	PaletteHandle	gGamePalette;
extern	short				gColorListSize;
extern	Handle			gShapeTableHandle[];
extern	long			gScreenXOffset,gScreenYOffset;
extern	long			gFrames,someLong;
extern	Boolean			gMusicOnFlag,gAbortDemoFlag,gISPInitialized;

/****************************/
/*    PROTOTYPES             */
/****************************/

static void CheckGameRegistration(void);
static Boolean ValidateRegistrationNumber(unsigned char *regInfo);
static void DoRegistrationDialog(unsigned char *out);

/****************************/
/*    CONSTANTS             */
/****************************/

#define		MOUSE_RESET_X	300
#define		MOUSE_RESET_Y	200

#define		ERROR_ALERT_ID		401
#define		ERROR_ALERT2_ID		402
#define		COPY2HD_ALERT_ID	403

#define		MAX_PACK_SEG_SIZE	0x7fff

enum {
	MTemp = 0x828,
    RawMouse = 0x82C,               /*[GLOBAL VAR]  un-jerked mouse coordinates [long]*/
	Mouse	=	0x830,
    CrsrNew = 0x08CE 				// char - set != 0 if mouse has moved
};

#define	DECOMP_PACKET_SIZE	20000L

									// FILE COMPRESSION TYPES
									//=======================

enum
{
	PACK_TYPE_RLB	=	0,					// Run-Length-Byte compression
	PACK_TYPE_LZSS	=	1,					// LZSS compression
	PACK_TYPE_NONE = 	2,					// no compression
	PACK_TYPE_ARTN = 	3,					// ARITH-N compression
	PACK_TYPE_HUFF = 	4,					// HUFF compression
	PACK_TYPE_LZW = 	5,					// LZW15 compression
	PACK_TYPE_RLW = 	6					// Run-Length-Word compression
};


        /* REGISTRATION */


#define REG_LENGTH      12


/**********************/
/*     VARIABLES      */
/**********************/

short	gPrefsFolderVRefNum;
long	gPrefsFolderDirID;

long	gTick;							// used for regulating speed
static	short	gOldMouseX,gOldMouseY;			// for mouse delta calculation

									// MENU BAR HIDE/SHOW STUFF
static	short	oldMBarHeight;
static	RgnHandle	mBarRgn;

static	short			gNumScreenResets = 0;


						/* GLOBAL FLAGS */

Boolean		gGlobalFlagList[MAX_GLOBAL_FLAGS];

short		gThermometerX,gThermometerY;

static	unsigned long seed0 = 0, seed1 = 0, seed2 = 0;

long		gOriginalSystemVolume = 200;
long		gOriginalSystemVolumeFudge;

static	Str255		gBasePathName = "\pMightyMike";
static	Str255		gMemoryErr = "\pTry increasing the applications memory size by selecting Get Info from the finder.";

Byte		gRLBDecompBuffer[DECOMP_PACKET_SIZE];

long	gCPUType;

Boolean     gGameIsRegistered = false;

unsigned long       gDemoVersionTimer = 0;

/**************** CLEAR GLOBAL FLAGS ****************/

void ClearGlobalFlags(void)
{
short		i;

	for (i=0; i <MAX_GLOBAL_FLAGS; i++)
		gGlobalFlagList[i] = false;

}

/****************** DO SYSTEM ERROR ***************/

void ShowSystemErr(OSErr err)
{
Str255		numStr;

	RestoreDefaultCLUT();
	ShowCursor();
	NumToString((long)err, numStr);
	DoAlert (numStr);
	MyShowMenuBar();
	RestoreDefaultCLUT();
	CleanQuit();
}


/*********************** DO ALERT *******************/

void DoAlert(Str255 s)
{
	RestoreDefaultCLUT();
	ShowCursor();
	ParamText(s,NIL_STRING,NIL_STRING,NIL_STRING);
	Alert(ERROR_ALERT_ID,nil);
}


/*********************** DO FATAL ALERT *******************/

void DoFatalAlert(Str255 s)
{
	RestoreDefaultCLUT();
	InitCursor();
//	if (gGameWindow != nil)
//		DisposeWindow(gGameWindow);
	ParamText(s,NIL_STRING,NIL_STRING,NIL_STRING);
	Alert(ERROR_ALERT_ID,nil);
	MyShowMenuBar();
	CleanQuit();
}


/*********************** DO FATAL ALERT 2 *******************/

void DoFatalAlert2(Str255 s1, Str255 s2)
{
	RestoreDefaultCLUT();
	InitCursor();
//	if (gGameWindow != nil)
//   	DisposeWindow(gGameWindow);
	ParamText(s1,s2,NIL_STRING,NIL_STRING);
	Alert(ERROR_ALERT2_ID,nil);
	MyShowMenuBar();
	CleanQuit();
}


/*********************** CLEAN QUIT *******************/

void CleanQuit(void)
{
static Boolean beenHereFlag = false;

//	DisposeHandle(gShapeTableHandle[0]);			// zap this if it exists

	if (beenHereFlag)								// see if already been called
		goto	exit;

	CleanupDisplay();								// unloads Draw Sprocket

    SaveDemoTimer();

exit:

    if (gISPInitialized)
   		ISpShutdown();

//	RestoreDefaultCLUT();

	ShowCursor();
//	MyShowMenuBar();
	SetDefaultOutputVolume(gOriginalSystemVolume);	// reset to entry volume
	FlushEvents (everyEvent, REMOVE_ALL_EVENTS);	// flush before exiting
	ExitToShell();
}



/************************ HIDE/SHOW MENU BAR **************************/

void MyHideMenuBar(void)
{
#if 0
	Rect	mBarRect;

	oldMBarHeight = GetMBarHeight();
	LMSetMBarHeight(0);			/* make the Menu Bar's height zero */
	SetRect(&mBarRect, qd.screenBits.bounds.left, qd.screenBits.bounds.top,
			qd.screenBits.bounds.right, qd.screenBits.bounds.top + oldMBarHeight);
	mBarRgn = NewRgn();
	RectRgn(mBarRgn, &mBarRect);
	UnionRgn(LMGetGrayRgn(), mBarRgn, LMGetGrayRgn());/* tell the desktop it covers the menu
													 * bar
													 */
	PaintOne(nil, mBarRgn);	/* redraw desktop */
#endif
}

void MyShowMenuBar(void)
{
#if 0
	LMSetMBarHeight(oldMBarHeight);	/* make the menu bar's height normal */
	DiffRgn(LMGetGrayRgn(), mBarRgn, LMGetGrayRgn());	/* remove the menu bar from the
													 * desktop
													 */
	DisposeRgn(mBarRgn);
#endif
}


/************************* WAIT ***********************/
//
// Waits for specified # of ticks or mouse click
//
// OUTPUT: true if aborted early
//

Boolean Wait(long time)
{
long	old;

	while (time > 0)
	{
		old = TickCount();							// wait for 1 tick to pass
		while(TickCount() == old);
		--time;
		if (GetKeyState2(KEY_SPACE) || (GetKeyState2(KEY_RETURN)))	// see if keyboard break out
			return(true);
		DoSoundMaintenance(true);						// (must be after readkeyboard)
	}
	return(false);
}


/********************* WAIT 2 ********************/
//
// Waits while running the task loop
//

void Wait2(long time)
{
	for ( ;time > 0; time--)
	{
		RegulateSpeed2(1);
		EraseObjects();
		MoveObjects();
		DrawObjects();
		DumpUpdateRegions();
		ReadKeyboard();
//		if (GetKeyState(KEY_Q))						// see if key quit
//			CleanQuit();

		if (GetKeyState2(KEY_SPACE) || GetKeyState2(KEY_RETURN))	// exit if key down
			break;

		DoSoundMaintenance(true);						// (must be after readkeyboard)
	}
}

/********************* WAIT 3 ********************/
//
// Waits while running the task loop & cannot be stopped early.
//

void Wait3(long time)
{
	for ( ;time > 0; time--)
	{
		RegulateSpeed2(1);
		EraseObjects();
		MoveObjects();
		DrawObjects();
		DumpUpdateRegions();
		ReadKeyboard();
//		if (GetKeyState(KEY_Q))						// see if key quit
//			CleanQuit();

		DoSoundMaintenance(true);						// (must be after readkeyboard)
	}
}

/************************* WAIT 4 ***********************/
//
// Waits for specified # of ticks NO mouse click
//

void Wait4(long time)
{
long	start;

	start = TickCount();
	while (TickCount() < (start+time));
}


/********************* WAIT WHILE MUSIC ********************/
//
// Waits while running the task loop until music stops or user abort
//

void WaitWhileMusic(void)
{
	if (!gMusicOnFlag)								// if user aborted music, then just wait .. seconds
		Wait2(60*5);
	else
	{
		do
		{
			RegulateSpeed2(1);
			EraseObjects();
			MoveObjects();
			DrawObjects();
			DumpUpdateRegions();
			ReadKeyboard();

//			if (GetKeyState(KEY_CAPSLOCK))				// see if pause
//				while (GetKeyState2(KEY_CAPSLOCK));

			if (GetKeyState2(KEY_SPACE) ||(GetKeyState2(KEY_RETURN)))	// exit if key down
				break;

			DoSoundMaintenance(true);						// (must be after readkeyboard)
		} while(IsMusicPlaying() && (!gAbortDemoFlag));
	}
}


/******************** CHECK SCREEN DEPTH **************/

void CheckScreenDepth(void)
{
GDHandle 	gdh;
OSErr 		myErr;
short		depth;

	gdh = GetMainDevice();

	depth = (*(*gdh)->gdPMap)->pixelSize;			// get current pixel depth

	if (depth != 8)
	{

		if (HasDepth(gdh,8,0,0))						// see if supports 256
		{
			myErr = SetDepth(gdh,8,1,1);				// set to 256 color
			if (myErr)
				ShowSystemErr(myErr);
		}
		else
			DoFatalAlert("\pMonitor cannot be set to 256 colors.  Requires 256 colors to run.");
	}
}


/*********************** RESET SCREEN ********************/
//
// If a -154 error occurs, this seems to help sometimes
//

void ResetScreen(void)
{
GDHandle gdh;

	RestoreDefaultCLUT();
	if (gNumScreenResets++ < 2)
	{
		gdh = GetMainDevice();
//		SetDepth(gdh,1,1,0);				// set to 2 color
		SetDepth(gdh,8,1,1);				// set to 256 color
	}
	else
		DoFatalAlert("\pScreen CLUT cannot be set the way I want?!  Launch me again.");

}


/******************** LOAD PACKED FILE *****************/

Handle LoadPackedFile(Str255 fileName)
{
OSErr		iErr;
short		fRefNum,vRefNum;
long		fileSize,fileSizeCopy,decompType;
Str255		volName;
Handle		dataHand;
long	numToRead,decompSize;

	MaxMem(&someLong);									// clean up
	CompactMem(maxSize);


	iErr = GetVol(volName,&vRefNum);					// get default volume

					/*  OPEN THE FILE */

	OpenMikeFile(fileName,&fRefNum,"\pCant open Packed file!");

					/* GET SIZE OF FILE */

	if	(GetEOF(fRefNum,&fileSize) != noErr)
		DoFatalAlert("\pErr Packed file EOF!");
	fileSizeCopy = fileSize;							// remember size

					/*	READ DECOMP SIZE */

	numToRead = 4;
	iErr = FSRead(fRefNum,&numToRead,&decompSize);			// read 4 byte length
	if (iErr != noErr)
		DoFatalAlert ("\pError reading Packed data!");
	fileSize -= numToRead;

					/*	READ DECOMP TYPE */

	numToRead = 4;
	iErr = FSRead(fRefNum,&numToRead,&decompType);			// read compression type
	if (iErr != noErr)
		DoFatalAlert ("\pError reading Packed data Header!");
	fileSize -= numToRead;

					/* GET MEMORY FOR UNPACKED DATA */

	dataHand = AllocHandle(decompSize);
	if (dataHand == nil)
	{
		DoAlert (fileName);		//-----------
		DoFatalAlert2 ("\pNo Memory for Unpacked Data!",gMemoryErr);
	}
	HLockHi(dataHand);

	switch(decompType)
	{
		case 	PACK_TYPE_RLB:
				DecompressRLBFile(fRefNum,*dataHand,decompSize);
				break;

		case	PACK_TYPE_LZSS:
				if (LZSS_Decode(fRefNum,*dataHand,fileSize) != decompSize)
					DoAlert("\pDecomp Sizes Dont match!");
				break;

//		case	PACK_TYPE_ARTN:
//				ARITHN_Expand(fRefNum,*dataHand,fileSize);
//				break;

//		case	PACK_TYPE_LZW:
//				LZW_Expand(fRefNum,(unsigned char *)*dataHand,fileSize);
//				break;

		case	PACK_TYPE_RLW:
				RLW_Expand(fRefNum,(unsigned short *)*dataHand,fileSize);
				break;

		case	PACK_TYPE_NONE:
				FSRead(fRefNum,&fileSize,*dataHand);
				break;

		default:
				DoFatalAlert("\pUnsupported compression Type!");

	}


					/*  CLOSE THE FILE */

	iErr = FSClose(fRefNum);
	if (iErr != noErr)
		DoFatalAlert ("\pCant close Packed file!");

	HUnlock(dataHand);								// optimize memory
	MaxMem(&someLong);								// clean up
	CompactMem(maxSize);
	HLockHi(dataHand);

	return(dataHand);								// return handle to unpacked data
}

/****************** DECOMPRESS RLB FILE *******************/

void	DecompressRLBFile(short fRefNum, Ptr destPtr, long decompSize)
{
long	numToRead;
Byte	count,data;
Ptr		packetStart,srcPtr;

					/* GET MEMORY FOR PACK BUFFER */

	packetStart = (Ptr)&gRLBDecompBuffer[0];

//	if ((packetStart = AllocPtr(DECOMP_PACKET_SIZE)) == nil)
//		DoFatalAlert2 ("\pNo Memory for RLB UnPack Buffer!",gMemoryErr);

	srcPtr = packetStart;

					/* READ 1ST DATA PACKET */

	numToRead = DECOMP_PACKET_SIZE;							// read a packet
	FSRead(fRefNum,&numToRead,packetStart);


					/* UNPACK IT */
	do
	{
		count = *srcPtr++;									// get count byte
		numToRead--;
		if (numToRead <= 0)
		{
			numToRead = DECOMP_PACKET_SIZE;					// read a packet
			FSRead(fRefNum,&numToRead,packetStart);
			srcPtr = packetStart;
		}

		if (count > 0x7f)									// (-) means packed data
		{
			count = (-count)+1;
			data = *srcPtr++;								// get data byte
			numToRead--;
			if (numToRead <= 0)
			{
				numToRead = DECOMP_PACKET_SIZE;				// read a packet
				FSRead(fRefNum,&numToRead,packetStart);
				srcPtr = packetStart;
			}
			decompSize -= count;
			for (;count>0; count--)
				*destPtr++ = data;

		}
		else												// (+) means nonpacked data
		{
			count += 1;
			decompSize -= count;
			for (; count>0; count--)
			{
				*destPtr++ = *srcPtr++;						// get data byte
				numToRead--;
				if (numToRead <= 0)
				{
					numToRead = DECOMP_PACKET_SIZE;			// read a packet
					FSRead(fRefNum,&numToRead,packetStart);
					srcPtr = packetStart;
				}
			}
		}
	} while (decompSize > 0);

//	DisposePtr(packetStart);					// nuke packed data buffer

}

/******************** RLW EXPAND FILE *********************/
//
// Expand a Run-Length Word file
//

void RLW_Expand(short fRefNum, unsigned short *output, long sourceSize)
{
register unsigned short	*srcOriginalPtr,*sourcePtr;
register unsigned char	*lengthPtr;
register unsigned short	runCount,seed;
register long	buffSize;
long	numToRead;

				/* GET MEMORY FOR SOURCE DATA */

	srcOriginalPtr = (unsigned short *)&gRLBDecompBuffer[0];

//	srcOriginalPtr = (unsigned short *)AllocPtr(DECOMP_PACKET_SIZE);
//	if (srcOriginalPtr == nil)
//		DoFatalAlert("\pCouldnt allocate memory for RLW unpack buffer!");

				/* READ FIRST SOURCE PACKET */

	numToRead = DECOMP_PACKET_SIZE;
	FSRead(fRefNum,&numToRead,srcOriginalPtr);
	sourcePtr = srcOriginalPtr;
	buffSize = DECOMP_PACKET_SIZE;

	while (sourceSize > 0)
	{
		lengthPtr = (unsigned char *)sourcePtr;
		runCount = *lengthPtr++;							// get length byte
		sourcePtr = (unsigned short *)lengthPtr;
		sourceSize--;
		if (--buffSize == 0)								// see if need to refill buffer
		{
			numToRead = DECOMP_PACKET_SIZE;
			FSRead(fRefNum,&numToRead,srcOriginalPtr);
			sourcePtr = srcOriginalPtr;
			buffSize = DECOMP_PACKET_SIZE;
		}
		if (runCount&0x80)									// see if packed stream or not
		{
					/* DECODE PACKED STREAM */

			if (buffSize == 1)								// if only 1 byte in buffer, then adjust & reload
			{
				SetFPos(fRefNum,fsFromMark,-1);
				numToRead = DECOMP_PACKET_SIZE;
				FSRead(fRefNum,&numToRead,srcOriginalPtr);
				sourcePtr = srcOriginalPtr;
				buffSize = DECOMP_PACKET_SIZE;
			}
			seed = *sourcePtr++;							// get the packed seed
			sourceSize -= 2;
			if ((buffSize -= 2) == 0)						// see if need to refill buffer
			{
				numToRead = DECOMP_PACKET_SIZE;
				FSRead(fRefNum,&numToRead,srcOriginalPtr);
				sourcePtr = srcOriginalPtr;
				buffSize = DECOMP_PACKET_SIZE;
			}

			runCount = (runCount&0x7f)+1;					// get counter
			for (; runCount; runCount--)
			{
				*output++ = seed;
			}
		}
		else
		{
					/* DECODE UNPACKED STREAM */

			runCount++;
			for (; runCount; runCount--)
			{
				if (buffSize == 1)								// if only 1 byte in buffer, then adjust & reload
				{
					SetFPos(fRefNum,fsFromMark,-1);
					numToRead = DECOMP_PACKET_SIZE;
					FSRead(fRefNum,&numToRead,srcOriginalPtr);
					sourcePtr = srcOriginalPtr;
					buffSize = DECOMP_PACKET_SIZE;
				}
				*output++ = *sourcePtr++;
				sourceSize-=2;
				if ((buffSize -= 2) == 0)						// see if need to refill buffer
				{
					numToRead = DECOMP_PACKET_SIZE;
					FSRead(fRefNum,&numToRead,srcOriginalPtr);
					sourcePtr = srcOriginalPtr;
					buffSize = DECOMP_PACKET_SIZE;
				}
			}
		}
	}

//	DisposPtr((Ptr)srcOriginalPtr);
}


/******************** REGULATE SPEED ***************/
//
// INPUT: speed = # microseconds to wait
//


void RegulateSpeed(long speed)
{
UnsignedWide	tick;
static	UnsignedWide oldTick = {0,0};

	do
	{
		Microseconds(&tick);					// get current micro secs

		if (tick.hi != oldTick.hi)              // if hi value is diff then just skip a beat and bail
		    break;

        if ((tick.lo - oldTick.lo) >= speed)    // see if enough time has passed
            break;

	} while(true);

	oldTick = tick;

	gFrames++;
}


/******************** REGULATE SPEED 2 ***************/
//
// v2 does by ticks, not microseconds
//

void RegulateSpeed2(short speed)
{
	while ((TickCount() - gTick) < speed);				// wait for 1 tick
	gTick = TickCount();							// remember current time
	gFrames++;
}




/************************* RANDOM RANGE *************************/

unsigned short	RandomRange(unsigned short min, unsigned short max)
{
register	unsigned short		qdRdm;							// treat return value as 0-65536
register	unsigned long		range, t;

	qdRdm = MyRandomShort();
	range = max+1 - min;
	t = (qdRdm * range)>>16;	 							// now 0 <= t <= range

	return( t+min );
}



/******************** DECAY *****************/

void	Decay(long	*value, unsigned long amount)
{
register long	temp;

	if	(*value == 0)				// see if already 0
		return;

	temp = *value;

	if (temp>0)						// see if +
	{
		temp -= amount;
		if (temp < 0)
			temp = 0;
	}
	else							// else -
	{
		temp += amount;
		if (temp > 0)
			temp = 0;
	}

	*value = temp;					// save value
}



/****************** ABSOLUTE *********************/

long Absolute(long num)
{
	if (num < 0)
		num = -num;

	return(num);
}

#pragma mark -

/******************* VERIFY SYSTEM ******************/

void VerifySystem(void)
{
OSErr	iErr;
long		createdDirID;


	GetDefaultOutputVolume(&gOriginalSystemVolume);				// remember entry volume
	gOriginalSystemVolumeFudge = gOriginalSystemVolume&0xff0;


			/* CHECK PREFERENCES FOLDER */

	iErr = FindFolder(kOnSystemDisk,kPreferencesFolderType,kDontCreateFolder,		// locate the folder
					&gPrefsFolderVRefNum,&gPrefsFolderDirID);
	if (iErr != noErr)
		DoFatalAlert("\pCannot locate Preferences folder.  Be sure you have a valid Preferences folder in your System Folder.");

	iErr = DirCreate(gPrefsFolderVRefNum,gPrefsFolderDirID,"\pMightyMike",&createdDirID);		// make MightyMike folder in there


            /* SEE IF GAME IS REGISTERED OR NOT */

#if !OEM
    CheckGameRegistration();
#else
    gGameIsRegistered = true;
#endif

}


/********************** CHECK GAME REGISTRATION *************************/

static void CheckGameRegistration(void)
{
OSErr   iErr;
FSSpec  spec;
Str255  regFileName = "\p:MightyMike:Info";
short		fRefNum;
long        	numBytes = REG_LENGTH;
unsigned char	regInfo[REG_LENGTH];

            /* GET SPEC TO REG FILE */

	iErr = FSMakeFSSpec(gPrefsFolderVRefNum, gPrefsFolderDirID, regFileName, &spec);
    if (iErr)
        goto game_not_registered;


            /*************************/
            /* VALIDATE THE REG FILE */
            /*************************/

            /* READ REG DATA */

    if (FSpOpenDF(&spec,fsCurPerm,&fRefNum) != noErr)
        goto game_not_registered;

	FSRead(fRefNum,&numBytes,regInfo);

    FSClose(fRefNum);

            /* VALIDATE IT */

    if (!ValidateRegistrationNumber(&regInfo[0]))
        goto game_not_registered;

    gGameIsRegistered = true;
    return;

        /* GAME IS NOT REGISTERED YET, SO DO DIALOG */

game_not_registered:

    DoRegistrationDialog(&regInfo[0]);

    if (gGameIsRegistered)                                  // see if write out reg file
    {
	    FSpDelete(&spec);	                                // delete existing file if any
	    iErr = FSpCreate(&spec,'MMik','xxxx',-1);
        if (iErr == noErr)
        {
        	numBytes = REG_LENGTH;
			FSpOpenDF(&spec,fsCurPerm,&fRefNum);
			FSWrite(fRefNum,&numBytes,regInfo);
		    FSClose(fRefNum);
     	}
    }

            /* DEMO MODE */
    else
    {
		/* SEE IF TIMER HAS EXPIRED */

        GetDemoTimer();
    	if (gDemoVersionTimer > ((GAME_FPS * 60) * 60))		// let play for n minutes
    	{
    		DoDemoExpiredScreen();
    		CleanQuit();
    	}
    }

}


/********************* VALIDATE REGISTRATION NUMBER ******************/

static Boolean ValidateRegistrationNumber(unsigned char *regInfo)
{
short   i,j;

            /* EXTRACT COMPONENTS */

    for (i = 0, j = REG_LENGTH-1; i < REG_LENGTH; i += 2, j -= 2)     // get checksum
    {
        Byte    value,c,d;

		if ((regInfo[i] >= 'a') && (regInfo[i] <= 'z'))	// convert to upper case
			regInfo[i] = 'A' + (regInfo[i] - 'a');

		if ((regInfo[j] >= 'a') && (regInfo[j] <= 'z'))	// convert to upper case
			regInfo[j] = 'A' + (regInfo[j] - 'a');

        value = regInfo[i] - 'A';           // convert letter to digit 0..9
        c = ('R' - regInfo[j]);             // convert character to number

        d = c - value;                      // the difference should be == i

        if (d != 0)
            return(false);
    }

    return(true);
}


/****************** DO REGISTRATION DIALOG *************************/

static void DoRegistrationDialog(unsigned char *out)
{
DialogPtr 		myDialog;
Boolean			dialogDone = false, isValid;
short			itemType,itemHit;
ControlHandle	itemHandle;
Rect			itemRect;
Str255          regInfo;

	FlushEvents ( everyEvent, REMOVE_ALL_EVENTS);
	myDialog = GetNewDialog(128,nil,MOVE_TO_FRONT);

				/* DO IT */

	while(dialogDone == false)
	{
		ModalDialog(nil, &itemHit);
		switch (itemHit)
		{
			case	1:									        // Register
					GetDialogItem(myDialog,4,&itemType,(Handle *)&itemHandle,&itemRect);
					GetDialogItemText((Handle)itemHandle,regInfo);
                    BlockMove(&regInfo[1], &regInfo[0], 100);         // shift out length byte

                    isValid = ValidateRegistrationNumber(regInfo);    // validate the number

                    if (isValid == true)
                    {
                        gGameIsRegistered = true;
                        dialogDone = true;
                        BlockMove(regInfo, out, REG_LENGTH);		// copy to output
                    }
                    else
                    {
                        DoAlert("\pSorry, that registration code is not valid.  Note that validation codes are case sensitive.  Please try again.");
                    }
					break;

            case    2:                                  // Demo
                    dialogDone = true;
                    break;

			case 	3:									// QUIT
                    CleanQuit();
					break;
		}
	}
	DisposeDialog(myDialog);

}




#pragma mark -




/****************** ALLOC HANDLE ********************/

Handle	AllocHandle(long size)
{
Handle	hand;

	hand = NewHandle(size);							// alloc in APPL
	if (hand == nil)
	{
#if BETA
		SysBeep(0);	//----------
#endif
		hand = NewHandleSys(size);					// try SYS
		return(hand);								// use TEMP
	}
	else
		return(hand);								// use APPL
}


/****************** ALLOC PTR ********************/

Ptr	AllocPtr(long size)
{
Ptr	pr;

	pr = NewPtr(size);						// alloc in APPL
	if (pr == nil)
	{
#if BETA
		SysBeep(0);	//----------
#endif
		return(NewPtrSys(size));			// alloc in SYS
	}
	else
		return(pr);
}


/******************* INIT THERMOMETER ***********************/
//
// Please wait thermometer.
//

void InitThermometer(void)
{
Rect	theRect;
ColorSpec 	aTable[2];

	SetRect(&theRect,0,0,VISIBLE_WIDTH,VISIBLE_HEIGHT);		// erase screen
	BlankScreenArea(theRect);


				/* SETUP B&W&R PALETTE */

	ReserveEntry(0,false);
	ProtectEntry(0,false);
	ReserveEntry(1,false);
	ProtectEntry(1,false);
	aTable[0].rgb.red =
	aTable[0].rgb.green =
	aTable[0].rgb.blue = 0xffff;						// white
	aTable[0].value = 0;
	aTable[1].rgb.red =	0xffff;							// red
	aTable[1].rgb.green =
	aTable[1].rgb.blue = 0x00ff;
	aTable[1].value = 0;
	SetEntries(0,1,aTable);								// use it

				/* INIT THERMOMETER */

	gThermometerX = gScreenXOffset+220;
	gThermometerY = gScreenYOffset+230;

//	SetPort(gGameWindow);								// show wait text
	ForeColor(whiteColor);
//	MoveTo(gThermometerX,gThermometerY+3);
//	TextMode(srcBic);
//	TextFace(0);
//	TextSize(9);
//	TextFont(monaco);
//	DrawString("\pCharging Batteries:");

	LoadIMAGE("\p:data:images:charging.image",SHOW_IMAGE_MODE_QUICK);

														// draw thermometer box

	SetRect(&theRect,gThermometerX,gThermometerY,gThermometerX+200,gThermometerY+16);
	FrameRect(&theRect);

}


/************************ FILL THERMOMETER ***********************/

void FillThermometer(short percent)
{
Rect	theRect;

	percent *= 2;

	ForeColor(redColor);
	SetRect(&theRect,gThermometerX+1,gThermometerY+1,
			gThermometerX+percent-2,gThermometerY+16-1);
	PaintRect(&theRect);
}


/**************** OPEN MIKE FILE **********************/

void	OpenMikeFile(Str255 filename,short *fRefNumPtr, Str255 errString)
{
short		vRefNum;
OSErr		iErr;
Byte		i,j;
Str255		newString;


				/* FIRST SEE IF WE CAN GET IT OFF OF DEFAULT VOLUME */

	GetVol(nil,&vRefNum);													// get default volume
	iErr = FSOpen(filename,vRefNum,fRefNumPtr);								// try to open
	if (iErr == noErr)
		return;

					/* NEW WE NEED TO SCAN OFF OF CD-ROM DRIVE */

	BlockMove(gBasePathName,newString,gBasePathName[0]+1);		// re-init base path string
	i = newString[0];											// get size of cd-rom name
	j = filename[0];
	BlockMove(&filename[1],&newString[i+1],j);
	newString[0] = i+j;


	iErr = FSOpen(newString,0,fRefNumPtr);						// try to open
	if (iErr == noErr)
		return;

	DoFatalAlert("\pCannot find file.  Re-insert the Power Pete CD-ROM and run the application again.");
}


/**************** OPEN MIKE REZ FILE **********************/

short	OpenMikeRezFile(Str255 filename,Str255 errString)
{
Str255		newString;
Byte		i,j;
short		srcFile;

				/* FIRST SEE IF WE CAN GET IT OFF OF DEFAULT VOLUME */

	srcFile = OpenResFile(filename);						// open resource fork
	if (srcFile != -1)										// see if error
		return(srcFile);

	return(-1);
}


/******************** MY RANDOM LONG **********************/
//
// My own random number generator that returns a LONG
//
// NOTE: call this instead of MyRandomShort if the value is going to be
//		masked or if it just doesnt matter since this version is quicker
//		without the 0xffff at the end.
//

unsigned long MyRandomLong(void)
{
  return seed2 ^= (((seed1 ^= (seed2>>5)*1568397607UL)>>7)+
                   (seed0 = (seed0+1)*3141592621UL))*2435386481UL;
}

/******************** MY RANDOM SHORT **********************/
//
// My own random number generator that returns a SHORT
//

unsigned short MyRandomShort(void)
{
  return (unsigned short)(seed2 ^= (((seed1 ^= (seed2>>5)*1568397607UL)>>7)+
                   (seed0 = (seed0+1)*3141592621UL))*2435386481UL) & 0xffffL;
}


/**************** SET MY RANDOM SEED *******************/

void SetMyRandomSeed(unsigned long seed)
{
	seed0 = seed;
	seed1 = 0;
	seed2 = 0;
}

#pragma mark -


/******************* GET DEMO TIMER *************************/

void GetDemoTimer(void)
{
OSErr				iErr;
short				refNum;
FSSpec				file;
long				count;

				/* READ TIMER FROM FILE */

	FSMakeFSSpec(gPrefsFolderVRefNum, gPrefsFolderDirID, "\pPimple", &file);
	iErr = FSpOpenDF(&file, fsRdPerm, &refNum);
	if (iErr)
		gDemoVersionTimer = 0;
	else
	{
		count = sizeof(gDemoVersionTimer);
		iErr = FSRead(refNum, &count,  &gDemoVersionTimer);			// read data from file
		if (iErr)
		{
			FSClose(refNum);
			FSpDelete(&file);										// file is corrupt, so delete
			gDemoVersionTimer = 0;
			return;
		}
		FSClose(refNum);
	}

}


/************************ SAVE DEMO TIMER ******************************/

void SaveDemoTimer(void)
{
FSSpec				file;
OSErr				iErr;
short				refNum;
long				count;

				/* CREATE BLANK FILE */

	FSMakeFSSpec(gPrefsFolderVRefNum, gPrefsFolderDirID, "\pPimple", &file);
	FSpDelete(&file);															// delete any existing file
	iErr = FSpCreate(&file, '????', 'xxxx', smSystemScript);					// create blank file
	if (iErr)
		return;


				/* OPEN FILE */

	FSMakeFSSpec(gPrefsFolderVRefNum, gPrefsFolderDirID, "\pPimple", &file);
	iErr = FSpOpenDF(&file, fsRdWrPerm, &refNum);
	if (iErr)
		return;

				/* WRITE DATA */

	count = sizeof(gDemoVersionTimer);
	FSWrite(refNum, &count, &gDemoVersionTimer);
	FSClose(refNum);
}





