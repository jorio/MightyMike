//
// IO.h
//

#define	FONT_WIDTH		20

enum
{
	DEMO_MODE_OFF,
	DEMO_MODE_RECORD,
	DEMO_MODE_PLAYBACK
};

enum
{
	CONTROL_MODE_KEYBOARD,
	CONTROL_MODE_MOUSE
};


extern void	Home(void);
extern void	DoCR(void);
extern void	PrintChar(char);
extern void	StartRecordingDemo(void);
extern void	SaveDemoData(void);
extern void	InitDemoPlayback(void);
extern void	ReadKeyboard(void);
extern void	StopDemo(void);
extern Boolean	GetKeyState(unsigned short);
extern Boolean	GetKeyState2(unsigned short);
extern char	CheckKey(void);
extern void	PrintNum(long, short, short, short);
extern void	PrintNum2(long, short, short, short);
extern void	PrintNumAt(long, short, short, short, Ptr, long);
extern void	PrintBigNum(long, short);
extern void	WriteLn(char *);
extern void	PrintBigChar(char);
extern void	WaitKeyUp(Byte);
extern Boolean	CheckNewKeyDown(Byte, Byte, Boolean *);
extern Boolean	CheckNewKeyDown2(Byte, Boolean *);
extern Byte	ASCIIToBigFont(char);
