//
// Sound2.h
//

#define		BASE_EFFECT_RESOURCE	10000
#define		NUM_DEFAULT_EFFECTS		32
#define		MAX_EFFECTS				(NUM_DEFAULT_EFFECTS+15)


//====================== SOUNDS =================================

enum
{
	SOUND_POP,
	SOUND_COINS,
	SOUND_CRUNCH,
	SOUND_SQUEEK,
	SOUND_ILLSAVE,
	SOUND_COMEHERERODENT,
	SOUND_RADAR,
	SOUND_TAKETHAT,
	SOUND_EATMYDUST,
	SOUND_SELECTCHIME,
	SOUND_BADHIT,
	SOUND_DEATHSCREAM,
	SOUND_RUBBERGUN,
	SOUND_HEATSEEK,
	SOUND_PIESQUISH,
	SOUND_SUCKPOP,
	SOUND_MISSLELAUNCH,
	SOUND_RIFLESHOT,
	SOUND_TRACERSHOT,
	SOUND_MACHINEGUN,
	SOUND_HEALTHDING,
	SOUND_FOOD,
	SOUND_GETWEAPON,
	SOUND_NUKE,
	SOUND_MIKEHURT,
	SOUND_GETPOW,
	SOUND_PIXIEDUST,
	SOUND_SPLASH,
	SOUND_FREEDUDE,
	SOUND_GETKEY,
	SOUND_NICEGUY,
	SOUND_FIREHOLE
};

//===================== SONGS =============================

enum{
	SONG_ID_JURASSIC		=	11000,
	SONG_ID_CARNIVAL		=	11001,
	SONG_ID_CANDY			=	11002,
	SONG_ID_FAIRY			=	11003,
	SONG_ID_BARGAIN 		=	11004,
	SONG_ID_RACE 			=	11005,
	SONG_ID_CANDY_INTRO 	= 	11006,
	SONG_ID_CLOWN_INTRO		=	11007,
	SONG_ID_WORLD_INTRO		=	11008,
	SONG_ID_FAIRY_INTRO		=	11009,
	SONG_ID_BARGAIN_INTRO	=	11010,
	SONG_ID_PANGEA			=	11011,
	SONG_ID_JURASSIC_INTRO	=	11012,
	SONG_ID_LOSEGAME		=	11013,
	SONG_ID_TITLE			=	11014,
	SONG_ID_WINGAME			=	11015,
	SONG_ID_WINLEVEL		=	11016,
	SONG_ID_WINGAMELOOP		=	11017,
	SONG_ID_WINHUM			=	11018
};


//===================== PROTOTYPES ===================================


extern void	InitSoundTools(void);
extern void	LoadDefaultSounds(void);
extern void	UnlockDefaultSounds(void);
extern void	OptimizeDefaultSounds(void);
extern void	StartMusic(void);
extern void	StopMusic(void);
extern void	StopAChannel(short);
extern void	StopAllSound(void);
extern void	PlaySong(short);
extern void	KillSong(void);
extern short	PlaySound(short);
extern void	SetVolume(void);
extern void	ToggleMusic(void);
extern void	ToggleEffects(void);
void DoSoundMaintenance(Boolean checkKeys);
extern short	AddEffect(Str255, short);
extern void	ZapAllAddedSounds(void);
extern void	PlayAreaMusic(void);
extern void	LoadAreaSound(void);
extern Boolean	IsMusicPlaying(void);
