#include "structures.h"
#include "input.h"
#include <SDL.h>

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
#define CB(x)		{kButton, SDL_CONTROLLER_BUTTON_##x}
#define CAPLUS(x)	{kAxisPlus, SDL_CONTROLLER_AXIS_##x}
#define CAMINUS(x)	{kAxisMinus, SDL_CONTROLLER_AXIS_##x}
#define CBNULL()	{kUnbound, 0}

const KeyBinding kDefaultKeyBindings[NUM_CONTROL_NEEDS] =
{
//Need-------------------     Keys---------------------------  Mouse--------  Gamepad----------------------------------------
[kNeed_Up				] = { { SC(UP)		, SC(W)			}, MBNULL()		, { CB(DPAD_UP)			, CAMINUS(LEFTY)		} },
[kNeed_Down				] = { { SC(DOWN)	, SC(S)			}, MBNULL()		, { CB(DPAD_DOWN)		, CAPLUS(LEFTY)			} },
[kNeed_Left				] = { { SC(LEFT)	, SC(A)			}, MBNULL()		, { CB(DPAD_LEFT)		, CAMINUS(LEFTX)		} },
[kNeed_Right			] = { { SC(RIGHT)	, SC(D)			}, MBNULL()		, { CB(DPAD_RIGHT)		, CAPLUS(LEFTX)			} },
[kNeed_PrevWeapon		] = { { SC(RSHIFT)	, SC_PWEAPON2	}, MWPLUS()		, { CB(LEFTSHOULDER)	, CBNULL()				} },
[kNeed_NextWeapon		] = { { SC(LSHIFT)	, SC_NWEAPON2	}, MWMINUS()	, { CB(RIGHTSHOULDER)	, CB(B)					} },
[kNeed_Attack			] = { { SC(SPACE)	, SC_ATTACK2	}, MB(LEFT)		, { CAPLUS(TRIGGERRIGHT), CB(X)					} },
[kNeed_Radar			] = { { SC(R)		, SC(TAB)		}, MB(MIDDLE)	, { CB(BACK)			, CB(Y)					} },
[kNeed_ToggleMusic		] = { { SC(M)		, 0				}, MBNULL()		, { CBNULL()			, CBNULL()				} },
[kNeed_ToggleFullscreen	] = { { SC(F11)		, 0				}, MBNULL()		, { CBNULL()			, CBNULL()				} },
[kNeed_RaiseVolume		] = { { SC(EQUALS)	, 0				}, MBNULL()		, { CBNULL()			, CBNULL()				} },
[kNeed_LowerVolume		] = { { SC(MINUS)	, 0				}, MBNULL()		, { CBNULL()			, CBNULL()				} },
[kNeed_UIUp				] = { { SC(UP)		, 0				}, MBNULL()		, { CB(DPAD_UP)			, CAMINUS(LEFTY)		} },
[kNeed_UIDown			] = { { SC(DOWN)	, 0				}, MBNULL()		, { CB(DPAD_DOWN)		, CAPLUS(LEFTY)			} },
[kNeed_UILeft			] = { { SC(LEFT)	, 0				}, MBNULL()		, { CB(DPAD_LEFT)		, CAMINUS(LEFTX)		} },
[kNeed_UIRight			] = { { SC(RIGHT)	, 0				}, MBNULL()		, { CB(DPAD_RIGHT)		, CAPLUS(LEFTX)			} },
[kNeed_UIConfirm		] = { { SC(RETURN)	, SC(SPACE)		}, MBNULL()		, { CB(START)			, CB(A)					} },
[kNeed_UIBack			] = { { SC(ESCAPE)	, 0				}, MBNULL()		, { CB(BACK)			, CB(B)					} },
[kNeed_UIPause			] = { { SC(ESCAPE)	, 0				}, MBNULL()		, { CB(START)			, CBNULL()				} },
};
