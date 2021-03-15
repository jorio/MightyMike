/**************************************************************
	LZSS.C
***************************************************************

**************************************************************/


#include "myglobals.h"

#define RING_BUFF_SIZE		 4096							/* size of ring buffer */
#define UPPER_LIM		   	18								/* upper limit for match_length */
#define THRESHOLD			2   							/* encode string into position and length
						   									if match_length is greater than this */

static	unsigned char	*text_buf;					/* ring buffer of size N,with extra F-1 bytes to facilitate string comparison */


#include "misc.h"

/*==============================================================================*/

long LZSS_Decode(short fRefNum, Ptr destPtr, long sourceSize)
{
register	short				j,r,k,i;
register	unsigned short 	flags;
register	Ptr				srcOriginalPtr;
register	unsigned char 	*sourcePtr;
long			decompSize;

	decompSize = 0;

				/* GET MEMORY FOR TEXT_BUFF */

	text_buf = (unsigned char *)AllocPtr(sizeof(unsigned char) * (RING_BUFF_SIZE + UPPER_LIM - 1));
	if (text_buf == nil)
		DoFatalAlert("Couldnt alloc memory for ZS buffer!");

				/* GET MEMORY FOR LZSS DATA */

	srcOriginalPtr = AllocPtr(sourceSize);
	if (srcOriginalPtr == nil)
		DoFatalAlert("Couldnt allocate memory for pack buffer!");
	sourcePtr = (unsigned char *)srcOriginalPtr;

				/* READ LZSS DATA */

	FSRead(fRefNum,&sourceSize,srcOriginalPtr);



					/* DECOMPRESS IT */

	for (i = 0; i < (RING_BUFF_SIZE - UPPER_LIM); i++)						// clear buff to "default char"? (BLG)
		text_buf[i] = ' ';

	r = RING_BUFF_SIZE - UPPER_LIM;
	flags = 0;
	for ( ; ; )
	{
		if (!((flags >>= 1) & 256))
		{
			if (--sourceSize < 0)				// see if @ end of source data
				break;
																// get a source byte
			flags = (unsigned short)*sourcePtr++ | 0xff00;		// uses higher byte cleverly
		}
													// to count eight
		if (flags & 1)
		{
			if (--sourceSize < 0)				// see if @ end of source data
				break;
			text_buf[r++] = *destPtr++ = *sourcePtr++;		// get a source byte
			decompSize++;
			r &= (RING_BUFF_SIZE - 1);
		}
		else
		{
			if (--sourceSize < 0)				// see if @ end of source data
				break;
			i = *sourcePtr++;					// get a source byte
			if (--sourceSize < 0)				// see if @ end of source data
				break;
			j = *sourcePtr++;					// get a source byte

			i |= ((j & 0xf0) << 4);
			j = (j & 0x0f) + THRESHOLD;
			for (k = 0; k <= j; k++)
			{
				text_buf[r++] = *destPtr++ = text_buf[(i + k) & (RING_BUFF_SIZE - 1)];
				decompSize++;
				r &= (RING_BUFF_SIZE - 1);
			}
		}
	}

	DisposePtr(srcOriginalPtr);				// release the memory for packed buffer
	DisposePtr((Ptr)text_buf);

	return(decompSize);
}






