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


extern void	PlaySpinFile(short);
extern void	PreLoadSpinFile(Str255, long);
extern Boolean	ContinueSpinLoad(void);
extern void	GetSpinHeader(void);
extern void	GetSpinPalette(void);
extern void	DoSpinFrame(void);
extern void	DrawSpinFrame(Ptr);
extern void	DrawSpinFrame_Double(Ptr);
extern void	RegulateSpinSpeed(long);
