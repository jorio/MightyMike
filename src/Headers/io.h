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


extern void	StartRecordingDemo(void);
extern void	SaveDemoData(void);
extern void	InitDemoPlayback(void);
extern void	ReadKeyboard(void);
extern void	StopDemo(void);
extern void	PrintNum(long, short, short, short);
extern void	PrintBigNum(long, short);
extern void	WriteLn(char *);
extern void	PrintBigChar(char);
extern Byte	ASCIIToBigFont(char);
