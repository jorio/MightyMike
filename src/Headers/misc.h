//
// MISC.H
//

#include <stdio.h>

#define MAX_GLOBAL_FLAGS		10
#define gGlobFlag_MeDoneDead	gGlobalFlagList[0]		// flag set when I'm done with death anim

#define		ONE_MINUTE_DELAY	(60L*1000L*1000L)					// delay in microseconds





extern void	ClearGlobalFlags(void);
extern void	ShowSystemErr(OSErr);
extern void	DoAlert(const char*);
void DoAssert(const char* msg, const char* file, int line);
extern void	DoFatalAlert(const char*);
extern void	DoFatalAlert2(const char*, const char*);
extern void	CleanQuit(void);
extern Boolean	Wait(long);
extern void	Wait2(long);
extern void	Wait3(long);
extern void	Wait4(long);
extern void	WaitWhileMusic(void);
extern Handle	LoadPackedFile(Str255);
extern void	DecompressRLBFile(short, Ptr, long);
extern void	RLW_Expand(short, unsigned short *, long);
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


static inline Boolean HandleBoundsCheck(Handle h, Ptr p)
{
	return p >= *h && p < *h + GetHandleSize(h);
}

#define TODO_REWRITE_THIS()		DoFatalAlert2("REWRITE THIS!", __func__)
#define TODO_REWRITE_THIS_MINOR()		printf("TODO: rewrite this! %s\n", __func__)


#define GAME_ASSERT(condition)											\
	do {																\
		if (!(condition))												\
			DoAssert(#condition, __func__, __LINE__);					\
	} while(0)

#define GAME_ASSERT_MESSAGE(condition, message)							\
	do {																\
		if (!(condition))												\
			DoAssert(message, __func__, __LINE__);						\
	} while(0)
