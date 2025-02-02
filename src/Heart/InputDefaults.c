#include "structures.h"
#include "input.h"
#include <SDL3/SDL.h>

#if __APPLE__
#define SC_NWEAPON2		SDL_SCANCODE_LGUI
#define SC_PWEAPON2		SDL_SCANCODE_RGUI
#define SC_ATTACK2		0						// on macos, ctrl+arrows is reserved by the OS by default
#else
#define SC_NWEAPON2		SDL_SCANCODE_LALT
#define SC_PWEAPON2		SDL_SCANCODE_RALT
#define SC_ATTACK2		SC(LCTRL)
#endif

// Scancode
#define SC(x)		SDL_SCANCODE_##x

// Mouse button/wheel
#define MB(x)		{kButton, SDL_BUTTON_##x}
#define MWPLUS()	{kAxisPlus, 0}
#define MWMINUS()	{kAxisMinus, 0}
#define MBNULL()	{kUnbound, 0}

// Controller button/axis
#define GB(x)		{kButton, SDL_GAMEPAD_BUTTON_##x}
#define GAPLUS(x)	{kAxisPlus, SDL_GAMEPAD_AXIS_##x}
#define GAMINUS(x)	{kAxisMinus, SDL_GAMEPAD_AXIS_##x}
#define GBNULL()	{kUnbound, 0}

const KeyBinding kDefaultKeyBindings[NUM_CONTROL_NEEDS] =
{
//Need---------------     Keys---------------------------  Mouse--------  Gamepad--------------------------------------
[kNeed_Up			] = { {SC(UP)		, SC(W)			}, MBNULL()		, {GB(DPAD_UP)			, GAMINUS(LEFTY)	} },
[kNeed_Down			] = { {SC(DOWN)		, SC(S)			}, MBNULL()		, {GB(DPAD_DOWN)		, GAPLUS(LEFTY)		} },
[kNeed_Left			] = { {SC(LEFT)		, SC(A)			}, MBNULL()		, {GB(DPAD_LEFT)		, GAMINUS(LEFTX)	} },
[kNeed_Right		] = { {SC(RIGHT)	, SC(D)			}, MBNULL()		, {GB(DPAD_RIGHT)		, GAPLUS(LEFTX)		} },
[kNeed_PrevWeapon	] = { {SC(RSHIFT)	, SC_PWEAPON2	}, MWPLUS()		, {GB(LEFT_SHOULDER)	, GBNULL()			} },
[kNeed_NextWeapon	] = { {SC(LSHIFT)	, SC_NWEAPON2	}, MWMINUS()	, {GB(RIGHT_SHOULDER)	, GB(EAST)			} },
[kNeed_Attack		] = { {SC(SPACE)	, SC_ATTACK2	}, MB(LEFT)		, {GAPLUS(RIGHT_TRIGGER), GB(WEST)			} },
[kNeed_Radar		] = { {SC(R)		, SC(TAB)		}, MB(MIDDLE)	, {GB(BACK)				, GB(NORTH)			} },
[kNeed_ToggleMusic	] = { {SC(M)		, 0				}, MBNULL()		, {GBNULL()				, GBNULL()			} },
[kNeed_UIUp			] = { {SC(UP)		, 0				}, MBNULL()		, {GB(DPAD_UP)			, GAMINUS(LEFTY)	} },
[kNeed_UIDown		] = { {SC(DOWN)		, 0				}, MBNULL()		, {GB(DPAD_DOWN)		, GAPLUS(LEFTY)		} },
[kNeed_UILeft		] = { {SC(LEFT)		, 0				}, MBNULL()		, {GB(DPAD_LEFT)		, GAMINUS(LEFTX)	} },
[kNeed_UIRight		] = { {SC(RIGHT)	, 0				}, MBNULL()		, {GB(DPAD_RIGHT)		, GAPLUS(LEFTX)		} },
[kNeed_UIPrev		] = { {0			, 0				}, MBNULL()		, {GB(LEFT_SHOULDER)	, GBNULL()			} },
[kNeed_UINext		] = { {0			, 0				}, MBNULL()		, {GB(RIGHT_SHOULDER)	, GBNULL()			} },
[kNeed_UIConfirm	] = { {SC(RETURN)	, SC(SPACE)		}, MBNULL()		, {GB(START)			, GB(SOUTH)			} },
[kNeed_UIBack		] = { {SC(ESCAPE)	, SC(BACKSPACE)	}, MBNULL()		, {GB(BACK)				, GB(EAST)			} },
[kNeed_UIPause		] = { {SC(ESCAPE)	, 0				}, MBNULL()		, {GB(START)			, GBNULL()			} },
};
