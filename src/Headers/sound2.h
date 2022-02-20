//
// Sound2.h
//

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
	SOUND_FIREHOLE,
	NUM_DEFAULT_EFFECTS,
	MAX_EFFECTS = NUM_DEFAULT_EFFECTS + 15
};

//===================== SONGS =============================

enum{
	SONG_ID_JURASSIC,
	SONG_ID_CARNIVAL,
	SONG_ID_CANDY,
	SONG_ID_FAIRY,
	SONG_ID_BARGAIN,
	SONG_ID_RACE,
	SONG_ID_CANDY_INTRO,
	SONG_ID_CLOWN_INTRO,
	SONG_ID_WORLD_INTRO,
	SONG_ID_FAIRY_INTRO,
	SONG_ID_BARGAIN_INTRO,
	SONG_ID_PANGEA,
	SONG_ID_JURASSIC_INTRO,
	SONG_ID_LOSEGAME,
	SONG_ID_TITLE,
	SONG_ID_WINGAME,
	SONG_ID_WINLEVEL,
	SONG_ID_WINGAMELOOP,
	SONG_ID_WINHUM,
	SONG_ID_MAX
};


//===================== PROTOTYPES ===================================


void	InitSoundTools(void);
void OnChangeAudioInterpolation(void);
void	LoadDefaultSounds(void);
void	StartMusic(void);
void	StopMusic(void);
void	StopAChannel(short);
void	StopAllSound(void);
void	PlaySong(short);
void	KillSong(void);
short	PlaySound(short);
void	SetVolume(void);
void	OnToggleMusic(void);
void	ToggleMusic(void);
void	ToggleEffects(void);
void DoSoundMaintenance(Boolean checkKeys);
void	ZapAllAddedSounds(void);
void ZapAllSounds(void);
void	PlayAreaMusic(void);
void	LoadAreaSound(void);
Boolean	IsMusicPlaying(void);
