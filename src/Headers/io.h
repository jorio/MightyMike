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


void	StartRecordingDemo(void);
void	SaveDemoData(void);
void	InitDemoPlayback(void);
void	ReadKeyboard(void);
void	StopDemo(void);
void	PrintNum(long, short, short, short);
void	PrintBigNum(long, short);
void	WriteLn(char *);
void	PrintBigChar(char);
Byte	ASCIIToBigFont(char);
