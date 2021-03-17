//
// main.h
//


void	ToolBoxInit(void);
void	InitGame(void);
void	InitArea(void);
void	LoadAreaArt(void);
void	PlayArea(void);
void	SwitchPlayer(void);
void	SaveCurrentPlayer(void);
void	LoadCurrentPlayer(Boolean minimal);
void	TryToSaveBothPlayers(short gameNum);
void	SaveGame(short	gameNum, Boolean atNextFlag);
void	LoadGame(short);
void	Do1PlayerGame(void);
void	Do2PlayerGame(void);
void	CleanMemory(void);
void	OptimizeMemory(void);
OSErr LoadPrefs(void);
void	SavePrefs(void);
void	main(void);
