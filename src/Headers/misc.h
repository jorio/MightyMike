//
// MISC.H
//

#define MAX_GLOBAL_FLAGS		10
#define gGlobFlag_MeDoneDead	gGlobalFlagList[0]		// flag set when I'm done with death anim

#define		ONE_MINUTE_DELAY	(60L*1000L*1000L)					// delay in microseconds





extern void	ClearGlobalFlags(void);
extern void	ShowSystemErr(OSErr);
extern void	DoAlert(Str255);
extern void	DoFatalAlert(Str255);
extern void	DoFatalAlert2(Str255, Str255);
extern void	TellCopyToHD(void);
extern void	CleanQuit(void);
extern void	MyHideMenuBar(void);
extern void	MyShowMenuBar(void);
extern Boolean	Wait(long);
extern void	Wait2(long);
extern void	Wait3(long);
extern void	Wait4(long);
extern void	WaitWhileMusic(void);
extern void	CheckScreenDepth(void);
extern void	ResetScreen(void);
extern Handle	LoadPackedFile(Str255);
extern void	DecompressRLBFile(short, Ptr, long);
extern void	RLW_Expand(short, unsigned short *, long);
extern void	StartSystemTiming(void);
extern void	RegulateSpeed(long);
extern void	RegulateSpeed(long);
extern void	RegulateSpeed2(short);
extern unsigned short	RandomRange(unsigned short, unsigned short);
extern void	Decay(long *, unsigned long);
extern long	Absolute(long);
extern void	VerifySystem(void);
extern Handle	AllocHandle(long);
extern Ptr	AllocPtr(long);
extern void	InitThermometer(void);
extern void	FillThermometer(short);
extern void	OpenMikeFile(Str255, short *, Str255);
extern short	OpenMikeRezFile(Str255, Str255);
extern unsigned long	MyRandomLong(void);
extern unsigned short	MyRandomShort(void);
extern void	SetMyRandomSeed(unsigned long);
void GetDemoTimer(void);
void SaveDemoTimer(void);

