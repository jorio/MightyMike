/****************************/
/*     SOUND ROUTINES       */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/
#include "myglobals.h"
//#include <sound.h>
#include "misc.h"
#include "sound2.h"
#include "io.h"
#include "input.h"

extern	short		gMainAppRezFile;
extern	long		gTick,someLong,gFrames,gOriginalSystemVolumeFudge;
extern	Byte		gSceneNum;


/****************************/
/*    CONSTANTS             */
/****************************/

#define		DEFAULT_VOLUME			100				// default volume of channels
#define		MAX_CHANNELS			6

#define		SET_SYS_VOL		0


/**********************/
/*     VARIABLES      */
/**********************/

static	Handle			EffectHandles[MAX_EFFECTS];							// handles to ALL sounds, default AND added
static	Handle			AddedHandles[MAX_EFFECTS-NUM_DEFAULT_EFFECTS];		// handle to only sound which were added / not default
static	Handle			SoundHand_Music = nil;

static	SndChannelPtr	gSndChannel[MAX_CHANNELS];

static	short			gMaxChannels;

Boolean			gMusicOnFlag = true;
static	Boolean			gEffectsOnFlag = true;

static	Boolean			gMKeyDownFlag	=	false;
static	Boolean			gSKeyDownFlag	=	false;

static	unsigned char	gVolume = DEFAULT_VOLUME;

static	short			gNumEffectsLoaded,gNumAddedSounds;

														// ADDED SOUND NUMS

short			gSoundNum_UngaBunga,gSoundNum_DinoBoom,gSoundNum_DoorOpen,
				gSoundNum_BarneyJump,gSoundNum_DogRoar;
short			gSoundNum_ChocoBunny,gSoundNum_Carmel,gSoundNum_GummyHaha;
short			gSoundNum_JackInTheBox,gSoundNum_CarrotThrow;
short			gSoundNum_WitchHaha,gSoundNum_Skid,gSoundNum_Shriek;
short			gSoundNum_Ship,gSoundNum_ExitShip,gSoundNum_Frog,gSoundNum_RobotDanger;
short			gSoundNum_ClownLaugh;

static	Boolean			gSongPlayingFlag = false;

static	short			gStatusBits[MAX_CHANNELS];				// set in maintainsounds (for debugging)

/********************* INIT SOUND TOOLS ********************/

void InitSoundTools(void)
{
OSErr		iErr;
short	srcFile;
static Str255	errStr = "Couldnt Open Music Resource File.";


	srcFile = OpenMikeRezFile(":data:audio:music",errStr);	// open music rez file


	gMaxChannels = 0;
	gNumEffectsLoaded = 0;
	gNumAddedSounds = 0;

	for (gMaxChannels = 0; gMaxChannels < MAX_CHANNELS; gMaxChannels++)
	{
						/* ALLOC CHANNEL */

		iErr = SndNewChannel(&gSndChannel[gMaxChannels],sampledSynth,
							initMono+initNoInterp+initNoDrop,nil);
		if (iErr)
			break;
	}

	LoadDefaultSounds();									// these are never deleted!

	SetVolume();											// make sure all channels set to current volume

#if SET_SYS_VOL
	SetDefaultOutputVolume(0x100L);							// set system volume
#endif
}


/************************** LOAD DEFAULT SOUNDS ************************/
//
// Loads the standard default effect sounds
//

void LoadDefaultSounds(void)
{
Str255		error = "Couldnt Open Sound Resource File.";
OSErr		iErr;
short			i;
short			srcFile1,srcFile2;

	gNumEffectsLoaded = 0;

						/* OPEN SOUNDS RESOURCE */

	srcFile1 = OpenMikeRezFile(":data:audio:general.sounds",error);	// open sound resource fork
	UseResFile( srcFile1 );
	srcFile2 = OpenMikeRezFile(":data:audio:weapon.sounds",error);
	UseResFile( srcFile2 );

					/* LOAD ALL EFFECTS */

	for (i=0; i<NUM_DEFAULT_EFFECTS; i++)
	{

		EffectHandles[i] = GetResource('snd ',BASE_EFFECT_RESOURCE+i);
		if (EffectHandles[i] == nil)
			DoFatalAlert("Couldnt find sound resource.");
		DetachResource(EffectHandles[i]);							// detach resource from rez file & make a normal Handle
		if ( iErr = ResError() )
			ShowSystemErr(iErr);
		HNoPurge(EffectHandles[i]);									// make non-purgeable
		HLockHi (EffectHandles[i]);
		gNumEffectsLoaded++;
	}

	CloseResFile(srcFile1);
	CloseResFile(srcFile2);
	UseResFile(gMainAppRezFile);
}


/*************** UNLOCK DEFAULT SOUNDS ********************/
//
// This is used as part of the memory optimization algorithm.
// First then sounds are unlocked, then other things are zapped and optimized,
// then the sounds are optimized and relocked.
//

void UnlockDefaultSounds(void)
{
short	i;

	StopAllSound();

	for (i=0; i<NUM_DEFAULT_EFFECTS; i++)
		HUnlock(EffectHandles[i]);
}


/**************** OPTIMIZE DEFAULT SOUNDS ***************/
//
// Assumes that UnlockDef.. has been previously called.
//

void OptimizeDefaultSounds(void)
{
short	i;
long	growBytes;

				/* COMPACT */

	MaxMem(&growBytes);								// compact all
	CompactMem(maxSize);

			/* RELOCK ALL */

	for (i = 0; i < NUM_DEFAULT_EFFECTS; i++)
		HLockHi(EffectHandles[i]);					// relock it
}


/********************* START MUSIC **********************/
//
// NOTE: freqCmd must be used for the sample to use the loop points.
//

void StartMusic(void)
{
SndCommand 	mySndCmd;
long		offset;

	if (!gMusicOnFlag)									// see if music activated
		return;

	GetSoundHeaderOffset((SndListHandle)SoundHand_Music,&offset);		// get offset to header

	mySndCmd.cmd = soundCmd;							// install sample in the channel
	mySndCmd.param1 = 0;
	mySndCmd.param2 = (long)(*SoundHand_Music+offset);		// pointer to SoundHeader
	SndDoCommand(gSndChannel[0], &mySndCmd, true);

	mySndCmd.cmd = freqCmd;								// call this to START sound & keep looping
	mySndCmd.param1 = 0;
	mySndCmd.param2 = kMiddleC;							// MIDI freq
	SndDoImmediate(gSndChannel[0], &mySndCmd);

	gSongPlayingFlag = true;
}


/********************* STOP MUSIC **********************/

void StopMusic(void)
{
	if (!gMusicOnFlag)									// see if music activated
		return;

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

void PlaySong(short songNum)
{
short	srcFile;
OSErr 		iErr;
static Str255	errStr = "Couldnt Open Music Resource File.";

	KillSong();											// see if zap existing song
	MaxMem(&someLong);									// clean up
	CompactMem(maxSize);

						/* OPEN MUSIC RESOURCE */

	srcFile = OpenMikeRezFile(":data:audio:music",errStr);
	UseResFile( srcFile );

	SoundHand_Music = GetResource('snd ',songNum);		// load the song
	if (SoundHand_Music == nil)
		return;						// (if err, just don't play anything)
//		DoFatalAlert("Couldnt find Music Resource.");
	DetachResource(SoundHand_Music);					// detach resource from rez file & make a normal Handle
	if ( iErr = ResError() )
		ShowSystemErr(iErr);

	HLockHi (SoundHand_Music);

	CloseResFile(srcFile);
	UseResFile(gMainAppRezFile);

	StartMusic();
}


/*********************** KILL SONG *********************/

void KillSong(void)
{
	if (SoundHand_Music != nil)							// see if zap existing song
	{
		StopMusic();
		DisposeHandle(SoundHand_Music);
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
static 	OSErr			iErr;
static	SndChannelPtr	chanPtr;
short					theChan;
SCStatus				theStatus;
long	offset;
OSErr	myErr;

	if (!gEffectsOnFlag)								// see if effects activated
		return(-1);

	if (soundNum >= gNumEffectsLoaded)					// see if illegal sound #
		DoFatalAlert("Illegal sound number!");

	for (theChan=gMusicOnFlag; theChan < gMaxChannels; theChan++)
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
	mySndCmd.param2 = (long)(*EffectHandles[soundNum]+offset);	// pointer to SoundHeader
	myErr = SndDoImmediate(chanPtr, &mySndCmd);
	if (myErr)
		ShowSystemErr(myErr);

	mySndCmd.cmd = freqCmd;								// call this to START sound & keep looping
	mySndCmd.param1 = 0;
	mySndCmd.param2 = kMiddleC;							// MIDI freq
	myErr = SndDoImmediate(chanPtr, &mySndCmd);
	if (myErr)
		ShowSystemErr(myErr);

	return(theChan);									// return channel #
}



/********************* SET VOLUME *******************/

void SetVolume(void)
{
short	i;
static	SndChannelPtr	chanPtr;
static	SndCommand 	mySndCmd;
static 	OSErr		iErr;

	for (i=0; i<gMaxChannels; i++)
	{
		chanPtr = gSndChannel[i];

		mySndCmd.cmd = ampCmd;
		mySndCmd.param1 = gVolume;
		mySndCmd.param2 = 0;
		iErr = SndDoImmediate(chanPtr, &mySndCmd);
	}
}

/******************** TOGGLE MUSIC *********************/

void ToggleMusic(void)
{
	if (gMusicOnFlag)
		StopMusic();


	gMusicOnFlag = gMusicOnFlag^1;

	if (gMusicOnFlag)
		StartMusic();

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
short		theChan;
SCStatus	theStatus;


    if (checkKeys)
    {
    			/* SEE IF TOGGLE MUSIC */

    	if (CheckNewKeyDown2(kKey_ToggleMusic,&gMKeyDownFlag))
    		ToggleMusic();


    			/* SEE IF TOGGLE EFFECTS */

    	if (CheckNewKeyDown2(kKey_ToggleEffects,&gSKeyDownFlag))
    		ToggleEffects();


    			/* SEE IF CHANGE VOLUME */

    	if (GetKeyState(kKey_RaiseVolume))
    	{
    		if (gVolume < (255-4))
    		{
    			gVolume+=4;
    			SetVolume();
    		}
    	}
    	else
    	if (GetKeyState(kKey_LowerVolume))
    	{
    		if (gVolume > 4)
    		{
    			gVolume -= 4;
    			SetVolume();
    		}
    	}
    }

				/* GET STATUS OF CHANNELS (FOR TESTING ONLY) */

	for (theChan=0; theChan < gMaxChannels; theChan++)
	{
		SndChannelStatus(gSndChannel[theChan],sizeof(SCStatus),&theStatus);	// get channel info
		gStatusBits[theChan] = theStatus.scChannelBusy;
	}
}



/******************* ADD EFFECT *******************/
//
// INPUT: rezFile = pathname of resource file to get sound from
//        rezNum = rezID # of sound
//		  freq = freq to play sound
//
// OUTPUT: sound #
//

short AddEffect(Str255 rezFile, short rezNum)
{
short			srcFile;

						/* OPEN SOUNDS RESOURCE */

	srcFile = OpenMikeRezFile(rezFile,"Couldnt Open a Sound Resource File.");	// open sound resource fork
	UseResFile( srcFile );

					/* LOAD IT */

	AddedHandles[gNumAddedSounds] = EffectHandles[gNumEffectsLoaded] = GetResource('snd ',rezNum);
	if (AddedHandles[gNumAddedSounds] == nil)
		DoFatalAlert("Couldnt find sound resource.");
	DetachResource(AddedHandles[gNumAddedSounds]);
	HNoPurge(EffectHandles[gNumEffectsLoaded]);					// make non-purgeable
	HLockHi(EffectHandles[gNumEffectsLoaded]);
	gNumEffectsLoaded++;
	gNumAddedSounds++;

	CloseResFile(srcFile);
	UseResFile(gMainAppRezFile);

	return(gNumEffectsLoaded-1);
}


/****************** ZAP ALL ADDED SOUNDS ******************/
//
// Zaps all of the added sounds
//

void ZapAllAddedSounds(void)
{
short		i;

	StopAllSound();

	gNumEffectsLoaded = NUM_DEFAULT_EFFECTS; 		// reset this to default value

	for (i = 0; i < gNumAddedSounds; i++)
	{
		DisposeHandle(AddedHandles[i]);
	}
	gNumAddedSounds = 0;
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
static Str255	jurassic = ":data:audio:jurassic.sounds";
static Str255	candy = ":data:audio:candy.sounds";
static Str255	clown = ":data:audio:clown.sounds";
static Str255	fairy = ":data:audio:fairy.sounds";
static Str255	bargain = ":data:audio:bargain.sounds";

	switch(gSceneNum)
	{
		case	SCENE_JURASSIC:
				gSoundNum_UngaBunga = AddEffect(jurassic,BASE_EFFECT_RESOURCE);
				gSoundNum_DinoBoom = AddEffect(jurassic,BASE_EFFECT_RESOURCE+1);
				gSoundNum_BarneyJump = AddEffect(jurassic,BASE_EFFECT_RESOURCE+2);
				gSoundNum_DoorOpen = AddEffect(jurassic,BASE_EFFECT_RESOURCE+3);
				break;

		case	SCENE_CANDY:
				gSoundNum_ChocoBunny = AddEffect(candy,BASE_EFFECT_RESOURCE);
				gSoundNum_Carmel = AddEffect(candy,BASE_EFFECT_RESOURCE+1);
				gSoundNum_GummyHaha = AddEffect(candy,BASE_EFFECT_RESOURCE+2);
				break;

		case	SCENE_CLOWN:
				gSoundNum_JackInTheBox = AddEffect(clown,BASE_EFFECT_RESOURCE);
				gSoundNum_Skid = AddEffect(clown,BASE_EFFECT_RESOURCE+1);
				gSoundNum_DoorOpen = AddEffect(clown,BASE_EFFECT_RESOURCE+2);
				gSoundNum_ClownLaugh = AddEffect(clown,BASE_EFFECT_RESOURCE+3);
				break;

		case	SCENE_FAIRY:
				gSoundNum_WitchHaha = AddEffect(fairy,BASE_EFFECT_RESOURCE+0);
				gSoundNum_Shriek = AddEffect(fairy,BASE_EFFECT_RESOURCE+1);
				gSoundNum_DoorOpen = AddEffect(fairy,BASE_EFFECT_RESOURCE+2);
				gSoundNum_Frog = AddEffect(fairy,BASE_EFFECT_RESOURCE+3);
				gSoundNum_BarneyJump = AddEffect(jurassic,BASE_EFFECT_RESOURCE+2);
				gSoundNum_DinoBoom = AddEffect(jurassic,BASE_EFFECT_RESOURCE+1);
				break;

		case	SCENE_BARGAIN:
				gSoundNum_Ship = AddEffect(bargain,BASE_EFFECT_RESOURCE+0);
				gSoundNum_ExitShip = AddEffect(bargain,BASE_EFFECT_RESOURCE+1);
				gSoundNum_DoorOpen = AddEffect(bargain,BASE_EFFECT_RESOURCE+2);
				gSoundNum_DogRoar = AddEffect(bargain,BASE_EFFECT_RESOURCE+3);
				gSoundNum_RobotDanger = AddEffect(bargain,BASE_EFFECT_RESOURCE+4);
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

	if (!gMusicOnFlag)						// if music is deactivated, then return true anyway to trick wait routines
		return(true);

	SndChannelStatus(gSndChannel[0],sizeof(SCStatus),&theStatus);	// get channel info
	return (theStatus.scChannelBusy);								// see if channel busy
}

