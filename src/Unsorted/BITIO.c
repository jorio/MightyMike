/************************** Start of BITIO.C *************************
 *
 * This utility file contains all of the routines needed to impement
 * bit oriented routines under either ANSI or K&R C.  It needs to be
 * linked with every program used in the entire book.
 *
 */
#include "bitio.h"

extern	long		gInputSize,gOutputSize,gOriginalInputSize;




int InputBit( bit_file )
BIT_FILE *bit_file;
{
int value;

    if ( bit_file->mask == 0x80 )
    {
        bit_file->rack = *( bit_file->file++ );
    }
    value = bit_file->rack & bit_file->mask;
    bit_file->mask >>= 1;
    if ( bit_file->mask == 0 )
	bit_file->mask = 0x80;
    return( value ? 1 : 0 );
}

unsigned long InputBits( bit_file, bit_count )
BIT_FILE *bit_file;
int bit_count;
{
    unsigned long mask;
    unsigned long return_value;

    mask = 1L << ( bit_count - 1 );
    return_value = 0;
    while ( mask != 0) {
	if ( bit_file->mask == 0x80 )
	{
	    bit_file->rack = *bit_file->file++;
	}
	if ( bit_file->rack & bit_file->mask )
            return_value |= mask;
        mask >>= 1;
        bit_file->mask >>= 1;
        if ( bit_file->mask == 0 )
            bit_file->mask = 0x80;
    }
    return( return_value );
}


