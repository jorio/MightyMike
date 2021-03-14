//
// CINEMA.H
//

							// BONUS OBJECT TYPE LIST
							//================================

#define	GROUP_BONUS		GROUP_AREA_SPECIFIC
enum
{
	ObjType_BonusCursor,
	ObjType_BonusBunny
};

enum
{
	GroupNum_BonusCursor = GROUP_BONUS,
	GroupNum_BonusBunny = GROUP_MAIN
};


							// TITLE OBJECT TYPE LIST
							//================================

enum
{
	ObjType_View,
	ObjType_TitleIcons,
	ObjType_MeTitle
};

enum
{
	GroupNum_View = GROUP_MAIN,
	GroupNum_TitleIcons = GROUP_MAIN,
	GroupNum_MeTitle = GROUP_MAIN
};


							// PLAYERCHOOSE OBJECT TYPE LIST
							//================================

enum
{
	ObjType_PCBattery,
	ObjType_SaveGame
};

enum
{
	GroupNum_PCBattery = GROUP_OVERHEAD,
	GroupNum_SaveGame = GROUP_OVERHEAD
};

							// VIEW OBJECT TYPE LIST
							//================================

enum
{
	ObjType_ViewType,
	ObjType_ViewBorder,
	ObjType_Head
};

enum
{
	GroupNum_ViewType = GROUP_MAIN,
	GroupNum_ViewBorder = GROUP_MAIN,
	GroupNum_Head = GROUP_MAIN
};


							// OVERHEAD OBJECT TYPE LIST
							//================================

enum
{
	ObjType_OverheadIcons,
	ObjType_OHMPlayerSignal
};

enum
{
	GroupNum_OverheadIcons = GROUP_OVERHEAD,
	GroupNum_OHMPlayerSignal = GROUP_OVERHEAD
};


							// DIFFICULTY OBJECT TYPE LIST
							//================================

enum
{
	ObjType_DifficultyHead,
	ObjType_DifficultyCursor
};

enum
{
	GroupNum_DifficultyHead = GROUP_MAIN,
	GroupNum_DifficultyCursor = GROUP_MAIN
};



							// WIN OBJECT TYPE LIST
							//================================

enum
{
	ObjType_Confetti
};

enum
{
	GroupNum_Confetti = GROUP_WIN
};


extern void	DoTitleScreen(void);
extern void	MoveTitleCursor(void);
extern void	DoSceneScreen(void);
extern void	ClearHighScores(void);
extern void	ShowHighScores(void);
extern void	DisplayScores(void);
extern void	AddHighScore(long);
extern void	ShowLastScore(void);
extern void	DoEnterName(short);
extern void	SaveHighScores(void);
extern void	LoadHighScores(void);
extern void	DoPangeaLogo(void);
void DoLegal(void);
extern void	ChoosePlayerMode(void);
extern void	DoCredits(void);
extern void	MoveCreditLetter(void);
extern void	DoOverheadMap(void);
extern void	DoLoseScreen(void);
extern void	DoWinScreen(void);
extern void	MoveConfetti(void);
extern void	DoHeadScreen(void);
extern void	MoveHead(void);
extern void	ShowBonusScreen(void);
extern void	PrintBonusNum(long, short, short, short);
void DoSettingsScreen(void);
