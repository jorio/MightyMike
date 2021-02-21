/***************************
 RUN-LENGTH - WORD COMPRESSION
****************************/


#include "RLW.h"
#include "misc.h"

extern	WindowPtr	gThermometerWindow;
extern	long		gInputSize,gOutputSize,gOriginalInputSize;
extern	Rect		gThermRect;


/******************** RLW EXPAND FILE *********************/

void RLW_Expand(short fRefNum, unsigned int *output, long sourceSize)
{
register unsigned int	*srcOriginalPtr,*sourcePtr;
register unsigned char	*lengthPtr;
register unsigned int	runCount,seed;

				/* GET MEMORY FOR SOURCE DATA */

	srcOriginalPtr = (unsigned int *)NewPtrClear(sourceSize);
	if (srcOriginalPtr == nil)
		DoFatalAlert("Couldnt allocate memory for RLW pack buffer!");
	sourcePtr = srcOriginalPtr;

				/* READ SOURCE DATA */

	FSRead(fRefNum,&sourceSize,srcOriginalPtr);

	while (sourceSize > 0)
	{
		lengthPtr = (unsigned char *)sourcePtr;
		runCount = *lengthPtr++;							// get length byte
		sourceSize--;
		sourcePtr = (unsigned int *)lengthPtr;
		if (runCount&0x80)									// see if packed stream or not
		{
					/* DECODE PACKED STREAM */

			seed = *sourcePtr++;							// get the packed seed
			sourceSize -= 2;
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
				*output++ = *sourcePtr++;
				sourceSize-=2;
			}
		}
	}

	DisposPtr((Ptr)srcOriginalPtr);
}



