/****************************/
/*        SPIN              */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/
#include <string.h>
#include "myglobals.h"
#include "spin.h"
#include "misc.h"
#include "io.h"
#include "input.h"
#include "sound2.h"
#include "window.h"
#include "externs.h"

/****************************/
/*    CONSTANTS             */
/****************************/

struct SpinHeaderType
{
	int16_t			width;				// width in pixels
	int16_t			height;
	int16_t			fps;
};
typedef struct SpinHeaderType SpinHeaderType;

struct SpinExtendedHeaderType
{
	int16_t		versNo;
	int16_t		width;				// width in pixels
	int16_t		height;
	int16_t		fps;
	Boolean		zoomFlag;
	Byte		undefined1;
	int16_t		undefined2;
	int16_t		undefined3;
};
typedef struct SpinExtendedHeaderType SpinExtendedHeaderType;


#define	SPIN_LOAD_SEG_SIZE		5000L				// # bytes to continue loading each frame (timing is more stable with smaller numbers)

/**********************/
/*     VARIABLES      */
/**********************/

static	Handle		gSpinFileHandle;
static	Ptr			gSpinPtr;						// ptr into gSpinFileHandle
static	Ptr			gSpinLoadPtr;					// ptr to location to continue loading data

static	SpinExtendedHeaderType	gSpinHeader;

static	short			gSpinX,gSpinY;


static	short	gSpinfRefNum;						// fRefNum for SPIN file
static	long	gSpinFileSize;						// remaining file size for SPIN file
static	long	gSpinReadSize;						// amount if file already read

static	Boolean	gDoublePix;

/******************** PLAY SPIN FILE *********************/
//
// NOTE: MUST HAVE ALREADY CALLED PreLoadSpinFile!!!!!!!
//
// INPUT: loopDuration is time in ticks to run, 0 = continuous
//

void PlaySpinFile(short	duration)
{
Ptr		loopPtr;
unsigned long	time;

	GetSpinHeader();										// get header
	GetSpinPalette();										// get palette
	DoSpinFrame();											// draw the first frame
	FadeInGameCLUT();										// fade in screen
	loopPtr = 	gSpinPtr;									// go back to here if loop
	time = TickCount();

	while (*gSpinPtr != SPIN_COMMAND_STOP)
	{
		UpdateInput();
		if (UserWantsOut())									// see if key stop
			goto bye;

		DoSoundMaintenance(true);								// (must be after readkeyboard)

		RegulateSpinSpeed(1000L*1000L/gSpinHeader.fps);

		switch(*gSpinPtr)
		{
			case 	SPIN_COMMAND_FRAMEDATA:
					DoSpinFrame();
					break;

			case 	SPIN_COMMAND_LOOP:
					gSpinPtr = loopPtr;
					DoSpinFrame();
					break;
		}

		PresentIndexedFramebuffer();

						/* CHECK FOR LIMITED DURATIONS */

		if (duration > 0)									// if 0, then we don't care
		{
			if (TickCount() != time)						// see if a tick has passed
			{
				if ((duration -= (TickCount() - time)) <= 0)
					goto bye;
				time = TickCount();
			}
		}
	}


				/* CLEANUP AND EXIT */
bye:
	DisposeHandle(gSpinFileHandle);							// zap the file
	FSClose(gSpinfRefNum);									// close the file
}


/******************* PRE-LOAD SPIN FILE ******************/
//
// Open SPIN file and preload initial data
//

void PreLoadSpinFile(Str255 fileName, long preLoadSize)
{
OSErr		iErr;
short		vRefNum;
Str255		volName;
static		long	mem,numToRead,decompSize;

			/* PREPARE SCREEN */

	BlankEntireScreenArea();


					/*  OPEN THE FILE */

	iErr = GetVol(volName,&vRefNum);					// get default volume
	OpenMikeFile(fileName,&gSpinfRefNum,"Cant open SPIN file!");

					/* GET SIZE OF FILE */

	if	(GetEOF(gSpinfRefNum,&gSpinFileSize) != noErr)
		DoFatalAlert("Err SPIN file EOF!");

				/* ALLOC MEMORY FOR THE FILE */

	gSpinFileHandle = AllocHandle(gSpinFileSize);
	if (gSpinFileHandle == nil)
		DoFatalAlert("Sorry, not enough memory to play SPIN file movie!");
	HLockHi(gSpinFileHandle);
	gSpinPtr = *gSpinFileHandle;								// set master process pointer
	gSpinLoadPtr = *gSpinFileHandle;							// set load pointer

				/* READ THE FILE */

	if (preLoadSize > gSpinFileSize)							// see if preloading too much
		preLoadSize = gSpinFileSize;

	iErr = FSRead(gSpinfRefNum,&preLoadSize,gSpinLoadPtr);		// read the preLoad amount only
	if (iErr != noErr)
		DoFatalAlert("Cant Read SPIN file!");

	gSpinLoadPtr += preLoadSize;								// set ptr to next load
	gSpinFileSize -= preLoadSize;								// dec size of remaining data

	gSpinReadSize = preLoadSize;								// set amount read
}


/***************** CONTINUE SPIN LOAD ***********************/
//
// returns true if anything was loaded
//

Boolean ContinueSpinLoad(void)
{
long	numBytes;
OSErr	iErr;

	if (!gSpinFileSize)											// see if anything to load
		return(false);

	numBytes = SPIN_LOAD_SEG_SIZE;								// assume load max

	if (numBytes > gSpinFileSize)								// see if too much
		numBytes = gSpinFileSize;

				/* READ THE FILE */

	iErr = FSRead(gSpinfRefNum,&numBytes,gSpinLoadPtr);			// read it
	if (iErr != noErr)
		DoFatalAlert("Cant Continue to Read SPIN file!");

	gSpinLoadPtr += numBytes;									// set ptr to next load
	gSpinFileSize -= numBytes;									// dec size of remaining data

	gSpinReadSize += numBytes;									// update size counter
	return(true);
}



/**************** GET SPIN HEADER *****************/
//
// This routine assumes that we are currently pointing at a header command!
//

void GetSpinHeader(void)
{
	switch(*gSpinPtr++)								// see if NORMAL OR EXTENDED Headers
	{
		case	SPIN_COMMAND_HEADER:
				// WARNING: shorts are not aligned to 16-bit boundaries
				gSpinHeader.width	= Byteswap16(gSpinPtr+0);	// get WIDTH
				gSpinHeader.height	= Byteswap16(gSpinPtr+2);	// get HEIGHT
				gSpinHeader.fps		= Byteswap16(gSpinPtr+4);	// get FPS
				gSpinPtr += 6;				// skip header
				gDoublePix = false;
				break;

		case	SPIN_COMMAND_EXTENDEDHEADER:
				DoFatalAlert("SPIN extended header not supported!");
#if 0
				intPtr = (short *)gSpinPtr;
				intPtr++;										// skip versNo
				ByteswapInts(2, 3, gSpinPtr);					// byteswap the next 3 shorts
				gSpinHeader.width = *intPtr++;					// get WIDTH
				gSpinHeader.height = *intPtr++;					// get HEIGHT
				gSpinHeader.fps	= *intPtr++;					// get FPS
				gSpinHeader.zoomFlag = gDoublePix = *((Ptr)(intPtr));	// get zoomflag
				gSpinPtr += sizeof(SpinExtendedHeaderType);		// skip header
#endif
				break;


		default:
				DoFatalAlert("Not Pointing to valid SPIN Header!");
	}


	if (gDoublePix)
	{
		gSpinX = (VISIBLE_WIDTH-(gSpinHeader.width*2))/2;		// calc coords to center on screen
		gSpinY = (VISIBLE_HEIGHT-(gSpinHeader.height*2))/2;
	}
	else
	{
		gSpinX = (VISIBLE_WIDTH-gSpinHeader.width)/2;
		gSpinY = (VISIBLE_HEIGHT-gSpinHeader.height)/2;
	}
}



/**************** GET SPIN PALETTE *****************/
//
// This routine assumes that we are currently pointing at a palette command!
//

void GetSpinPalette(void)
{
RGBColor	*rgbPtr,rgb;
short			i;

	if (*gSpinPtr++ != SPIN_COMMAND_PALETTE)			// verify command
		DoFatalAlert("Not Pointing to SPIN palette command!");

	rgbPtr = (RGBColor *)(gSpinPtr);					// get ptr to palette data
	for (i=0; i<256; i++)
	{
		rgb = *rgbPtr++;								// get a color
		gGamePalette[i] = RGBColorToU32(&rgb);			// set
	}

	gSpinPtr = (Ptr)rgbPtr;								// update file pointer
}


/**************** DO SPIN FRAME *****************/
//
// This routine assumes that we are currently pointing at a frame command!
//

void DoSpinFrame(void)
{
long	frameSize;
Byte	count;
Ptr		framePtr,srcPtr;
Ptr		framePtrBase;

	srcPtr = gSpinPtr;

	if (*srcPtr++ != SPIN_COMMAND_FRAMEDATA)			// verify command
		DoFatalAlert("Not Pointing to SPIN Frame command!");

	srcPtr += 4;										// skip packed size
	frameSize = Byteswap32(srcPtr);						// get unpacked size
	srcPtr += 4;

					/* GET MEMORY FOR FRAME */

	if ((framePtrBase = AllocPtr(frameSize)) == nil)
		DoFatalAlert ("No Memory for SPIN Frame!");
	framePtr = framePtrBase;

						/* UNPACK IT */
	do
	{
		count = *srcPtr++;									// get count byte

		if (count > 0x7f)									// (-) means PACKED data
		{
			count = (-count)+1;
			frameSize -= count;
			uint8_t data = *srcPtr++;						// get data byte (upper 3 bytes are trash)

			memset(framePtr, data, count);
			framePtr += count;
		}
		else												// (+) means NON-PACKED data
		{
			count++;
			frameSize -= count;

			while(srcPtr+count+200 > gSpinLoadPtr)			// see if @ end of current buffer (200 is leeway margin)
			{
				if (!ContinueSpinLoad())					// keep loading until we have enough or its @ EOF
					break;
			}

			memcpy(framePtr, srcPtr, count);
			framePtr += count;
			srcPtr += count;
		}
	} while (frameSize > 0);

						/* UPDATE THE SCREEN */

	DrawSpinFrame(framePtrBase);

					/* CLEANUP & EXIT */

	DisposePtr(framePtrBase);								// nuke expanded frame data
	gSpinPtr =	srcPtr;										// update file ptr
}


/******************** DRAW SPIN FRAME *******************/

void DrawSpinFrame(Ptr srcPtr)
{
	GAME_ASSERT_MESSAGE(!gDoublePix, "draw doubled was removed");	// see if draw doubled

	short numChunks = Byteswap16(srcPtr);					// get # chunks to update
	srcPtr += 2;

	if (numChunks == 0)
		return;

	do
	{
		int x = *srcPtr;				srcPtr += 1;		// get X coord (in longs)
		int y = Byteswap16(srcPtr);		srcPtr += 2;		// get Y coord
		int size = *srcPtr;				srcPtr += 1;		// get SIZE (# longs)

		x *= 4;												// X and size were given in longs to pre-optimize
		size *= 4;											// memory copy on 68k.

		uint8_t* destPtr = gScreenLookUpTable[y+gSpinY] + gSpinX + x;	// point to screen
		memcpy(destPtr, srcPtr, size);						// copy data
		srcPtr += size;

	}while(--numChunks);
}



/******************** REGULATE SPIN SPEED ***************/
//
// INPUT: speed = # microseconds to wait
//

void RegulateSpinSpeed(long speed)
{
    RegulateSpeed(speed/2);
}
