//
// input.h
//

#include <SDL3/SDL.h>

void InitInput(void);
void UpdateInput(void);
void ClearInput(void);

bool GetNewSDLKeyState(SDL_Scancode sdlScanCode);
bool GetSDLKeyState(SDL_Scancode sdlScanCode);
bool UserWantsOut(void);
bool UserWantsOutContinuous(void);
bool IsCmdQPressed(void);
bool GetNewNeedState(int needID);
bool GetNeedState(int needID);
SDL_Gamepad* TryOpenGamepad(bool showMessage);
int32_t GetLeftStickMagnitude_Fix32(void);
short GetRightStick8WayAim(void);
