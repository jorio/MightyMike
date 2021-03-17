//
// spin.h
//

enum
{
	SPIN_COMMAND_FRAMEDATA,
	SPIN_COMMAND_PALETTE,
	SPIN_COMMAND_STOP,
	SPIN_COMMAND_HEADER,
	SPIN_COMMAND_LOOP,
	SPIN_COMMAND_EXTENDEDHEADER
};

#define	DEFAULT_SPIN_PRELOAD_SIZE	1000000L


void	PlaySpinFile(short);
void PreLoadSpinFile(const char* filename, long preLoadSize);
Boolean	ContinueSpinLoad(void);
void	GetSpinHeader(void);
void	GetSpinPalette(void);
void	DoSpinFrame(void);
void	DrawSpinFrame(Ptr);
void	RegulateSpinSpeed(long);
