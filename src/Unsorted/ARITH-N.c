/************************** Start of ARITH-N.C *************************
 *
 * This is the order-n arithmetic coding module used in the final
 * part of chapter 6.
 *
 * Compile with BITIO.C, ERRHAND.C, and either MAIN-C.C or MAIN-E.C
 * This program should be compiled in large model.  An even better
 * alternative is a DOS Extender.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitio.h"
#include "arith-n.h"
#include "misc.h"


#define MAXIMUM_SCALE   16383  			// Maximum allowed frequency count
#define ESCAPE          256    			// The escape symbol
#define DONE            (-1)   			// The output stream empty  symbol
#define FLUSH           (-2)   			// The symbol to flush the model


char *CompressionName = "Adaptive order n model with arithmetic coding";
char *Usage           = "in-file out-file [ -o order ]\n\n";
int gMaxOrder         = 3;							// I CAN CHANGE THIS (WAS AN INPUT OPTION)



CONTEXT **gContexts;
int current_order;
short int totals[ 258 ];
char scoreboard[ 256 ];

static unsigned short int code;  /* The present input code value       */
static unsigned short int low;   /* Start of the current code range    */
static unsigned short int high;  /* End of the current code range      */
long underflow_bits;             /* Number of underflow bits pending   */



/***************** EXPAND FILE *********************/
/*
 * The main loop for expansion is very similar to the expansion
 * routine used in the simpler compression program, ARITH1E.C.  The
 * routine first has to initialize the the arithmetic coder and the
 * model.  The decompression loop differs in a couple of respects.
 * First of all, it handles the special ESCAPE character, by
 * removing them from the input bit stream but just throwing them
 * away otherwise. Secondly, it handles the special FLUSH character.
 * Once the main decoding loop is done, the cleanup code is called,
 * and the program exits.
 *
 */

void ARITHN_Expand(short fRefNum, Ptr output, long sourceSize)
{
static	SYMBOL	s;
register int	c;
register int	count;
static	BIT_FILE	*input,theBitFile;
register	Ptr			srcOriginalPtr;
register	Ptr			sourcePtr;


				/* GET MEMORY FOR SOURCE DATA */

	srcOriginalPtr = NewPtr(sourceSize);
	if (srcOriginalPtr == nil)
		DoFatalAlert("\pCouldnt allocate memory for ART-N pack buffer!");
	sourcePtr = srcOriginalPtr;

				/* READ SOURCE DATA */

	FSRead(fRefNum,&sourceSize,srcOriginalPtr);


					/* INIT BIT_FILE */

	theBitFile.file = (unsigned char *)sourcePtr;
   	theBitFile.rack = 0;
    theBitFile.mask = 0x80;
	input = &theBitFile;

	gInputSize = gOutputSize = 0;							// get global size info

    ARITHN_initialize_model();
    ARITHN_initialize_arithmetic_decoder( input );

    for ( ; ; )
    {
        do
        {
            ARITHN_get_symbol_scale( &s );
            count = ARITHN_get_current_count( &s );
            c = ARITHN_convert_symbol_to_int( count, &s );
            ARITHN_remove_symbol_from_stream( input, &s );
        } while ( c == ESCAPE );

        if ( c == DONE )
            break;

        if ( c != FLUSH )
        	*output++ = c;
        else
            ARITHN_flush_model();

        ARITHN_update_model( c );
        ARITHN_add_character_to_model( c );
    }
	DisposePtr(srcOriginalPtr);				// release the memory for packed buffer
}


/*************** INITIALIZE MODEL ********************/
/*
 * This routine has to get everything set up properly so that
 * the model can be maintained properly.  The first step is to create
 * the *gContexts[] array used later to find current context tables.
 * The *gContexts[] array indices go from -2 up to gMaxOrder, so
 * the table needs to be fiddled with a little.  This routine then
 * has to create the special order -2 and order -1 tables by hand,
 * since they aren't quite like other tables.  Then the current
 * context is set to \0, \0, \0, ... and the appropriate tables
 * are built to support that context.  The current order is set
 * to gMaxOrder, the scoreboard is cleared, and the system is
 * ready to go.
 */

void ARITHN_initialize_model(void)
{
register int i;
register CONTEXT *null_table;
register CONTEXT *control_table;

    current_order = gMaxOrder;
    gContexts = (CONTEXT **) calloc( sizeof( CONTEXT * ), 10 );
    if ( gContexts == NULL )
        DoFatalAlert( "\pFailure #1: allocating context table!" );
    gContexts += 2;
    null_table = (CONTEXT *) calloc( sizeof( CONTEXT ), 1 );
    if ( null_table == NULL )
        DoFatalAlert( "\pFailure #2: allocating null table!" );
    null_table->max_index = -1;
    gContexts[ -1 ] = null_table;
    for ( i = 0 ; i <= gMaxOrder ; i++ )
        gContexts[ i ] = ARITHN_allocate_next_order_table( gContexts[ i-1 ],
                                               0,
                                               gContexts[ i-1 ] );
    free( (char *) null_table->stats );
    null_table->stats =
         (STATS *) calloc( sizeof( STATS ), 256 );
    if ( null_table->stats == NULL )
        DoFatalAlert( "\pFailure #3: allocating null table!" );
    null_table->max_index = 255;
    for ( i=0 ; i < 256 ; i++ ) {
        null_table->stats[ i ].symbol = (unsigned char) i;
        null_table->stats[ i ].counts = 1;
    }

    control_table = (CONTEXT *) calloc( sizeof(CONTEXT), 1 );
    if ( control_table == NULL )
        DoFatalAlert( "\pFailure #4: allocating null table!" );
    control_table->stats =
         (STATS *) calloc( sizeof( STATS ), 2 );
    if ( control_table->stats == NULL )
        DoFatalAlert( "\pFailure #5: allocating null table!" );
    gContexts[ -2 ] = control_table;
    control_table->max_index = 1;
    control_table->stats[ 0 ].symbol = -FLUSH;
    control_table->stats[ 0 ].counts = 1;
    control_table->stats[ 1 ].symbol = -DONE;
    control_table->stats[ 1 ].counts = 1;

    for ( i = 0 ; i < 256 ; i++ )
        scoreboard[ i ] = 0;
}


/********** ALLOCATE NEXT ORDER TABLE **************/
/*
 * This is a utility routine used to create new tables when a new
 * context is created.  It gets a pointer to the current context,
 * and gets the symbol that needs to be added to it.  It also needs
 * a pointer to the lesser context for the table that is to be
 * created.  For example, if the current context was "ABC", and the
 * symbol 'D' was read in, ARITHN_add_character_to_model would need to
 * create the new context "BCD".  This routine would get called
 * with a pointer to "BC", the symbol 'D', and a pointer to context
 * "CD".  This routine then creates a new table for "BCD", adds it
 * to the link table for "BC", and gives "BCD" a back pointer to
 * "CD".  Note that finding the lesser context is a difficult
 * task, and isn't done here.  This routine mainly worries about
 * modifying the stats and links fields in the current context.
 */

CONTEXT *ARITHN_allocate_next_order_table(CONTEXT *table, int symbol,CONTEXT *lesser_context )
{
register CONTEXT *new_table;
register int i;
register unsigned int new_size;

    for ( i = 0 ; i <= table->max_index ; i++ )
        if ( table->stats[ i ].symbol == (unsigned char) symbol )
            break;
    if ( i > table->max_index ) {
        table->max_index++;
        new_size = sizeof( LINKS );
        new_size *= table->max_index + 1;
        if ( table->links == NULL )
            table->links = (LINKS *) calloc( new_size, 1 );
        else
            table->links = (LINKS *)
                 realloc( (char *) table->links, new_size );
        new_size = sizeof( STATS );
        new_size *= table->max_index + 1;
        if ( table->stats == NULL )
            table->stats = (STATS *) calloc( new_size, 1 );
        else
            table->stats = (STATS *)
                realloc( (char *) table->stats, new_size );
        if ( table->links == NULL )
            DoFatalAlert( "\pFailure #6: allocating new table" );
        if ( table->stats == NULL )
            DoFatalAlert( "\pFailure #7: allocating new table" );
        table->stats[ i ].symbol = (unsigned char) symbol;
        table->stats[ i ].counts = 0;
    }
    new_table = (CONTEXT *) calloc( sizeof( CONTEXT ), 1 );
    if ( new_table == NULL )
        DoFatalAlert( "\pFailure #8: allocating new table" );
    new_table->max_index = -1;
    table->links[ i ].next = new_table;
    new_table->lesser_context = lesser_context;
    return( new_table );
}

/**************** UPDATE MODEL **************/
/*
 * This routine is called to increment the counts for the current
 * gContexts.  It is called after a character has been encoded or
 * decoded.  All it does is call ARITHN_update_table for each of the
 * current gContexts, which does the work of incrementing the count.
 * This particular version of ARITHN_update_model() practices update exclusion,
 * which means that if lower order models weren't used to encode
 * or decode the character, they don't get their counts updated.
 * This seems to improve compression performance quite a bit.
 * To disable update exclusion, the loop would be changed to run
 * from 0 to gMaxOrder, instead of current_order to gMaxOrder.
 */
void ARITHN_update_model( int symbol )
{
register int i;
register int local_order;

    if ( current_order < 0 )
        local_order = 0;
    else
        local_order = current_order;
    if ( symbol >= 0 ) {
        while ( local_order <= gMaxOrder ) {
            if ( symbol >= 0 )
                ARITHN_update_table( gContexts[ local_order ], symbol );
            local_order++;
        }
    }
    current_order = gMaxOrder;
    for ( i = 0 ; i < 256 ; i++ )
        scoreboard[ i ] = 0;
}


/*************** UPDATE TABLE ******************/
/*
 * This routine is called to update the count for a particular symbol
 * in a particular table.  The table is one of the current gContexts,
 * and the symbol is the last symbol encoded or decoded.  In principle
 * this is a fairly simple routine, but a couple of complications make
 * things a little messier.  First of all, the given table may not
 * already have the symbol defined in its statistics table.  If it
 * doesn't, the stats table has to grow and have the new guy added
 * to it.  Secondly, the symbols are kept in sorted order by count
 * in the table so as that the table can be trimmed during the flush
 * operation.  When this symbol is incremented, it might have to be moved
 * up to reflect its new rank.  Finally, since the counters are only
 * bytes, if the count reaches 255, the table absolutely must be rescaled
 * to get the counts back down to a reasonable level.
 */

void ARITHN_update_table(CONTEXT *table, int symbol )
{
register int i;
register int index;
register unsigned char temp;
register CONTEXT *temp_ptr;
register unsigned int new_size;

/*
 * First, find the symbol in the appropriate context table.  The first
 * symbol in the table is the most active, so start there.
 */
    index = 0;
    while ( index <= table->max_index &&
            table->stats[index].symbol != (unsigned char) symbol )
        index++;
    if ( index > table->max_index ) {
        table->max_index++;
        new_size = sizeof( LINKS );
        new_size *= table->max_index + 1;
        if ( current_order < gMaxOrder ) {
            if ( table->max_index == 0 )
                table->links = (LINKS *) calloc( new_size, 1 );
            else
                table->links = (LINKS *)
                   realloc( (char *) table->links, new_size );
            if ( table->links == NULL )
                DoFatalAlert( "\pError #9: reallocating table space!" );
            table->links[ index ].next = NULL;
        }
        new_size = sizeof( STATS );
        new_size *= table->max_index + 1;
        if (table->max_index==0)
            table->stats = (STATS *) calloc( new_size, 1 );
        else
            table->stats = (STATS *)
                realloc( (char *) table->stats, new_size );
        if ( table->stats == NULL )
            DoFatalAlert( "\pError #10: reallocating table space!" );
        table->stats[ index ].symbol = (unsigned char) symbol;
        table->stats[ index ].counts = 0;
    }
/*
 * Now I move the symbol to the front of its list.
 */
    i = index;
    while ( i > 0 &&
            table->stats[ index ].counts == table->stats[ i-1 ].counts )
        i--;
    if ( i != index ) {
        temp = table->stats[ index ].symbol;
        table->stats[ index ].symbol = table->stats[ i ].symbol;
        table->stats[ i ].symbol = temp;
        if ( table->links != NULL ) {
            temp_ptr = table->links[ index ].next;
            table->links[ index ].next = table->links[ i ].next;
            table->links[ i ].next = temp_ptr;
        }
        index = i;
    }
/*
 * The switch has been performed, now I can update the counts
 */
    table->stats[ index ].counts++;
    if ( table->stats[ index ].counts == 255 )
        ARITHN_rescale_table( table );
}


/******************** GET SYMBOL SCALE ******************/
/*
 * This routine is called when decoding an arithmetic number.  In
 * order to decode the present symbol, the current scale in the
 * model must be determined.  This requires looking up the current
 * table, then building the totals table.  Once that is done, the
 * cumulative total table has the symbol scale at element 0.
 */

void ARITHN_get_symbol_scale( SYMBOL *s )
{
register CONTEXT *table;

    table = gContexts[ current_order ];
    ARITHN_totalize_table( table );
    s->scale = totals[ 0 ];
}


/***************** CONVERT SYMBOL TO INT ***************/
/*
 * This routine is called during decoding.  It is given a count that
 * came out of the arithmetic decoder, and has to find the symbol that
 * matches the count.  The cumulative totals are already stored in the
 * totals[] table, form the call to ARITHN_get_symbol_scale, so this routine
 * just has to look through that table.  Once the match is found,
 * the appropriate character is returned to the caller.  Two possible
 * complications.  First, the character might be the ESCAPE character,
 * in which case the current_order has to be decremented.  The other
 * complication is that the order might be -2, in which case we return
 * the negative of the symbol so it isn't confused with a normal
 * symbol.
 */

int ARITHN_convert_symbol_to_int( int count, SYMBOL *s )
{
register int c;
register CONTEXT *table;

    table = gContexts[ current_order ];
    for ( c = 0; count < totals[ c ] ; c++ )
        ;
    s->high_count = totals[ c - 1 ];
    s->low_count = totals[ c ];
    if ( c == 1 ) {
        current_order--;
        return( ESCAPE );
    }
    if ( current_order < -1 )
        return( (int) -table->stats[ c-2 ].symbol );
    else
        return( table->stats[ c-2 ].symbol );
}


/******************* ADD CHARACTER TO MODEL *****************/

void ARITHN_add_character_to_model(int c )
{
register int i;

    if ( gMaxOrder < 0 || c < 0 )
       return;
    gContexts[ gMaxOrder ] =
       ARITHN_shift_to_next_context( gContexts[ gMaxOrder ], c, gMaxOrder );
    for ( i = gMaxOrder-1 ; i > 0 ; i-- )
        gContexts[ i ] = gContexts[ i+1 ]->lesser_context;
}


/******************* SHIFT TO NEXT CONTEXT *******************/

CONTEXT *ARITHN_shift_to_next_context(CONTEXT *table, int c, int order )
{
register int i;
register CONTEXT *new_lesser;

    table = table->lesser_context;
    if ( order == 0 )
        return( table->links[ 0 ].next );
    for ( i = 0 ; i <= table->max_index ; i++ )
        if ( table->stats[ i ].symbol == (unsigned char) c )
            if ( table->links[ i ].next != NULL )
                return( table->links[ i ].next );
            else
                break;

    new_lesser = ARITHN_shift_to_next_context( table, c, order-1 );

    table = ARITHN_allocate_next_order_table( table, c, new_lesser );
    return( table );
}


/******************* RESCALE TABLE ********************/

void ARITHN_rescale_table(CONTEXT *table )
{
register int i;

    if ( table->max_index == -1 )
        return;
    for ( i = 0 ; i <= table->max_index ; i++ )
        table->stats[ i ].counts /= 2;
    if ( table->stats[ table->max_index ].counts == 0 &&
         table->links == NULL ) {
        while ( table->stats[ table->max_index ].counts == 0 &&
                table->max_index >= 0 )
            table->max_index--;
        if ( table->max_index == -1 ) {
            free( (char *) table->stats );
            table->stats = NULL;
        } else {
            table->stats = (STATS *)
                realloc( (char *) table->stats,
                                 sizeof( STATS ) * ( table->max_index + 1 ) );
            if ( table->stats == NULL )
                DoFatalAlert( "\pError #11: reallocating stats space!" );
        }
    }
}


/******************* TOTALIZE TABLE ******************/

void ARITHN_totalize_table(CONTEXT *table )
{
register int i;
register unsigned char max;

    for ( ; ; ) {
        max = 0;
        i = table->max_index + 2;
        totals[ i ] = 0;
        for ( ; i > 1 ; i-- ) {
            totals[ i-1 ] = totals[ i ];
            if ( table->stats[ i-2 ].counts )
                if ( ( current_order == -2 ) ||
                     scoreboard[ table->stats[ i-2 ].symbol ] == 0 )
                     totals[ i-1 ] += table->stats[ i-2 ].counts;
            if ( table->stats[ i-2 ].counts > max )
                max = table->stats[ i-2 ].counts;
        }

        if ( max == 0 )
            totals[ 0 ] = 1;
        else {
            totals[ 0 ] = (short int) ( 256 - table->max_index );
            totals[ 0 ] *= table->max_index;
            totals[ 0 ] /= 256;
            totals[ 0 ] /= max;
            totals[ 0 ]++;
            totals[ 0 ] += totals[ 1 ];
        }
        if ( totals[ 0 ] < MAXIMUM_SCALE )
            break;
        ARITHN_rescale_table( table );
    }
    for ( i = 0 ; i < table->max_index ; i++ )
	if (table->stats[i].counts != 0)
            scoreboard[ table->stats[ i ].symbol ] = 1;
}

/**************** RECURSIVE FLUSH ******************/

void ARITHN_recursive_flush(CONTEXT *table )
{
register int i;

    if ( table->links != NULL )
        for ( i = 0 ; i <= table->max_index ; i++ )
            if ( table->links[ i ].next != NULL )
                ARITHN_recursive_flush( table->links[ i ].next );
    ARITHN_rescale_table( table );
}


/*********************** FLUSH MODEL *********************/

void ARITHN_flush_model()
{
//    putc( 'F', stdout );
    ARITHN_recursive_flush( gContexts[ 0 ] );
}



/******************* GET CURRENT COUNT *********************/

short int ARITHN_get_current_count(SYMBOL *s )
{
register long range;
register short int count;

    range = (long) ( high - low ) + 1;
    count = (short int)
            ((((long) ( code - low ) + 1 ) * s->scale-1 ) / range );
    return( count );
}


/***************** INITIALIZE ARITHMETIC DECODER *********************/

void ARITHN_initialize_arithmetic_decoder(BIT_FILE *stream )
{
register int i;

    code = 0;
    for ( i = 0 ; i < 16 ; i++ ) {
        code <<= 1;
        code += InputBit( stream );
    }
    low = 0;
    high = 0xffff;
}


/********************* REMOVE SYMBOL FROM STREAM **********************/

void ARITHN_remove_symbol_from_stream(BIT_FILE *stream,SYMBOL *s )
{
register    long range;

    range = (long)( high - low ) + 1;
    high = low + (unsigned short int)
                 (( range * s->high_count ) / s->scale - 1 );
    low = low + (unsigned short int)
                 (( range * s->low_count ) / s->scale );
    for ( ; ; ) {
        if ( ( high & 0x8000 ) == ( low & 0x8000 ) ) {
        }
        else if ((low & 0x4000) == 0x4000  && (high & 0x4000) == 0 ) {
            code ^= 0x4000;
            low   &= 0x3fff;
            high  |= 0x4000;
        } else

            return;
        low <<= 1;
        high <<= 1;
        high |= 1;
        code <<= 1;
        code += InputBit( stream );
    }
}

