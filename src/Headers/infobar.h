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



extern void	InitHealth(void);
extern void	ShowHealth(void);
extern void	GiveMeHealth(void);
extern void	UpdateInfoBar(void);
extern void	InitScore(void);
extern void	ShowScore(void);
extern void	GetPoints(long);
extern void	GetFreeDude(void);
extern void	InitFreeLives(void);
extern void	ShowLives(void);
extern void	ShowWeaponIcon(void);
extern void	EraseWeaponIcon(void);
extern void	ShowWeaponLife(void);
extern void	InitCoins(void);
extern void	ShowCoins(void);
extern void	GetCoins(short);
extern void	ShowNumBunnies(void);
extern void	InitKeys(void);
extern void	ShowKeys(void);
extern void	ShowPaused(void);
extern Boolean	AskIfQuit(void);
