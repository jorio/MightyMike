/****************************/
/*   	  INPUT.C	   	    */
/* By Brian Greenstone      */
/* (c)1997-99 Pangea Software  */
/* (c)2022 Iliyas Jorio     */
/****************************/


/***************/
/* EXTERNALS   */
/***************/

#include <Pomme.h>
#include <SDL3/SDL.h>
#include <math.h>

#include "misc.h"
#include "input.h"
#include "window.h"
#include "structures.h"
#include "externs.h"

/**********************/
/*     PROTOTYPES     */
/**********************/

SDL_Gamepad*		gSDLGamepad = NULL;
SDL_Haptic*			gSDLHaptic = NULL;

Byte				gRawKeyboardState[SDL_SCANCODE_COUNT];
char				gTextInput[64];

Byte				gNeedStates[NUM_CONTROL_NEEDS];

static void ParseAltEnter(void);
static void OnJoystickRemoved(SDL_JoystickID which);

/****************************/
/*    CONSTANTS             */
/****************************/

enum
{
	KEYSTATE_ACTIVE_BIT		= 0b001,
	KEYSTATE_CHANGE_BIT		= 0b010,
	KEYSTATE_IGNORE_BIT		= 0b100,

	KEYSTATE_OFF			= 0b000,
	KEYSTATE_DOWN			= KEYSTATE_ACTIVE_BIT | KEYSTATE_CHANGE_BIT,
	KEYSTATE_HELD			= KEYSTATE_ACTIVE_BIT,
	KEYSTATE_UP				= KEYSTATE_OFF | KEYSTATE_CHANGE_BIT,
	KEYSTATE_IGNOREHELD		= KEYSTATE_OFF | KEYSTATE_IGNORE_BIT,
};

const int16_t kJoystickDeadZone		= (33 * 32767 / 100);
const int16_t kJoystickDeadZone_UI	= (66 * 32767 / 100);


/**********************/
/*     VARIABLES      */
/**********************/


/**********************/
/* STATIC FUNCTIONS   */
/**********************/

static inline void UpdateKeyState(Byte* state, bool downNow)
{
	switch (*state)	// look at prev state
	{
		case KEYSTATE_HELD:
		case KEYSTATE_DOWN:
			*state = downNow ? KEYSTATE_HELD : KEYSTATE_UP;
			break;

		case KEYSTATE_OFF:
		case KEYSTATE_UP:
		default:
			*state = downNow ? KEYSTATE_DOWN : KEYSTATE_OFF;
			break;

		case KEYSTATE_IGNOREHELD:
			*state = downNow ? KEYSTATE_IGNOREHELD : KEYSTATE_OFF;
			break;
	}
}

/************************* INIT INPUT *********************************/

void InitInput(void)
{
}


void UpdateInput(void)
{

	gTextInput[0] = '\0';


	/**********************/
	/* DO SDL MAINTENANCE */
	/**********************/

	int mouseWheelDelta = 0;

	SDL_PumpEvents();
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_EVENT_QUIT:
			CleanQuit();
			return;

		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			CleanQuit();
			return;

		case SDL_EVENT_WINDOW_RESIZED:
			OnChangeIntegerScaling();
			break;

		case SDL_EVENT_TEXT_INPUT:
			SDL_snprintf(gTextInput, sizeof(gTextInput), "%s", event.text.text);
			break;

		case SDL_EVENT_GAMEPAD_ADDED:
			TryOpenGamepad(false);
			break;

		case SDL_EVENT_GAMEPAD_REMOVED:
			OnJoystickRemoved(event.gdevice.which);
			break;

		case SDL_EVENT_MOUSE_WHEEL:
			mouseWheelDelta += event.wheel.y;
			mouseWheelDelta += event.wheel.x;
			break;
		}
	}

	// --------------------------------------------
	// Update raw keyboard state

	int numkeys = 0;
	const bool* keystate = SDL_GetKeyboardState(&numkeys);
	uint32_t mouseButtons = SDL_GetMouseState(NULL, NULL);

	{
		int minNumKeys = numkeys < SDL_SCANCODE_COUNT ? numkeys : SDL_SCANCODE_COUNT;

		for (int i = 0; i < minNumKeys; i++)
		{
			UpdateKeyState(&gRawKeyboardState[i], keystate[i]);
		}

		// fill out the rest
		for (int i = minNumKeys; i < SDL_SCANCODE_COUNT; i++)
		{
			UpdateKeyState(&gRawKeyboardState[i], false);
		}
	}

	// --------------------------------------------
	// Parse system key chords

#if !OSXPPC	// on OSXPPC, hot-switching fullscreen mode is flaky
	ParseAltEnter();
#endif

	if ((!gIsInGame || gIsGamePaused) && IsCmdQPressed())
	{
		CleanQuit();
	}

	// --------------------------------------------
	// Update needs

	for (int i = 0; i < NUM_CONTROL_NEEDS; i++)
	{
		const KeyBinding* kb = &gGamePrefs.keys[i];

		bool downNow = false;

		for (int j = 0; j < KEYBINDING_MAX_KEYS; j++)
			if (kb->key[j] && kb->key[j] < numkeys)
				downNow |= KEYSTATE_ACTIVE_BIT & gRawKeyboardState[kb->key[j]];

		switch (kb->mouse.type)
		{
			case kButton:
				downNow |= KEYSTATE_ACTIVE_BIT & (mouseButtons & SDL_BUTTON_MASK(kb->mouse.id));
				break;

			case kAxisPlus:
				downNow |= mouseWheelDelta > 0;
				break;

			case kAxisMinus:
				downNow |= mouseWheelDelta < 0;
				break;

			default:
				break;
		}

		if (gSDLGamepad)
		{
			int16_t deadZone = i >= NUM_REMAPPABLE_NEEDS
								? kJoystickDeadZone_UI
								: kJoystickDeadZone;

			for (int j = 0; j < KEYBINDING_MAX_GAMEPAD_BUTTONS; j++)
			{
				switch (kb->gamepad[j].type)
				{
					case kButton:
						downNow |= KEYSTATE_ACTIVE_BIT & SDL_GetGamepadButton(gSDLGamepad, kb->gamepad[j].id);
						break;

					case kAxisPlus:
						downNow |= SDL_GetGamepadAxis(gSDLGamepad, kb->gamepad[j].id) > deadZone;
						break;

					case kAxisMinus:
						downNow |= SDL_GetGamepadAxis(gSDLGamepad, kb->gamepad[j].id) < -deadZone;
						break;

					default:
						break;
				}
			}
		}

		UpdateKeyState(&gNeedStates[i], downNow);
	}
}

void ClearInput(void)
{
	SDL_memset(gRawKeyboardState, KEYSTATE_HELD, sizeof(gRawKeyboardState));
	SDL_memset(gNeedStates, KEYSTATE_HELD, sizeof(gNeedStates));
//	ClearMouseState();
//	EatMouseEvents();
}

static void ParseAltEnter(void)
{
	if ((GetSDLKeyState(SDL_SCANCODE_LALT) || GetSDLKeyState(SDL_SCANCODE_RALT))
		&& GetNewSDLKeyState(SDL_SCANCODE_RETURN))
	{
		gRawKeyboardState[SDL_SCANCODE_LALT] = KEYSTATE_IGNOREHELD;
		gRawKeyboardState[SDL_SCANCODE_RALT] = KEYSTATE_IGNOREHELD;
		gRawKeyboardState[SDL_SCANCODE_RETURN] = KEYSTATE_IGNOREHELD;

#if 0
		gGamePrefs.displayMode++;
		gGamePrefs.displayMode %= kDisplayMode_COUNT;
#else
		static int previousFullscreenSetting = kDisplayMode_FullscreenStretched;

		if (gGamePrefs.displayMode == kDisplayMode_Windowed)
		{
			gGamePrefs.displayMode = previousFullscreenSetting;
		}
		else
		{
			previousFullscreenSetting = gGamePrefs.displayMode;
			gGamePrefs.displayMode = kDisplayMode_Windowed;
		}
#endif

		SetFullscreenMode(false);
	}
}

/************************ GET SKIP KEY STATE ***************************/

bool UserWantsOut(void)
{
	return GetNewNeedState(kNeed_UIConfirm) || GetNewNeedState(kNeed_UIBack);
}

bool UserWantsOutContinuous(void)
{
	return GetNeedState(kNeed_UIConfirm) || GetNeedState(kNeed_UIBack);
}

#pragma mark -


bool GetNewSDLKeyState(SDL_Scancode sdlScanCode)
{
	return gRawKeyboardState[sdlScanCode] == KEYSTATE_DOWN;
}

bool GetSDLKeyState(SDL_Scancode sdlScanCode)
{
	return gRawKeyboardState[sdlScanCode] == KEYSTATE_DOWN || gRawKeyboardState[sdlScanCode] == KEYSTATE_HELD;
}

bool GetNeedState(int needID)
{
	GAME_ASSERT(needID < NUM_CONTROL_NEEDS);
	return 0 != (gNeedStates[needID] & KEYSTATE_ACTIVE_BIT);
}

bool GetNewNeedState(int needID)
{
	GAME_ASSERT(needID < NUM_CONTROL_NEEDS);
	return gNeedStates[needID] == KEYSTATE_DOWN;
}

bool IsCmdQPressed(void)
{
#if __APPLE__
	return (GetSDLKeyState(SDL_SCANCODE_LGUI) || GetSDLKeyState(SDL_SCANCODE_RGUI))
		&& GetNewSDLKeyState(SDL_GetScancodeFromKey(SDLK_Q, NULL));
#else
	// on non-mac systems, alt-f4 is handled by the system
	return false;
#endif
}


#pragma mark -

/****************************** SDL JOYSTICK FUNCTIONS ********************************/

SDL_Gamepad* TryOpenGamepad(bool showMessage)
{
	if (gSDLGamepad)
	{
		SDL_Log("Already have a valid gamepad.");
		return gSDLGamepad;
	}

	int numJoysticks = 0;
	SDL_JoystickID* joysticks = SDL_GetJoysticks(&numJoysticks);

	for (int i = 0; i < numJoysticks && gSDLGamepad == NULL; i++)
	{
		SDL_JoystickID joystickID = joysticks[i];
		if (SDL_IsGamepad(joystickID))
		{
			gSDLGamepad = SDL_OpenGamepad(joystickID);
			if (gSDLGamepad)
				GAME_ASSERT(SDL_GetGamepadID(gSDLGamepad) == joystickID);
		}
	}

	SDL_free(joysticks);
	joysticks = NULL;

	if (numJoysticks == 0)
	{
		return NULL;
	}

	if (!gSDLGamepad)
	{
		SDL_Log("Joystick(s) found, but none is suitable as an SDL_Gamepad.");
		if (showMessage)
		{
			char messageBuf[1024];
			SDL_snprintf(messageBuf, sizeof(messageBuf),
				"The game does not support your controller yet (\"%s\").\n\n"
				"You can play with the keyboard instead. Sorry!",
				SDL_GetJoystickNameForID(0));
			SDL_ShowSimpleMessageBox(
				SDL_MESSAGEBOX_WARNING,
				"Controller not supported",
				messageBuf,
				gSDLWindow);
		}
		return NULL;
	}

	SDL_Log("Opened joystick %d as controller: %s", SDL_GetGamepadID(gSDLGamepad), SDL_GetGamepadName(gSDLGamepad));

	gSDLHaptic = SDL_OpenHapticFromJoystick(SDL_GetGamepadJoystick(gSDLGamepad));
	if (gSDLHaptic)
	{
		SDL_InitHapticRumble(gSDLHaptic);
	}

	return gSDLGamepad;
}

static void OnJoystickRemoved(SDL_JoystickID which)
{
	if (NULL == gSDLGamepad)		// don't care, I didn't open any controller
		return;

	if (which != SDL_GetGamepadID(gSDLGamepad))	// don't care, this isn't the joystick I'm using
		return;

	SDL_Log("Current joystick was removed: %d", which);

	// Nuke haptic device, if any
	if (gSDLHaptic)
	{
		SDL_CloseHaptic(gSDLHaptic);
		gSDLHaptic = NULL;
	}

	// Nuke reference to this controller+joystick
	SDL_CloseGamepad(gSDLGamepad);
	gSDLGamepad = NULL;

	// Try to open another gamepad if any is connected.
	TryOpenGamepad(false);
}

int32_t GetLeftStickMagnitude_Fix32(void)
{
	if (!gSDLGamepad)
	{
		return 0;
	}

	int dxRaw = (int) SDL_GetGamepadAxis(gSDLGamepad, SDL_GAMEPAD_AXIS_LEFTX);
	int dyRaw = (int) SDL_GetGamepadAxis(gSDLGamepad, SDL_GAMEPAD_AXIS_LEFTY);

	int magnitudeSquared = dxRaw * dxRaw + dyRaw * dyRaw;

	if (magnitudeSquared < kJoystickDeadZone * kJoystickDeadZone)
	{
		return 0;
	}

	return (int32_t)(0x10000 * sqrtf(dxRaw * dxRaw + dyRaw * dyRaw) / 32767.0f);
}

short GetRightStick8WayAim(void)
{
	if (!gSDLGamepad)
	{
		return AIM_NONE;
	}

	int dxRaw = (int) SDL_GetGamepadAxis(gSDLGamepad, SDL_GAMEPAD_AXIS_RIGHTX);
	int dyRaw = (int) SDL_GetGamepadAxis(gSDLGamepad, SDL_GAMEPAD_AXIS_RIGHTY);

	bool right	= dxRaw > kJoystickDeadZone;
	bool left	= dxRaw < -kJoystickDeadZone;
	bool down	= dyRaw > kJoystickDeadZone;
	bool up		= dyRaw < -kJoystickDeadZone;

	if (down)
	{
		if (right)		return AIM_DOWN_RIGHT;
		else if (left)	return AIM_DOWN_LEFT;
		else			return AIM_DOWN;
	}
	else if (up)
	{
		if (right)		return AIM_UP_RIGHT;
		else if (left)	return AIM_UP_LEFT;
		else			return AIM_UP;
	}
	else
	{
		if (right)		return AIM_RIGHT;
		else if (left)	return AIM_LEFT;
		else			return AIM_NONE;
	}
}
