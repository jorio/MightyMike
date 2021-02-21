/************************** Start of LZW15V.C *************************
 *
 * This is the LZW module which implements a more powerful version
 * of the algorithm.  This version of the program has three major
 * improvements over LZW12.C.  First, it expands the maximum code size
 * to 15 bits.  Second, it starts encoding with 9 bit codes, working
 * its way up in bit size only as necessary.  Finally, it flushes the
 * dictionary when done.
 *
 * Note that under MS-DOS this program needs to be built using the
 * Compact or Large memory model.
 *
 */

#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#include "bitio.h"
#include "lzw.h"
#include "misc.h"

extern	WindowPtr	gThermometerWindow;
extern	long		gInputSize,gOutputSize,gOriginalInputSize;

/*
 * Constants used throughout the program.  BITS defines the maximum
 * number of bits that can be used in the output code.  TABLE_SIZE defines
 * the size of the dictionary table.  TABLE_BANKS are the number of
 * 256 element dictionary pages needed.  The code defines should be
 * self-explanatory.
 */

#define BITS                       15
#define MAX_CODE                   ( ( 1 << BITS ) - 1 )
#define TABLE_SIZE                 35023L
#define TABLE_BANKS                ( ( TABLE_SIZE >> 8 ) + 1 )
#define END_OF_STREAM              256
#define BUMP_CODE                  257
#define FLUSH_CODE                 258
#define FIRST_CODE                 259
#define UNUSED                     -1


/*
 * This data structure defines the dictionary.  Each entry in the dictionary
 * has a code value.  This is the code emitted by the compressor.  Each
 * code is actually made up of two pieces:  a parent_code, and a
 * character.  Code values of less than 256 are actually plain
 * text codes.
 *
 * Note that in order to handle 16 bit segmented compilers, such as most
 * of the MS-DOS compilers, it was necessary to break up the dictionary
 * into a table of smaller dictionary pointers.  Every reference to the
 * dictionary was replaced by a macro that did a pointer dereference first.
 * By breaking up the index along byte boundaries we should be as efficient
 * as possible.
 */

struct dictionary {
    int code_value;
    int parent_code;
    char character;
} *dict[ TABLE_BANKS ];

#define DICT( i ) dict[ i >> 8 ][ i & 0xff ]

/*
 * Other global data structures.  The decode_stack is used to reverse
 * strings that come out of the tree during decoding.  next_code is the
 * next code to be added to the dictionary, both during compression and
 * decompression.  current_code_bits defines how many bits are currently
 * being used for output, and next_bump_code defines the code that will
 * trigger the next jump in word size.
 */

char	*decode_stack;
unsigned int next_code;
int		current_code_bits;
unsigned int next_bump_code;


/********************** INIT DICTIONARY ******************/
/*
 * This routine is used to initialize the dictionary, both when the
 * compressor or decompressor first starts up, and also when a flush
 * code comes in.  Note that even thought the decompressor sets all
 * the code_value elements to UNUSED, it doesn't really need to.
 */

void LZW_InitializeDictionary(void)
{
unsigned int i;

    for ( i = 0 ; i < TABLE_SIZE ; i++ )
        DICT( i ).code_value = UNUSED;

    next_code = FIRST_CODE;
    current_code_bits = 9;
    next_bump_code = 511;
}


/******************** INIT STORAGE *****************/
/*
 * This routine allocates the dictionary.  Since the total size of the
 * dictionary is much larger than 64K, it can't be allocated as a single
 * object.  Instead, it is allocated as a set of pointers to smaller
 * dictionary objects.  The special DICT() macro is used to translate
 * indices into pairs of references.
 */

void LZW_InitializeStorage(void)
{
int i;

    for ( i = 0 ; i < TABLE_BANKS ; i++ )
    {
        dict[ i ] = (struct dictionary *)NewPtrClear( 256 * sizeof ( struct dictionary ));
        if ( dict[ i ] == nil )
            DoFatalAlert( "Error allocating dictionary space" );
    }


    decode_stack = NewPtrClear(sizeof(char)*TABLE_SIZE);

}


/******************** ZAP STORAGE ***************/

void LZW_ZapStorage(void)
{
int i;

 	DisposPtr(decode_stack);

    for ( i = 0 ; i < TABLE_BANKS ; i++ )
    {
        DisposePtr((Ptr)dict[ i ]);
    }
}


/******************** LZW EXPAND FILE *********************/
/*
 * The file expander operates much like the encoder.  It has to
 * read in codes, the convert the codes to a string of characters.
 * The only catch in the whole operation occurs when the encoder
 * encounters a CHAR+STRING+CHAR+STRING+CHAR sequence.  When this
 * occurs, the encoder outputs a code that is not presently defined
 * in the table.  This is handled as an exception.  All of the special
 * input codes are handled in various ways.
 */

void LZW_Expand(short fRefNum, unsigned char *output, long sourceSize)
{
BIT_FILE *input,theBitFile;
register	unsigned char	*srcOriginalPtr,*sourcePtr;
register	unsigned int new_code;
register	unsigned int old_code;
register	int character;
register	unsigned int count;


				/* GET MEMORY FOR SOURCE DATA */

	srcOriginalPtr = (unsigned char *)NewPtrClear(sourceSize);
	if (srcOriginalPtr == nil)
		DoFatalAlert("Couldnt allocate memory for Z pack buffer!");
	sourcePtr = srcOriginalPtr;

				/* READ SOURCE DATA */

	FSRead(fRefNum,&sourceSize,srcOriginalPtr);


					/* INIT BIT_FILE */

	theBitFile.file = (unsigned char *)sourcePtr;
   	theBitFile.rack = 0;
    theBitFile.mask = 0x80;
	input = &theBitFile;

	gInputSize = sourceSize;
	gOutputSize = 0;							// get global size info


    LZW_InitializeStorage();
    for ( ; ; )
    {
        LZW_InitializeDictionary();
        old_code = (unsigned int) InputBits( input, current_code_bits );
        if ( old_code == END_OF_STREAM )
            goto bye;
        character = old_code;
        *output++ = old_code;
		gOutputSize++;

        for ( ; ; )
        {
            new_code = (unsigned int) InputBits( input, current_code_bits );

            if ((input->file - srcOriginalPtr) >= sourceSize)	//------ check if filled
            {
            	goto bye;
            }


            if ( new_code == END_OF_STREAM )
                goto bye;
            if ( new_code == FLUSH_CODE )
                break;
            if ( new_code == BUMP_CODE )
            {
                current_code_bits++;
                continue;
            }
            if ( new_code >= next_code )
            {
                decode_stack[ 0 ] = (char) character;
                count = LZW_decode_string( 1, old_code );
            }
            else
                count = LZW_decode_string( 0, new_code );
            character = decode_stack[ count - 1 ];
            while ( count > 0 )
            {
            	*output++ = decode_stack[ --count ];
				gOutputSize++;
           	}

            DICT( next_code ).parent_code = old_code;
            DICT( next_code ).character = (char) character;
            next_code++;
            old_code = new_code;
        }
    }
bye:
	LZW_ZapStorage();
	DisposPtr((Ptr)srcOriginalPtr);
}


/****************** DECODE STRING ******************/
/*
 * This routine decodes a string from the dictionary, and stores it
 * in the decode_stack data structure.  It returns a count to the
 * calling program of how many characters were placed in the stack.
 */

unsigned int LZW_decode_string( unsigned int count, unsigned int code )
{
register unsigned int	r_code,r_count;

	r_code = code;
	r_count = count;

    while ( r_code > 255 )
    {
        decode_stack[ r_count++ ] = DICT( r_code ).character;
        r_code = DICT( r_code ).parent_code;
    }
    decode_stack[ r_count++ ] = (char) r_code;
    return( r_count );
}




