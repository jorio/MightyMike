/****************************/
/*   	  INPUT.C	   	    */
/* (c)1997-99 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/

#include <Pomme.h>
#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "misc.h"
#include "input.h"
#include "window.h"
#include "structures.h"
#include "externs.h"

/**********************/
/*     PROTOTYPES     */
/**********************/

SDL_GameController* gSDLController = NULL;
SDL_JoystickID		gSDLJoystickInstanceID = -1;		// ID of the joystick bound to gSDLController
SDL_Haptic*			gSDLHaptic = NULL;

Byte				gRawKeyboardState[SDL_NUM_SCANCODES];
bool				gAnyNewKeysPressed = false;
char				gTextInput[SDL_TEXTINPUTEVENT_TEXT_SIZE];

Byte				gNeedStates[NUM_CONTROL_NEEDS];

static void OnJoystickRemoved(SDL_JoystickID which);

/****************************/
/*    CONSTANTS             */
/****************************/

enum
{
	KEYSTATE_OFF		= 0b00,
	KEYSTATE_UP			= 0b01,

	KEYSTATE_PRESSED	= 0b10,
	KEYSTATE_HELD		= 0b11,

	KEYSTATE_ACTIVE_BIT	= 0b10,
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
	case KEYSTATE_PRESSED:
		*state = downNow ? KEYSTATE_HELD : KEYSTATE_UP;
		break;
	case KEYSTATE_OFF:
	case KEYSTATE_UP:
	default:
		*state = downNow ? KEYSTATE_PRESSED : KEYSTATE_OFF;
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
		case SDL_QUIT:
			CleanQuit();
			return;

		case SDL_WINDOWEVENT:
			switch (event.window.event)
			{
			case SDL_WINDOWEVENT_CLOSE:
				CleanQuit();
				return;

			case SDL_WINDOWEVENT_RESIZED:
				OnChangeIntegerScaling();
				break;
			}
			break;

		case SDL_TEXTINPUT:
			memcpy(gTextInput, event.text.text, sizeof(gTextInput));
			_Static_assert(sizeof(gTextInput) == sizeof(event.text.text), "size mismatch: gTextInput / event.text.text");
			break;

		case SDL_JOYDEVICEADDED:	 // event.jdevice.which is the joy's INDEX (not an instance id!)
			TryOpenController(false);
			break;

		case SDL_JOYDEVICEREMOVED:	// event.jdevice.which is the joy's UNIQUE INSTANCE ID (not an index!)
			OnJoystickRemoved(event.jdevice.which);
			break;

		case SDL_MOUSEWHEEL:
			mouseWheelDelta += event.wheel.y;
			mouseWheelDelta += event.wheel.x;
			break;
		}
	}

	int numkeys = 0;
	const UInt8* keystate = SDL_GetKeyboardState(&numkeys);
	uint32_t mouseButtons = SDL_GetMouseState(NULL, NULL);

	gAnyNewKeysPressed = false;

	{
		int minNumKeys = numkeys < SDL_NUM_SCANCODES ? numkeys : SDL_NUM_SCANCODES;

		for (int i = 0; i < minNumKeys; i++)
		{
			UpdateKeyState(&gRawKeyboardState[i], keystate[i]);
			if (gRawKeyboardState[i] == KEYSTATE_PRESSED)
				gAnyNewKeysPressed = true;
		}

		// fill out the rest
		for (int i = minNumKeys; i < SDL_NUM_SCANCODES; i++)
			UpdateKeyState(&gRawKeyboardState[i], false);
	}

	// --------------------------------------------


	for (int i = 0; i < NUM_CONTROL_NEEDS; i++)
	{
		const KeyBinding* kb = &gGamePrefs.keys[i];

		bool downNow = false;

		for (int j = 0; j < KEYBINDING_MAX_KEYS; j++)
			if (kb->key[j] && kb->key[j] < numkeys)
				downNow |= 0 != keystate[kb->key[j]];

		switch (kb->mouse.type)
		{
			case kButton:
				downNow |= 0 != (mouseButtons & SDL_BUTTON(kb->mouse.id));
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

		if (gSDLController)
		{
			int16_t deadZone = i >= NUM_REMAPPABLE_NEEDS
								? kJoystickDeadZone_UI
								: kJoystickDeadZone;

			for (int j = 0; j < KEYBINDING_MAX_GAMEPAD_BUTTONS; j++)
			{
				switch (kb->gamepad[j].type)
				{
					case kButton:
						downNow |= 0 != SDL_GameControllerGetButton(gSDLController, kb->gamepad[j].id);
						break;

					case kAxisPlus:
						downNow |= SDL_GameControllerGetAxis(gSDLController, kb->gamepad[j].id) > deadZone;
						break;

					case kAxisMinus:
						downNow |= SDL_GameControllerGetAxis(gSDLController, kb->gamepad[j].id) < -deadZone;
						break;

					default:
						break;
				}
			}
		}

		UpdateKeyState(&gNeedStates[i], downNow);
	}

#if !OSXPPC	// on OSXPPC, hot-switching fullscreen mode is flaky
	if (GetNewNeedState(kNeed_ToggleFullscreen))
	{
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
#endif
}

void ClearInput(void)
{
	memset(gRawKeyboardState, KEYSTATE_HELD, sizeof(gRawKeyboardState));
	memset(gNeedStates, KEYSTATE_HELD, sizeof(gNeedStates));
//	ClearMouseState();
//	EatMouseEvents();
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


bool GetNewSDLKeyState(unsigned short sdlScanCode)
{
	return gRawKeyboardState[sdlScanCode] == KEYSTATE_PRESSED;
}

bool GetSDLKeyState(unsigned short sdlScanCode)
{
	return gRawKeyboardState[sdlScanCode] == KEYSTATE_PRESSED || gRawKeyboardState[sdlScanCode] == KEYSTATE_HELD;
}

bool AreAnyNewKeysPressed(void)
{
	return gAnyNewKeysPressed;
}

bool GetNeedState(int needID)
{
	GAME_ASSERT(needID < NUM_CONTROL_NEEDS);
	return 0 != (gNeedStates[needID] & KEYSTATE_ACTIVE_BIT);
}

bool GetNewNeedState(int needID)
{
	GAME_ASSERT(needID < NUM_CONTROL_NEEDS);
	return gNeedStates[needID] == KEYSTATE_PRESSED;
}


#pragma mark -

/****************************** SDL JOYSTICK FUNCTIONS ********************************/

SDL_GameController* TryOpenController(bool showMessage)
{
#if NOJOYSTICK
	(void) showMessage;
	return NULL;
#else
	if (gSDLController)
	{
		printf("Already have a valid controller.\n");
		return gSDLController;
	}

	if (SDL_NumJoysticks() == 0)
	{
		return NULL;
	}

	for (int i = 0; gSDLController == NULL && i < SDL_NumJoysticks(); ++i)
	{
		if (SDL_IsGameController(i))
		{
			gSDLController = SDL_GameControllerOpen(i);
			gSDLJoystickInstanceID = SDL_JoystickGetDeviceInstanceID(i);
		}
	}

	if (!gSDLController)
	{
		printf("Joystick(s) found, but none is suitable as an SDL_GameController.\n");
		if (showMessage)
		{
			char messageBuf[1024];
			snprintf(messageBuf, sizeof(messageBuf),
				"The game does not support your controller yet (\"%s\").\n\n"
				"You can play with the keyboard instead. Sorry!",
				SDL_JoystickNameForIndex(0));
			SDL_ShowSimpleMessageBox(
				SDL_MESSAGEBOX_WARNING,
				"Controller not supported",
				messageBuf,
				gSDLWindow);
		}
		return NULL;
	}

	printf("Opened joystick %d as controller: %s\n", gSDLJoystickInstanceID, SDL_GameControllerName(gSDLController));

	gSDLHaptic = SDL_HapticOpenFromJoystick(SDL_GameControllerGetJoystick(gSDLController));
	if (gSDLHaptic)
	{
		SDL_HapticRumbleInit(gSDLHaptic);
	}

	return gSDLController;
#endif
}

static void OnJoystickRemoved(SDL_JoystickID which)
{
#if NOJOYSTICK
	(void) which;
#else
	if (NULL == gSDLController)		// don't care, I didn't open any controller
		return;

	if (which != gSDLJoystickInstanceID)	// don't care, this isn't the joystick I'm using
		return;

	printf("Current joystick was removed: %d\n", which);

	// Nuke haptic device, if any
	if (gSDLHaptic)
	{
		SDL_HapticClose(gSDLHaptic);
		gSDLHaptic = NULL;
	}

	// Nuke reference to this controller+joystick
	SDL_GameControllerClose(gSDLController);
	gSDLController = NULL;
	gSDLJoystickInstanceID = -1;

	// Try to open another joystick if any is connected.
	TryOpenController(false);
#endif
}

int32_t GetLeftStickMagnitude_Fix32(void)
{
#if NOJOYSTICK
	return 0;
#else
	if (!gSDLController)
	{
		return 0;
	}

	int dxRaw = (int) SDL_GameControllerGetAxis(gSDLController, SDL_CONTROLLER_AXIS_LEFTX);
	int dyRaw = (int) SDL_GameControllerGetAxis(gSDLController, SDL_CONTROLLER_AXIS_LEFTY);

	int magnitudeSquared = dxRaw * dxRaw + dyRaw * dyRaw;

	if (magnitudeSquared < kJoystickDeadZone * kJoystickDeadZone)
	{
		return 0;
	}

	return (int32_t)(0x10000 * sqrtf(dxRaw * dxRaw + dyRaw * dyRaw) / 32767.0f);
#endif
}

short GetRightStick8WayAim(void)
{
#if NOJOYSTICK
	return -1;
#else
	if (!gSDLController)
	{
		return AIM_NONE;
	}

	int dxRaw = (int) SDL_GameControllerGetAxis(gSDLController, SDL_CONTROLLER_AXIS_RIGHTX);
	int dyRaw = (int) SDL_GameControllerGetAxis(gSDLController, SDL_CONTROLLER_AXIS_RIGHTY);

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
#endif
}
