//
// MISC.H
//

#include <stdio.h>

#define gGlobFlag_MeDoneDead	gGlobalFlagList[0]		// flag set when I'm done with death anim

#define		ONE_MINUTE_DELAY	(60L*1000L*1000L)					// delay in microseconds

#if _MSC_VER
	#define _Static_assert static_assert
#endif

void	ClearGlobalFlags(void);
void	ShowSystemErr(OSErr);
void	DoAlert(const char*);
void DoAssert(const char* msg, const char* file, int line);
void	DoFatalAlert(const char*);
void	DoFatalAlert2(const char*, const char*);
void	CleanQuit(void);
Boolean	Wait(long);
void	Wait2(long);
void	Wait4(long);
void	WaitWhileMusic(void);
Handle	LoadRawFile(const char* file);
Handle	LoadPackedFile(const char* file);
void	DecompressRLBFile(short, Ptr, long);
void	RLW_Expand(short, unsigned short *, long);
void	RegulateSpeed(long);
void	RegulateSpeed2(short);
unsigned short	RandomRange(unsigned short, unsigned short);
void	Decay(long *, unsigned long);
long	Absolute(long);
void	VerifySystem(void);
void	InitThermometer(void);
void	FillThermometer(short);
short	OpenMikeFile(const char* filename);
short	OpenMikeRezFile(const char* filename);
unsigned long	MyRandomLong(void);
unsigned short	MyRandomShort(void);
void	SetMyRandomSeed(unsigned long);


static inline Boolean HandleBoundsCheck(Handle h, Ptr p)
{
	return p >= *h && p < *h + GetHandleSize(h);
}

static inline int32_t Fix32_Int(int32_t a)
{
	return a >> 16;
}

static inline int32_t Fix32_Mul(int32_t a, int32_t b)
{
	return (int32_t)(((int64_t)a * (int64_t)b) >> 16);
}

static inline int PositiveModulo(int value, unsigned int m)
{
	int mod = value % (int)m;
	if (mod < 0)
	{
		mod += m;
	}
	return mod;
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

#define CHECKED_DISPOSEPTR(p)											\
	do {																\
		if (p) {														\
			DisposePtr((Ptr) p);										\
			(p) = nil;													\
		}																\
	} while(0)

#define CHECKED_DISPOSEHANDLE(h)										\
	do {																\
		if (h) {														\
			DisposeHandle(h);											\
			(h) = nil;													\
		}																\
	} while(0)
