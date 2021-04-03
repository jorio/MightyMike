/****************************/
/*      MISC ROUTINES       */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/

/***************/
/* EXTERNALS   */
/***************/

#include <SDL.h>
#include "myglobals.h"
#include "window.h"
#include "io.h"
#include "object.h"
#include "misc.h"
#include "sound2.h"
#include "picture.h"
#include "input.h"
#include "objecttypes.h"
#include "cinema.h"
#include "externs.h"

/****************************/
/*    PROTOTYPES             */
/****************************/

/****************************/
/*    CONSTANTS             */
/****************************/

// Source port note: the game's Wait functions were pure spinlocks.
// I threw in a simple SDL_Delay in Wait's loop so the game consumes less power
// until I get around to implementing a better solution for speed regulation.
#define		SPINLOCK_DELAY		2

#define	DECOMP_PACKET_SIZE	20000L

									// FILE COMPRESSION TYPES
									//=======================

// Note: Mighty Mike data files only use RLB, NONE or RLW.
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


/**********************/
/*     VARIABLES      */
/**********************/

short	gPrefsFolderVRefNum;
long	gPrefsFolderDirID;


						/* GLOBAL FLAGS */

Boolean		gGlobalFlagList[MAX_GLOBAL_FLAGS];

short		gThermometerX,gThermometerY;

static	unsigned long seed0 = 0, seed1 = 0, seed2 = 0;

Byte		gRLBDecompBuffer[DECOMP_PACKET_SIZE];

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

	NumToStringC((long)err, numStr);
	DoAlert (numStr);
	CleanQuit();
}


/*********************** DO ALERT *******************/

void DoAlert(const char* s)
{
	fprintf(stderr, "MIKE ALERT: %s\n", s);

	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Mighty Mike", s, NULL);
}


/*********************** DO ASSERT *******************/

void DoAssert(const char* msg, const char* file, int line)
{
	printf("NANOSAUR ASSERTION FAILED: %s - %s:%d\n", msg, file, line);
	static char alertbuf[1024];
	snprintf(alertbuf, 1024, "%s\n%s:%d", msg, file, line);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Nanosaur: Assertion Failed!", alertbuf, NULL);
	ExitToShell();
}


/*********************** DO FATAL ALERT *******************/

void DoFatalAlert(const char* s)
{
	fprintf(stderr, "MIKE FATAL ALERT: %s\n", s);

	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Mighty Mike", s, NULL);
	CleanQuit();
}


/*********************** DO FATAL ALERT 2 *******************/

void DoFatalAlert2(const char* s1, const char* s2)
{
	static char alertbuf[1024];
	snprintf(alertbuf, 1024, "%s\n%s", s1, s2);
	fprintf(stderr, "MIKE FATAL ALERT: %s\n", alertbuf);

	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Mighty Mike", alertbuf, NULL);
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

exit:

	ExitToShell();
}



/************************* WAIT ***********************/
//
// Waits for specified # of ticks or mouse click
//
// OUTPUT: true if aborted early
//

Boolean Wait(long time)
{
uint32_t	old;

	while (time > 0)
	{
		old = TickCount();							// wait for 1 tick to pass
		while(TickCount() == old)
		{
			SDL_Delay(SPINLOCK_DELAY);
		}
		PresentIndexedFramebuffer();
		--time;
		ReadKeyboard();
		if (UserWantsOut())							// see if keyboard break out
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

		if (UserWantsOut())								// exit if key down
			break;

		DoSoundMaintenance(true);						// (must be after readkeyboard)
	}
}


/************************* WAIT 4 ***********************/
//
// Waits for specified # of ticks NO mouse click
//

void Wait4(long time)
{
uint32_t	start;

	start = TickCount();
	while (TickCount() < (start+time))
		SDL_Delay(SPINLOCK_DELAY);
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

			if (UserWantsOut())	// exit if key down
				break;

			DoSoundMaintenance(true);						// (must be after readkeyboard)
		} while(IsMusicPlaying() && (!gAbortDemoFlag));
	}
}

/******************** LOAD RAW FILE *****************/

Handle LoadRawFile(const char* fileName)
{
OSErr		iErr;
short		fRefNum;
long		fileSize;
Handle		dataHand;

	fRefNum = OpenMikeFile(fileName);

	iErr = GetEOF(fRefNum, &fileSize);
	GAME_ASSERT(iErr == noErr);

	dataHand = NewHandle(fileSize);
	GAME_ASSERT(dataHand);

	iErr = FSRead(fRefNum,&fileSize,*dataHand);
	GAME_ASSERT(iErr == noErr);

	iErr = FSClose(fRefNum);
	GAME_ASSERT(iErr == noErr);

	return dataHand;
}

/******************** LOAD PACKED FILE *****************/

Handle LoadPackedFile(const char* fileName)
{
OSErr		iErr;
short		fRefNum;
long		fileSize;
Handle		dataHand;
long		numToRead;
int32_t		decompSize;
int32_t		decompType;

					/*  OPEN THE FILE */

	fRefNum = OpenMikeFile(fileName);

					/* GET SIZE OF FILE */

	iErr = GetEOF(fRefNum, &fileSize);
	GAME_ASSERT_MESSAGE(iErr == noErr, "Packed file EOF!");

					/*	READ DECOMP SIZE */

	numToRead = 4;
	iErr = FSRead(fRefNum,&numToRead,(Ptr)&decompSize);			// read 4 byte length
	GAME_ASSERT_MESSAGE(iErr == noErr, "Error reading Packed data!");
	GAME_ASSERT(numToRead == 4);
	ByteswapInts(numToRead, 1, &decompSize);
	fileSize -= numToRead;

					/*	READ DECOMP TYPE */

	numToRead = 4;
	iErr = FSRead(fRefNum,&numToRead,(Ptr)&decompType);			// read compression type
	GAME_ASSERT_MESSAGE(iErr == noErr, "Error reading Packed data Header!");
	GAME_ASSERT(numToRead == 4);
	ByteswapInts(numToRead, 1, &decompType);
	fileSize -= numToRead;

					/* GET MEMORY FOR UNPACKED DATA */

	dataHand = NewHandle(decompSize);
	GAME_ASSERT_MESSAGE(dataHand, "No Memory for Unpacked Data!");

	switch(decompType)
	{
		case 	PACK_TYPE_RLB:
				DecompressRLBFile(fRefNum,*dataHand,decompSize);
				break;

		case	PACK_TYPE_RLW:
				RLW_Expand(fRefNum,(unsigned short *)*dataHand,fileSize);
				break;

		case	PACK_TYPE_NONE:
				FSRead(fRefNum,&fileSize,*dataHand);
				break;

		default:
		{
				char error[256];
				snprintf(error, 256, "Unsupported compression type %d", decompType);
				DoFatalAlert(error);
		}
	}


					/*  CLOSE THE FILE */

	iErr = FSClose(fRefNum);
	GAME_ASSERT_MESSAGE(iErr == noErr, "Can't close Packed file!");


					/*  DUMP UNPACKED DATA TO FILE (FOR DEBUGGING ONLY) */

#if !_WIN32 && _DEBUG
	char debugPathBuffer[256];
	snprintf(debugPathBuffer, sizeof(debugPathBuffer), "/tmp/MikeUnpack_%s",fileName);

	for (char* c = debugPathBuffer; *c; c++)	// replace colon characters in path
		if (*c == ':')
			*c = '=';

	FILE* debugFile = fopen(debugPathBuffer, "wb");
	if (debugFile)
	{
		fwrite(*dataHand, 1, GetHandleSize(dataHand), debugFile);
		fclose(debugFile);
		printf("Wrote: %s\n", debugPathBuffer);
	}
	else
	{
		DoFatalAlert2("Couldn't open debug file for writing", debugPathBuffer);
	}
#endif

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
//		DoFatalAlert2 ("No Memory for RLB UnPack Buffer!",gMemoryErr);

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
uint16_t	*srcOriginalPtr,*sourcePtr;
uint8_t		*lengthPtr;
uint16_t	runCount,seed;
long		buffSize;
long		numToRead;

				/* GET MEMORY FOR SOURCE DATA */

	srcOriginalPtr = (unsigned short *)&gRLBDecompBuffer[0];

//	srcOriginalPtr = (unsigned short *)AllocPtr(DECOMP_PACKET_SIZE);
//	if (srcOriginalPtr == nil)
//		DoFatalAlert("Couldnt allocate memory for RLW unpack buffer!");

				/* READ FIRST SOURCE PACKET */

	numToRead = DECOMP_PACKET_SIZE;
	FSRead(fRefNum, &numToRead, (Ptr) srcOriginalPtr);
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
			FSRead(fRefNum, &numToRead, (Ptr) srcOriginalPtr);
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
				FSRead(fRefNum, &numToRead, (Ptr) srcOriginalPtr);
				sourcePtr = srcOriginalPtr;
				buffSize = DECOMP_PACKET_SIZE;
			}
			seed = *sourcePtr++;							// get the packed seed
			sourceSize -= 2;
			if ((buffSize -= 2) == 0)						// see if need to refill buffer
			{
				numToRead = DECOMP_PACKET_SIZE;
				FSRead(fRefNum, &numToRead, (Ptr) srcOriginalPtr);
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
					FSRead(fRefNum, &numToRead, (Ptr) srcOriginalPtr);
					sourcePtr = srcOriginalPtr;
					buffSize = DECOMP_PACKET_SIZE;
				}
				*output++ = *sourcePtr++;
				sourceSize-=2;
				if ((buffSize -= 2) == 0)						// see if need to refill buffer
				{
					numToRead = DECOMP_PACKET_SIZE;
					FSRead(fRefNum, &numToRead, (Ptr) srcOriginalPtr);
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

		SDL_Delay(SPINLOCK_DELAY);

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
static uint32_t gTick = 0;

	while ((TickCount() - gTick) < speed)				// wait for 1 tick
		SDL_Delay(SPINLOCK_DELAY);
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


			/* CHECK PREFERENCES FOLDER */

	iErr = FindFolder(kOnSystemDisk,kPreferencesFolderType,kDontCreateFolder,		// locate the folder
					&gPrefsFolderVRefNum,&gPrefsFolderDirID);
	if (iErr != noErr)
		DoFatalAlert("Cannot locate Preferences folder.  Be sure you have a valid Preferences folder in your System Folder.");

	iErr = DirCreate(gPrefsFolderVRefNum,gPrefsFolderDirID,"MightyMike",&createdDirID);		// make MightyMike folder in there
}


#pragma mark -


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

#if 0
	ReserveEntry(0,false);
	ProtectEntry(0,false);
	ReserveEntry(1,false);
	ProtectEntry(1,false);
#endif
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
//	DrawString("Charging Batteries:");

	LoadImage(":images:charging.tga", 0);

														// draw thermometer box

TODO_REWRITE_THIS_MINOR();
#if 0
	SetRect(&theRect,gThermometerX,gThermometerY,gThermometerX+200,gThermometerY+16);
	FrameRect(&theRect);
#endif
}


/************************ FILL THERMOMETER ***********************/

void FillThermometer(short percent)
{
	TODO_REWRITE_THIS_MINOR();
#if 0
Rect	theRect;

	percent *= 2;

	ForeColor(redColor);
	SetRect(&theRect,gThermometerX+1,gThermometerY+1,
			gThermometerX+percent-2,gThermometerY+16-1);
	PaintRect(&theRect);
#endif
}


/**************** OPEN MIKE FILE **********************/

short OpenMikeFile(const char* filename)
{
OSErr		iErr;
FSSpec		spec;
short		fRefNum;

	FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, filename, &spec);

	iErr = FSpOpenDF(&spec, fsRdPerm, &fRefNum);			// try to open
	if (iErr != noErr)
	{
		DoFatalAlert2("Cannot open data file", filename);
		return -1;
	}

	return fRefNum;
}


/**************** OPEN MIKE REZ FILE **********************/

short OpenMikeRezFile(const char* filename)
{
short		srcFile;
OSErr		iErr;
FSSpec		spec;

	iErr = FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, filename, &spec);
	GAME_ASSERT(iErr == noErr);

	srcFile = FSpOpenResFile(&spec, fsRdPerm);			// try to open
	if (srcFile == -1)									// see if error
	{
		DoFatalAlert2("Cannot open resource file", filename);
		return -1;
	}

	UseResFile(srcFile);
	return srcFile;
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

