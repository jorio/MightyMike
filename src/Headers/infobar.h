//
// Infobar.h
//

#define	MAX_HEARTS			8				// this is max hits

						// INFOBAR OBJECT TYPE LIST
						//======================================
enum
{
	ObjType_ScoreNumbers,
	ObjType_WeaponIcon,
	ObjType_HealthHearts,
	ObjType_KeyCover,
	ObjType_Paused,
	ObjType_Quit
};

enum
{
	GroupNum_ScoreNumbers = GROUP_INFOBAR,
	GroupNum_WeaponIcon = GROUP_INFOBAR,
	GroupNum_HealthHearts = GROUP_INFOBAR,
	GroupNum_KeyCover = GROUP_INFOBAR,
	GroupNum_Paused = GROUP_INFOBAR,
	GroupNum_Quit = GROUP_INFOBAR
};



void	InitHealth(void);
void	ShowHealth(void);
void	GiveMeHealth(void);
void	UpdateInfoBar(void);
void	InitScore(void);
void	ShowScore(void);
void	GetPoints(long);
void	GetFreeDude(void);
void	InitFreeLives(void);
void	ShowLives(void);
void	ShowWeaponIcon(void);
void	EraseWeaponIcon(void);
void	ShowWeaponLife(void);
void	InitCoins(void);
void	ShowCoins(void);
void	GetCoins(short);
void	ShowNumBunnies(void);
void	InitKeys(void);
void	ShowKeys(void);
void	ShowPaused(void);
Boolean	AskIfQuit(void);
