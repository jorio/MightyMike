
typedef struct bit_file {
    unsigned char *file;
    unsigned char mask;
    short			rack;
    long 		counter;
} BIT_FILE;


extern void	OutputBit(BIT_FILE *, short);
extern void	OutputBits(BIT_FILE *, unsigned long, short);
extern short	InputBit(BIT_FILE *);
extern unsigned long	InputBits(BIT_FILE *, short);
