//
// ARITH-N.h
//

/*
 * The SYMBOL structure is what is used to define a symbol in
 * arithmetic coding terms.  A symbol is defined as a range between
 * 0 and 1.  Since we are using integer math, instead of using 0 and 1
 * as our end points, we have an integer scale.  The low_count and
 * high_count define where the symbol falls in the range.
 */

typedef struct {
    unsigned short  low_count;
    unsigned short  high_count;
    unsigned short  scale;
} SYMBOL;

/* A context table contains a list of the counts for all symbols
 * that have been seen in the defined context.  For example, a
 * context of "Zor" might have only had 2 different characters
 * appear.  't' might have appeared 10 times, and 'l' might have
 * appeared once.  These two counts are stored in the context
 * table.  The counts are stored in the STATS structure.  All of
 * the counts for a given context are stored in and array of STATS.
 * As new characters are added to a particular contexts, the STATS
 * array will grow.  Sometimes, the STATS array will shrink
 * after flushing the model.
 */

typedef struct {
    unsigned char symbol;
    unsigned char counts;
} STATS;

/*
 * Each context has to have links to higher order contexts.  These
 * links are used to navigate through the context tables.  For example,
 * to find the context table for "ABC", I start at the order 0 table,
 * then find the pointer to the "A" context table by looking through
 * the LINKS array.  At that table, we find the "B" link and go to
 * that table.  The process continues until the destination table is
 * found.  The table pointed to by the LINKS array corresponds to the
 * symbol found at the same offset in the STATS table.  The reason that
 * LINKS is in a separate structure instead of being combined with
 * STATS is to save space.  All of the leaf context nodes don't need
 * next pointers, since they are in the highest order context.  In the
 * leaf nodes, the LINKS array is a NULL pointers.
 */

typedef struct {
    struct context *next;
} LINKS;

/*
 * The CONTEXT structure holds all of the known information about
 * a particular context.  The links and stats pointers are discussed
 * immediately above here.  The max_index element gives the maximum
 * index that can be applied to the stats or link array.  When the
 * table is first created, and stats is set to NULL, max_index is set
 * to -1.  As soon as single element is added to stats, max_index is
 * incremented to 0.
 *
 * The lesser context pointer is a navigational aid.  It points to
 * the context that is one less than the current order.  For example,
 * if the current context is "ABC", the lesser_context pointer will
 * point to "BC".  The reason for maintaining this pointer is that
 * this particular bit of table searching is done frequently, but
 * the pointer only needs to be built once, when the context is
 * created.
 */

typedef struct context {
    short max_index;
    LINKS *links;
    STATS *stats;
    struct context *lesser_context;
} CONTEXT;


extern void	ARITHN_Expand(short, Ptr, long);
extern void	ARITHN_initialize_model(void);
extern CONTEXT	*ARITHN_allocate_next_order_table(CONTEXT *, short, CONTEXT *);
extern void	ARITHN_update_model(short);
extern void	ARITHN_update_table(CONTEXT *, short);
extern void	ARITHN_get_symbol_scale(SYMBOL *);
extern short	ARITHN_convert_symbol_to_int(short, SYMBOL *);
extern void	ARITHN_add_character_to_model(short);
extern CONTEXT	*ARITHN_shift_to_next_context(CONTEXT *, short, short);
extern void	ARITHN_rescale_table(CONTEXT *);
extern void	ARITHN_totalize_table(CONTEXT *);
extern void	ARITHN_recursive_flush(CONTEXT *);
extern void	ARITHN_flush_model(void);
extern void	ARITHN_initialize_arithmetic_encoder(void);
extern short ARITHN_get_current_count(SYMBOL *);
extern void	ARITHN_initialize_arithmetic_decoder(BIT_FILE *);
extern void	ARITHN_remove_symbol_from_stream(BIT_FILE *, SYMBOL *);
