/****************************/
/*        SPIN              */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/
#include "myglobals.h"
//#include <pictutils.h>
#include "spin.h"
#include "misc.h"
#include "io.h"
//#include <timer.h>
#include "sound2.h"
#include "window.h"

extern	GamePalette		gGamePalette;
extern	char  			gMMUMode;
extern	long			gScreenLookUpTable[];
extern	long		gFrames,gScreenRowOffsetLW,gScreenRowOffset;

/****************************/
/*    CONSTANTS             */
/****************************/

#define		DrawDoubleMac	move.w	(srcPtr),col		\
							move.b	(srcPtr)+,col		\
							swap	col					\
							move.w	(srcPtr),col		\
							move.b	(srcPtr)+,col		\
							move.l	col,(destPtr)+		\
							move.l	col,(destPtr2)+


struct SpinHeaderType
{
	short			width;				// width in pixels
	short			height;
	short			fps;
};
typedef struct SpinHeaderType SpinHeaderType;

struct SpinExtendedHeaderType
{
	short		versNo;
	short		width;				// width in pixels
	short		height;
	short		fps;
	Boolean		zoomFlag;
	Byte		undefined1;
	short		undefined2;
	short		undefined3;
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
		ReadKeyboard();
		if (GetKeyState(KEY_SPACE) || (GetKeyState2(KEY_RETURN)))	// see if key stop
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
short		*intPtr;

	switch(*gSpinPtr++)								// see if NORMAL OR EXTENDED Headers
	{
		case	SPIN_COMMAND_HEADER:
				intPtr = (short *)gSpinPtr;
				gSpinHeader.width = *intPtr++;					// get WIDTH
				gSpinHeader.height = *intPtr++;					// get HEIGHT
				gSpinHeader.fps	= *intPtr++;					// get FPS
				gSpinPtr += sizeof(SpinHeaderType);				// skip header
				gDoublePix = false;
				break;

		case	SPIN_COMMAND_EXTENDEDHEADER:
				intPtr = (short *)gSpinPtr;
				intPtr++;										// skip versNo
				gSpinHeader.width = *intPtr++;					// get WIDTH
				gSpinHeader.height = *intPtr++;					// get HEIGHT
				gSpinHeader.fps	= *intPtr++;					// get FPS
				gSpinHeader.zoomFlag = gDoublePix = *((Ptr)(intPtr));	// get zoomflag
				gSpinPtr += sizeof(SpinExtendedHeaderType);		// skip header
				break;


		default:
				DoFatalAlert("Not Pointing to valid SPIN Header!");
	}


	if (gDoublePix)
	{
		gSpinX = (640-(gSpinHeader.width*2))/2;		// calc coords to center on screen
		gSpinY = (480-(gSpinHeader.height*2))/2;
	}
	else
	{
		gSpinX = (640-gSpinHeader.width)/2;
		gSpinY = (480-gSpinHeader.height)/2;
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
		SetEntryColor(gGamePalette,i,&rgb);				// set
	}

	gSpinPtr = (Ptr)rgbPtr;								// update file pointer
}


#ifdef __MWERKS__
//=======================================================================================
//                  POWERPC CODE
//=======================================================================================

/**************** DO SPIN FRAME *****************/
//
// This routine assumes that we are currently pointing at a frame command!
//

static void DoSpinFrame(void)
{
long	*longPtr;
long	frameSize,data;
Byte	count;
Ptr		framePtr,srcPtr;
Ptr		framePtrBase;

	srcPtr = gSpinPtr;

	if (*srcPtr++ != SPIN_COMMAND_FRAMEDATA)			// verify command
		DoFatalAlert("Not Pointing to SPIN Frame command!");

	longPtr = (long *)srcPtr;
	longPtr++;											// skip packed size
	frameSize = *longPtr++;								// get unpacked size
	srcPtr = (Ptr)longPtr;

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
			data = *srcPtr++;								// get data byte (upper 3 bytes are trash)

			for (;count>0; count--)
				*framePtr++ = data;
		}
		else												// (+) means NON-PACKED data
		{
			count++;
			frameSize -= count;

			while((StripAddress(srcPtr)+count+200) > StripAddress(gSpinLoadPtr))	// see if @ end of current buffer (200 is leeway margin)
			{
				if (!ContinueSpinLoad())					// keep loading until we have enough or its @ EOF
					break;
			}

			for (; count>0; count--)
				*framePtr++ = *srcPtr++;

		}
	} while (frameSize > 0);

						/* UPDATE THE SCREEN */

	DrawSpinFrame(framePtrBase);

					/* CLEANUP & EXIT */

	DisposePtr(framePtrBase);								// nuke expanded frame data
	gSpinPtr =	srcPtr;										// update file ptr
}


/******************** DRAW SPIN FRAME *******************/

static void DrawSpinFrame(Ptr sourcePtr)
{
short	numChunks,y;
Byte	x,size;
long	*destPtr;
Ptr		srcPtr;
short	*intPtr,i;
long	*longPtr;

	if (gDoublePix)											// see if draw doubled
	{
		DrawSpinFrame_Double(sourcePtr);
		return;
	}

	intPtr = (short *)sourcePtr;
	numChunks = *intPtr++;									// get # chunks to update
	srcPtr = StripAddress((Ptr)intPtr);

	if (numChunks == 0)
		return;

//	gMMUMode = true32b;										// we must do this in 32bit addressing mode
//	SwapMMUMode(&gMMUMode);

	do
	{
		x = *srcPtr++;										// get X coord (in longs)
		intPtr = (short *)srcPtr;
		y = *intPtr++;										// get Y coord
		srcPtr = (Ptr)intPtr;
		size = *srcPtr++;									// get SIZE (# longs)

		destPtr = (long *)(gScreenLookUpTable[y+gSpinY]+gSpinX)+x;		// point to screen

		longPtr = (long *)srcPtr;
		for (i=0; i < size; i++)							// copy data
			*destPtr++ = *longPtr++;
		srcPtr = (Ptr)longPtr;

	}while(--numChunks);

//	SwapMMUMode(&gMMUMode);								// Restore addressing mode
}



/******************** DRAW SPIN FRAME: DOUBLE *******************/
//
// Draws with double magification pixels
//

static void DrawSpinFrame_Double(Ptr sourcePtr)
{
unsigned short	numChunks,y;
Byte	x,size,pixel;
Ptr		destPtr,destPtr2;
long	i;
Ptr		srcPtr;
short	*intPtr;

	intPtr = (short *)sourcePtr;
	numChunks = *intPtr++;									// get # chunks to update
	srcPtr = StripAddress((Ptr)intPtr);

	if (numChunks == 0)
		return;

//	gMMUMode = true32b;										// we must do this in 32bit addressing mode
//	SwapMMUMode(&gMMUMode);

	do
	{
		x = *srcPtr++;										// get Y coord (in longs)
		intPtr = (short *)srcPtr;
		y = *intPtr;										// get X coord
		srcPtr = (Ptr)intPtr;
		size = *srcPtr;										// get SIZE (# longs)

		destPtr = (Ptr)(gScreenLookUpTable[(y<<1)+gSpinY]+gSpinX)+(x<<1); // point to screen
		destPtr2 = destPtr+gScreenRowOffset;

		size <<= 2;							// convert to # bytes wide
		for (i=0; i < size; i++)
		{
			pixel = *srcPtr++;
			*destPtr++ = pixel;
			*destPtr++ = pixel;
			*destPtr2++ = pixel;
			*destPtr2++ = pixel;
		}


	}while(--numChunks);

//	SwapMMUMode(&gMMUMode);								// Restore addressing mode
}


//=======================================================================================
//                  68000 CODE
//=======================================================================================

#else
/**************** DO SPIN FRAME *****************/
//
// This routine assumes that we are currently pointing at a frame command!
//

void DoSpinFrame(void)
{
register long	*longPtr;
register long	frameSize,col,data;
register Byte	count;
register Ptr	framePtr,srcPtr;
static	 Ptr	framePtrBase;
static	 unsigned long	temp;

	srcPtr = gSpinPtr;								// use register for speed

	if (*srcPtr++ != SPIN_COMMAND_FRAMEDATA)			// verify command
		DoFatalAlert("Not Pointing to SPIN Frame command!");

	longPtr = (long *)srcPtr;
	longPtr++;											// skip packed size
	frameSize = *longPtr++;								// get unpacked size
	srcPtr = (Ptr)longPtr;

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
			col = (32-(count>>2))<<1;						// see how many longs we need

			TODO_REWRITE_ASM();
#if 0	// TODO REWRITE ASM!
			asm
			{
					move.b	(srcPtr)+,data					// get data byte (upper 3 bytes are trash)

					move.l	#0,temp
					move.b	data,temp						// build a full long value
					or.b	data,temp+1
					or.b	data,temp+2
					or.b	data,temp+3
					move.l	temp,data

					jmp		@inline(col)

				@inline
					move.l	data,(framePtr)+			//0..15 longs
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+

					move.l	data,(framePtr)+			//16..31 longs
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
					move.l	data,(framePtr)+
			}
#endif

			switch(count&b11)							// do remainder of line
			{
				case 3:
						*framePtr++ = data;
				case 2:
						*framePtr++ = data;
				case 1:
						*framePtr++ = data;
			}
		}
		else												// (+) means NON-PACKED data
		{
			count++;
			frameSize -= count;

			while((StripAddress(srcPtr)+count+200) > StripAddress(gSpinLoadPtr))	// see if @ end of current buffer (200 is leeway margin)
			{
				if (!ContinueSpinLoad())					// keep loading until we have enough or its @ EOF
					break;
			}

			data = count&b11;								// (use data as temp storage for remainder)
			count >>= 2;									// see how many longs
			col = (32-count)<<1;

			TODO_REWRITE_ASM();
#if 0	// TODO REWRITE ASM!
			asm
			{
					jmp		@inline2(col)

				@inline2
					move.l	(srcPtr)+,(framePtr)+	//0..15 longs
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+

					move.l	(srcPtr)+,(framePtr)+	//16..31 longs
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+
					move.l	(srcPtr)+,(framePtr)+


			}
#endif

			switch(data)															// do remainder of line
			{
				case 3:
						*framePtr++ = *srcPtr++;
				case 2:
						*framePtr++ = *srcPtr++;
				case 1:
						*framePtr++ = *srcPtr++;
			}

		}
	} while (frameSize > 0);

						/* UPDATE THE SCREEN */

	DrawSpinFrame(framePtrBase);

					/* CLEANUP & EXIT */

	DisposePtr(framePtrBase);								// nuke expanded frame data
	gSpinPtr =	srcPtr;										// update file ptr
}


/******************** DRAW SPIN FRAME *******************/

void DrawSpinFrame(Ptr sourcePtr)
{
register short	numChunks,y;
register Byte	x,size;
register long	*destPtr,col;
register Ptr	srcPtr;

	if (gDoublePix)											// see if draw doubled
	{
		DrawSpinFrame_Double(sourcePtr);
		return;
	}

	srcPtr = StripAddress(sourcePtr);
	TODO_REWRITE_ASM();
#if 0	// TODO REWRITE ASM!
	asm
	{
		move.w	(srcPtr)+,numChunks							// get # chunks to update
	}
#endif

	if (numChunks == 0)
		return;

//	gMMUMode = true32b;										// we must do this in 32bit addressing mode
//	SwapMMUMode(&gMMUMode);

	do
	{
		TODO_REWRITE_ASM();
#if 0	// TODO REWRITE ASM!
		asm
		{
			move.b	(srcPtr)+,x								// get Y coord (in longs)
			move.w	(srcPtr)+,y								// get X coord
			move.b  (srcPtr)+,size							// get SIZE (# longs)
		}
#endif

		destPtr = (long *)(gScreenLookUpTable[y+gSpinY]+gSpinX)+x;		// point to screen

		col = (160-size)<<1;

		TODO_REWRITE_ASM();
#if 0	// TODO REWRITE ASM!
		asm
		{
				jmp		@inline(col)

			@inline
				move.l	(srcPtr)+,(destPtr)+	//0..15 longs
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+

				move.l	(srcPtr)+,(destPtr)+	//16..31
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+


				move.l	(srcPtr)+,(destPtr)+	//32..47
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+

				move.l	(srcPtr)+,(destPtr)+	//48..63
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+

				move.l	(srcPtr)+,(destPtr)+	//64..79
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+

				move.l	(srcPtr)+,(destPtr)+	//80..95
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+

				move.l	(srcPtr)+,(destPtr)+	//96..111
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+

				move.l	(srcPtr)+,(destPtr)+	//112..127
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+

				move.l	(srcPtr)+,(destPtr)+	//128..143
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+

				move.l	(srcPtr)+,(destPtr)+	//144..159
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
				move.l	(srcPtr)+,(destPtr)+
		}
#endif

	}while(--numChunks);

//	SwapMMUMode(&gMMUMode);								// Restore addressing mode
}



/******************** DRAW SPIN FRAME: DOUBLE *******************/
//
// Draws with double magification pixels
//

void DrawSpinFrame_Double(Ptr sourcePtr)
{
register unsigned short	numChunks,y;
register Byte			x,size;
register long			*destPtr,*destPtr2,col;
register Ptr			srcPtr;

	srcPtr = StripAddress(sourcePtr);
	TODO_REWRITE_ASM();
#if 0	// TODO REWRITE ASM!
	asm
	{
		move.w	(srcPtr)+,numChunks							// get # chunks to update
	}
#endif

	if (numChunks == 0)
		return;

//	gMMUMode = true32b;										// we must do this in 32bit addressing mode
//	SwapMMUMode(&gMMUMode);

	do
	{
		TODO_REWRITE_ASM();
#if 0	// TODO REWRITE ASM!
		asm
		{
			move.b	(srcPtr)+,x								// get Y coord (in longs)
			move.w	(srcPtr)+,y								// get X coord
			move.b  (srcPtr)+,size							// get SIZE (# longs)
		}
#endif

		destPtr = (long *)(gScreenLookUpTable[(y<<1)+gSpinY]+gSpinX)+(x<<1); // point to screen
		destPtr2 = destPtr+gScreenRowOffsetLW;

		col = (160-(size<<1))*14;
		TODO_REWRITE_ASM();
#if 0	// TODO REWRITE ASM!
		asm
		{
				jmp		@inline(col)

			@inline
				DrawDoubleMac								//0..15 (reads a word, writes a long)
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac

				DrawDoubleMac								//16..31
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac


				DrawDoubleMac								//32..47
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac

				DrawDoubleMac								//48..63
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac

				DrawDoubleMac								//64..79
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac

				DrawDoubleMac								//80..95
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac

				DrawDoubleMac								//96..111
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac

				DrawDoubleMac								//112..127
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac

				DrawDoubleMac								//128..143
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac

				DrawDoubleMac								//144..159
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
				DrawDoubleMac
		}
#endif

	}while(--numChunks);

//	SwapMMUMode(&gMMUMode);								// Restore addressing mode
}

#endif


/******************** REGULATE SPIN SPEED ***************/
//
// INPUT: speed = # microseconds to wait
//

void RegulateSpinSpeed(long speed)
{
    RegulateSpeed(speed/2);
}
