//
// input.h
//

#include <SDL.h>

void InitInput(void);
void UpdateInput(void);
void ClearInput(void);

bool GetNewSDLKeyState(unsigned short sdlScanCode);
bool GetSDLKeyState(unsigned short sdlScanCode);
bool UserWantsOut(void);
bool UserWantsOutContinuous(void);
bool IsCmdQPressed(void);
bool GetNewNeedState(int needID);
bool GetNeedState(int needID);
SDL_GameController* TryOpenController(bool showMessage);
int32_t GetLeftStickMagnitude_Fix32(void);
short GetRightStick8WayAim(void);
