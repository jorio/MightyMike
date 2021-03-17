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


void	DoTitleScreen(void);
void	MoveTitleCursor(void);
void	DoSceneScreen(void);
void	ClearHighScores(void);
void	ShowHighScores(void);
void	DisplayScores(void);
void	AddHighScore(long);
void	ShowLastScore(void);
void	DoEnterName(short);
void	SaveHighScores(void);
void	LoadHighScores(void);
void	DoPangeaLogo(void);
void DoLegal(void);
void	ChoosePlayerMode(void);
void	DoCredits(void);
void	MoveCreditLetter(void);
void	DoOverheadMap(void);
void	DoLoseScreen(void);
void	DoWinScreen(void);
void	MoveConfetti(void);
void	DoHeadScreen(void);
void	MoveHead(void);
void	ShowBonusScreen(void);
void	PrintBonusNum(long, short, short, short);
void DoSettingsScreen(void);
void ApplyPrefs(void);
