//
// LZW.h
//


extern void	LZW_InitializeDictionary(void);
extern void	LZW_InitializeStorage(void);
extern void	LZW_ZapStorage(void);
extern void	LZW_Expand(short, unsigned char *, long);
extern unsigned short	LZW_decode_string(unsigned short, unsigned short);
