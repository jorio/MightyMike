//
// main.h
//


extern void	ToolBoxInit(void);
extern void	InitGame(void);
extern void	InitArea(void);
extern void	LoadAreaArt(void);
extern void	PlayArea(void);
extern void	SwitchPlayer(void);
extern void	SaveCurrentPlayer(void);
extern	void LoadCurrentPlayer(Boolean minimal);
extern	void TryToSaveBothPlayers(short gameNum);
extern	void SaveGame(short	gameNum, Boolean atNextFlag);
extern void	LoadGame(short);
extern void	Do1PlayerGame(void);
extern void	Do2PlayerGame(void);
extern void	CleanMemory(void);
extern void	OptimizeMemory(void);
extern void	LoadPrefs(void);
extern void	SavePrefs(void);
extern void	main(void);
