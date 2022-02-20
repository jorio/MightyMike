/****************************/
/*     SOUND ROUTINES       */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/
#include "myglobals.h"
#include "misc.h"
#include "sound2.h"
#include "io.h"
#include "input.h"
#include "externs.h"
#include <stdio.h>  // snprintf

/****************************/
/*    CONSTANTS             */
/****************************/

#define		DEFAULT_VOLUME			100				// default volume of channels
#define		MAX_CHANNELS			6

static const char* kSongNames[SONG_ID_MAX] =
{
	[SONG_ID_JURASSIC]		= "PrehistoricPlaza",
	[SONG_ID_CARNIVAL]		= "ClowningAround",
	[SONG_ID_CANDY]			= "CandyCaneLane",
	[SONG_ID_FAIRY]			= "FairyTaleTrail",
	[SONG_ID_BARGAIN]		= "GamesGallery",
	[SONG_ID_RACE]			= "CarShopCartRace",
	[SONG_ID_CANDY_INTRO]	= "IntroToCandyCane",
	[SONG_ID_CLOWN_INTRO]	= "IntroToClowning",
	[SONG_ID_WORLD_INTRO]	= "IntroToEnteringWorlds",
	[SONG_ID_FAIRY_INTRO]	= "IntroToFairyTale",
	[SONG_ID_BARGAIN_INTRO]	= "IntroToGamesGallery",
	[SONG_ID_PANGEA]		= "PangeaIntro",
	[SONG_ID_JURASSIC_INTRO]= "IntroToPrehistoric",
	[SONG_ID_LOSEGAME]		= "LoseGame",
	[SONG_ID_TITLE]			= "MainTitleTheme",
	[SONG_ID_WINGAME]		= "WinGame",
	[SONG_ID_WINLEVEL]		= "MikeFinishLevel",
	[SONG_ID_WINGAMELOOP]	= "WinGameLoop",
	[SONG_ID_WINHUM]		= "WinHum",
};

static const char* kEffectNames[NUM_DEFAULT_EFFECTS] =
{
	[SOUND_POP]				= "Pop",
	[SOUND_COINS]			= "GetCoin",
	[SOUND_CRUNCH]			= "EnemyExplode",
	[SOUND_SQUEEK]			= "Squeek",
	[SOUND_ILLSAVE]			= "IllSaveYou",
	[SOUND_COMEHERERODENT]	= "ComeHereRodent",
	[SOUND_RADAR]			= "RadarEnter",
	[SOUND_TAKETHAT]		= "TakeThat",
	[SOUND_EATMYDUST]		= "EatMyDust",
	[SOUND_SELECTCHIME]		= "SelectChime",
	[SOUND_BADHIT]			= "BadHit",
	[SOUND_DEATHSCREAM]		= "DeathScream",
	[SOUND_RUBBERGUN]		= "RubberGun",
	[SOUND_HEATSEEK]		= "HeatSeekBeew",
	[SOUND_PIESQUISH]		= "Pie",
	[SOUND_SUCKPOP]			= "SuctionCupPop",
	[SOUND_MISSLELAUNCH]	= "MissleLaunch",
	[SOUND_RIFLESHOT]		= "RifleShot",
	[SOUND_TRACERSHOT]		= "TracerShot",
	[SOUND_MACHINEGUN]		= "MachineGun",
	[SOUND_HEALTHDING]		= "Heart",
	[SOUND_FOOD]			= "Food",
	[SOUND_GETWEAPON]		= "WeaponPickup",
	[SOUND_NUKE]			= "Nuke",
	[SOUND_MIKEHURT]		= "Ouch",
	[SOUND_GETPOW]			= "GetPOW",
	[SOUND_PIXIEDUST]		= "PixieDust",
	[SOUND_SPLASH]			= "Splash",
	[SOUND_FREEDUDE]		= "FreeDude",
	[SOUND_GETKEY]			= "GetKey",
	[SOUND_NICEGUY]			= "NoMoreNiceGuy",
	[SOUND_FIREHOLE]		= "FireInTheHole",
};


/**********************/
/*     VARIABLES      */
/**********************/

static	SndListHandle	EffectHandles[MAX_EFFECTS];							// handles to ALL sounds, default AND added
static	SndListHandle	SoundHand_Music = nil;

static	SndChannelPtr	gSndChannel[MAX_CHANNELS];

static  short			gSndLastEffectInChannel[MAX_CHANNELS];
static  short			gSndEffectLastPlayedInChannel[MAX_EFFECTS];

static	short			gMaxChannels;

static	Boolean			gEffectsOnFlag = true;

static	unsigned char	gVolume = DEFAULT_VOLUME;

static	short			gNumEffectsLoaded;

														// ADDED SOUND NUMS

short			gSoundNum_UngaBunga,gSoundNum_DinoBoom,gSoundNum_DoorOpen,
				gSoundNum_BarneyJump,gSoundNum_DogRoar;
short			gSoundNum_ChocoBunny,gSoundNum_Carmel,gSoundNum_GummyHaha;
short			gSoundNum_JackInTheBox;
short			gSoundNum_WitchHaha,gSoundNum_Skid,gSoundNum_Shriek;
short			gSoundNum_Ship,gSoundNum_ExitShip,gSoundNum_Frog,gSoundNum_RobotDanger;
short			gSoundNum_ClownLaugh;

static	Boolean			gSongPlayingFlag = false;

/********************* INIT SOUND TOOLS ********************/

static long GetSoundChannelInitializationParameters(void)
{
	long initBits = initMono | initNoDrop;

	if (!gGamePrefs.interpolateAudio)
		initBits |= initNoInterp;

	return initBits;
}

void InitSoundTools(void)
{
OSErr		iErr;

	gMaxChannels = 0;
	gNumEffectsLoaded = 0;

	memset(EffectHandles, 0, sizeof(EffectHandles));

	const long initBits = GetSoundChannelInitializationParameters();

	for (gMaxChannels = 0; gMaxChannels < MAX_CHANNELS; gMaxChannels++)
	{
						/* ALLOC CHANNEL */

		iErr = SndNewChannel(&gSndChannel[gMaxChannels], sampledSynth, initBits, nil);
		GAME_ASSERT(iErr == noErr);
	}

	for (int i = 0; i < MAX_EFFECTS; i++)
		gSndEffectLastPlayedInChannel[i] = -1;

	for (int i = 0; i < MAX_CHANNELS; i++)
		gSndLastEffectInChannel[i] = -1;

	LoadDefaultSounds();									// these are never deleted!

	SetVolume();											// make sure all channels set to current volume
}


void OnChangeAudioInterpolation(void)
{
	SndCommand cmd;
	cmd.cmd = reInitCmd;
	cmd.param1 = 0;
	cmd.param2 = GetSoundChannelInitializationParameters();

	for (int i = 0; i < gMaxChannels; i++)
	{
		SndDoImmediate(gSndChannel[i], &cmd);
	}
}



static SndListHandle LoadAIFF(const char* bankName, const char* effectName)
{
SndListHandle effectHandle;
char path[256];
FSSpec spec;
short refNum;
OSErr err;

	snprintf(path, sizeof(path), ":Audio:%s:%s.aiff", bankName, effectName);

	err = FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, path, &spec);
	GAME_ASSERT_MESSAGE(err == noErr, path);

	err = FSpOpenDF(&spec, fsRdPerm, &refNum);
	GAME_ASSERT_MESSAGE(err == noErr, path);

	effectHandle = Pomme_SndLoadFileAsResource(refNum);
	GAME_ASSERT_MESSAGE(effectHandle, path);

	FSClose(refNum);

				/* GET OFFSET INTO IT */

	long offset;
	GetSoundHeaderOffset(effectHandle, &offset);

				/* DECOMPRESS IT AHEAD OF TIME */

	Pomme_DecompressSoundResource(&effectHandle, &offset);

	return effectHandle;
}

/************************** LOAD DEFAULT SOUNDS ************************/
//
// Loads the standard default effect sounds
//

void LoadDefaultSounds(void)
{
	gNumEffectsLoaded = 0;

					/* LOAD ALL EFFECTS */

	for (int i = 0; i < NUM_DEFAULT_EFFECTS; i++)
	{
		EffectHandles[i] = LoadAIFF("Default", kEffectNames[i]);
		gNumEffectsLoaded++;
	}
}


/********************* START MUSIC **********************/
//
// NOTE: freqCmd must be used for the sample to use the loop points.
//

void StartMusic(void)
{
SndCommand 	mySndCmd;
long		offset;

	if (!gGamePrefs.music)								// see if music activated
		return;

	GetSoundHeaderOffset(SoundHand_Music, &offset);		// get offset to header

	mySndCmd.cmd = soundCmd;							// install sample in the channel
	mySndCmd.param1 = 0;
	mySndCmd.ptr = (Ptr)*SoundHand_Music+offset;		// pointer to SoundHeader
	SndDoImmediate(gSndChannel[0], &mySndCmd);

	mySndCmd.cmd = freqCmd;								// call this to START sound & keep looping
	mySndCmd.param1 = 0;
	mySndCmd.param2 = kMiddleC;							// MIDI freq
	SndDoImmediate(gSndChannel[0], &mySndCmd);

	gSongPlayingFlag = true;
}


/********************* STOP MUSIC **********************/

void StopMusic(void)
{
	StopAChannel(0);									// music is always on channel 0

	gSongPlayingFlag = false;
}


/********************* STOP A CHANNEL **********************/
//
// Stops the indicated sound channel from playing.
//

void StopAChannel(short channelNum)
{
SndCommand 	mySndCmd;
OSErr 		myErr;

	if ((channelNum < 0) || (channelNum >= gMaxChannels))		// make sure its a legal #
		return;

	mySndCmd.cmd = flushCmd;
	mySndCmd.param1 = 0;
	mySndCmd.param2 = 0;
	myErr = SndDoImmediate(gSndChannel[channelNum], &mySndCmd);
	if (myErr)
		ShowSystemErr(myErr);

	mySndCmd.cmd = quietCmd;
	mySndCmd.param1 = 0;
	mySndCmd.param2 = 0;
	myErr = SndDoImmediate(gSndChannel[channelNum], &mySndCmd);
	if (myErr)
		ShowSystemErr(myErr);

	gSndLastEffectInChannel[channelNum] = -1;
}


/********************* STOP ALL SOUND **********************/

void StopAllSound(void)
{
short		i;

	for (i=0; i < gMaxChannels; i++)
	{
		StopAChannel(i);
	}
}


/******************** PLAY SONG ***********************/

void PlaySong(short songID)
{
	KillSong();											// see if zap existing song

			/* RESOLVE SONG FILENAME */

	GAME_ASSERT(songID >= 0);
	GAME_ASSERT((size_t)songID < sizeof(kSongNames)/sizeof(kSongNames[0]));

	const char* songName = kSongNames[songID];

			/* LOAD MUSIC FILE */

	SoundHand_Music = LoadAIFF("Music", songName);

			/* GET IT GOING */

	StartMusic();
}


/*********************** KILL SONG *********************/

void KillSong(void)
{
	if (SoundHand_Music != nil)							// see if zap existing song
	{
		StopMusic();
		DisposeHandle((Handle) SoundHand_Music);
		SoundHand_Music = nil;
	}
}


/***************************** PLAY SOUND ***************************/
//
// Plays a single shot (presumably) sound.
//
// OUTPUT: channel # used to play sound
//

short PlaySound(short soundNum)
{
static	SndCommand 		mySndCmd;
static	SndChannelPtr	chanPtr;
short					theChan;
SCStatus				theStatus;
long	offset;
OSErr	myErr;

	if (!gEffectsOnFlag)								// see if effects activated
		return(-1);

	GAME_ASSERT_MESSAGE(soundNum < gNumEffectsLoaded, "Illegal sound number!");		// see if illegal sound #

			/* DON'T PLAY EFFECT MULTIPLE TIMES AT ONCE */
			// (Source port addition)

	theChan = gSndEffectLastPlayedInChannel[soundNum];
	if (theChan != -1 && gSndLastEffectInChannel[theChan] == soundNum)
	{
		myErr = SndChannelStatus(gSndChannel[theChan], sizeof(SCStatus), &theStatus);
		if (myErr == noErr && theStatus.scChannelBusy)
		{
			StopAChannel(theChan);
		}
		goto got_chan;
	}


			/* FIND A FREE CHANNEL */

	for (theChan = 1; theChan < gMaxChannels; theChan++)	// start at 1; channel 0 is reserved for music
	{
		myErr = SndChannelStatus(gSndChannel[theChan],sizeof(SCStatus),&theStatus);	// get channel info
		if (myErr)
			ShowSystemErr(myErr);
		if (!theStatus.scChannelBusy)					// see if channel not busy
			goto got_chan;
	}

						/********************/
						/* NO FREE CHANNELS */
						/********************/

	return(-1);

got_chan:
	gSndEffectLastPlayedInChannel[soundNum] = theChan;
	gSndLastEffectInChannel[theChan] = soundNum;

					/* GET IT GOING */

	chanPtr = gSndChannel[theChan];

	GetSoundHeaderOffset((SndListHandle)EffectHandles[soundNum],&offset);	// get offset to header

	mySndCmd.cmd = flushCmd;
	mySndCmd.param1 = 0;
	mySndCmd.param2 = 0;
	myErr = SndDoImmediate(chanPtr, &mySndCmd);
	if (myErr)
		ShowSystemErr(myErr);

	mySndCmd.cmd = quietCmd;
	mySndCmd.param1 = 0;
	mySndCmd.param2 = 0;
	myErr = SndDoImmediate(chanPtr, &mySndCmd);
	if (myErr)
		ShowSystemErr(myErr);

	mySndCmd.cmd = soundCmd;								// install sample in the channel
	mySndCmd.param1 = 0;
	mySndCmd.ptr = (Ptr)*EffectHandles[soundNum]+offset;	// pointer to SoundHeader
	myErr = SndDoImmediate(chanPtr, &mySndCmd);
	if (myErr)
		ShowSystemErr(myErr);

	mySndCmd.cmd = freqCmd;								// call this to START sound & keep looping
	mySndCmd.param1 = 0;
	mySndCmd.param2 = kMiddleC;							// MIDI freq
	myErr = SndDoImmediate(chanPtr, &mySndCmd);
	if (myErr)
		ShowSystemErr(myErr);

	mySndCmd.cmd = ampCmd;
	mySndCmd.param1 = gVolume;
	mySndCmd.param2 = 0;
	myErr = SndDoImmediate(chanPtr, &mySndCmd);
	if (myErr)
		ShowSystemErr(myErr);

	return(theChan);									// return channel #
}



/********************* SET VOLUME *******************/

void SetVolume(void)
{
short	i;
SndChannelPtr	chanPtr;
SndCommand		mySndCmd;

	for (i=0; i<gMaxChannels; i++)
	{
		chanPtr = gSndChannel[i];

		mySndCmd.cmd = ampCmd;
		mySndCmd.param1 = gVolume;
		mySndCmd.param2 = 0;
		SndDoImmediate(chanPtr, &mySndCmd);
	}
}


/******************** TOGGLE MUSIC CALLBACK *********************/

void OnToggleMusic(void)
{
	if (gGamePrefs.music)
		StartMusic();
	else
		StopMusic();
}


/******************** TOGGLE MUSIC *********************/

void ToggleMusic(void)
{
	gGamePrefs.music = !gGamePrefs.music;
	OnToggleMusic();
}


/********************* TOGGLE EFFECTS *****************/

void ToggleEffects(void)
{
	gEffectsOnFlag = gEffectsOnFlag^1;
}


/******************** DO SOUND MAINTENANCE *************/
//
// 		ReadKeyboard() must have already been called
//

void DoSoundMaintenance(Boolean checkKeys)
{
	if (!checkKeys)
		return;

			/* SEE IF TOGGLE MUSIC */

	if (GetNewNeedState(kNeed_ToggleMusic))
		ToggleMusic();
}



/******************* ADD EFFECT *******************/


static short AddEffect(const char* bankName, const char* effectName)
{
	short effectID = gNumEffectsLoaded;

	EffectHandles[effectID] = LoadAIFF(bankName, effectName);

	gNumEffectsLoaded++;

	return effectID;
}


/****************** ZAP ALL ADDED SOUNDS ******************/
//
// Zaps all of the added sounds
//

void ZapAllAddedSounds(void)
{
	StopAllSound();

	for (int i = NUM_DEFAULT_EFFECTS; i < gNumEffectsLoaded; i++)
	{
		if (EffectHandles[i])
		{
			DisposeHandle((Handle) EffectHandles[i]);
			EffectHandles[i] = nil;
		}
	}

	gNumEffectsLoaded = NUM_DEFAULT_EFFECTS; 		// reset this to default value
}

void ZapAllSounds(void)
{
	StopAllSound();

	for (int i = 0; i < gNumEffectsLoaded; i++)
	{
		if (EffectHandles[i])
		{
			DisposeHandle((Handle) EffectHandles[i]);
			EffectHandles[i] = nil;
		}
	}

	gNumEffectsLoaded = 0;
}

/*************** PLAY AREA MUSIC ****************/

void PlayAreaMusic(void)
{
	switch(gSceneNum)
	{
		case	SCENE_JURASSIC:
				PlaySong(SONG_ID_JURASSIC);
				break;

		case	SCENE_CANDY:
				PlaySong(SONG_ID_CANDY);
				break;

		case	SCENE_CLOWN:
				PlaySong(SONG_ID_CARNIVAL);
				break;

		case	SCENE_FAIRY:
				PlaySong(SONG_ID_FAIRY);
				break;

		case	SCENE_BARGAIN:
				PlaySong(SONG_ID_BARGAIN);
				break;
	}
}

/*************** LOAD AREA SOUND ****************/

void LoadAreaSound(void)
{
	switch(gSceneNum)
	{
		case	SCENE_JURASSIC:
				gSoundNum_UngaBunga		= AddEffect("jurassic", "ungabunga");
				gSoundNum_DinoBoom		= AddEffect("jurassic", "dinoboom");
				gSoundNum_BarneyJump	= AddEffect("jurassic", "barneybounce");
				gSoundNum_DoorOpen		= AddEffect("jurassic", "dooropen");
				break;

		case	SCENE_CANDY:
				gSoundNum_ChocoBunny	= AddEffect("candy", "bunnyhop");
				gSoundNum_Carmel		= AddEffect("candy", "carmelmonster");
				gSoundNum_GummyHaha		= AddEffect("candy", "hehehe");
				break;

		case	SCENE_CLOWN:
				gSoundNum_JackInTheBox	= AddEffect("clown", "jackinthebox");
				gSoundNum_Skid			= AddEffect("clown", "tireskid");
				gSoundNum_DoorOpen		= AddEffect("clown", "dooropen");
				gSoundNum_ClownLaugh	= AddEffect("clown", "clownlaugh");
				break;

		case	SCENE_FAIRY:
				gSoundNum_WitchHaha		= AddEffect("fairy", "witch");
				gSoundNum_Shriek		= AddEffect("fairy", "shriek");
				gSoundNum_DoorOpen		= AddEffect("fairy", "dooropen");
				gSoundNum_Frog			= AddEffect("fairy", "frog");
				gSoundNum_BarneyJump	= AddEffect("jurassic", "barneybounce");
				gSoundNum_DinoBoom		= AddEffect("jurassic", "dinoboom");
				break;

		case	SCENE_BARGAIN:
				gSoundNum_Ship			= AddEffect("bargain", "spaceship");
				gSoundNum_ExitShip		= AddEffect("bargain", "exitship");
				gSoundNum_DoorOpen		= AddEffect("bargain", "dooropen");
				gSoundNum_DogRoar		= AddEffect("bargain", "dogroar");
				gSoundNum_RobotDanger	= AddEffect("bargain", "robotdanger");
				break;
	}
}


/************************* IS MUSIC PLAYING *************************/
//
// Checks the music channel to see if it's still busy
//

Boolean	IsMusicPlaying(void)
{
SCStatus	theStatus;

	if (!gGamePrefs.music)					// if music is deactivated, then return true anyway to trick wait routines
		return(true);

	SndChannelStatus(gSndChannel[0],sizeof(SCStatus),&theStatus);	// get channel info
	return (theStatus.scChannelBusy);								// see if channel busy
}

