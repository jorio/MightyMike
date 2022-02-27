//
// WINDOWS.h
//


					/* OFFSCREEN DEFINES */

#define OFFSCREEN_BORDER_WIDTHX	4L					// border on each side (MULTIPLE OF 4!!!)
#define OFFSCREEN_BORDER_WIDTHY	4L					// border on each side

#define	OFFSCREEN_WIDTH		(VISIBLE_WIDTH+(OFFSCREEN_BORDER_WIDTHX*2))	// dimensions of offscreen buffer
#define	OFFSCREEN_HEIGHT	(VISIBLE_HEIGHT+(OFFSCREEN_BORDER_WIDTHY*2))


#define OFFSCREEN_WINDOW_LEFT	OFFSCREEN_BORDER_WIDTHX		//((OFFSCREEN_WIDTH/2)-(VISIBLE_WIDTH/2))
#define OFFSCREEN_WINDOW_TOP	OFFSCREEN_BORDER_WIDTHY		//((OFFSCREEN_HEIGHT/2)-(VISIBLE_HEIGHT/2))
#define OFFSCREEN_WINDOW_RIGHT	(OFFSCREEN_WINDOW_LEFT+VISIBLE_WIDTH-1)
#define OFFSCREEN_WINDOW_BOTTOM	(OFFSCREEN_WINDOW_TOP+VISIBLE_HEIGHT)
														// for centering picts in buffer
#define	WINDOW_OFFSET	 ((OFFSCREEN_WINDOW_TOP*OFFSCREEN_WIDTH)+OFFSCREEN_WINDOW_LEFT)



void CleanupDisplay(void);

void	EraseBackgroundBuffer(void);
void	MakeGameWindow(void);
void	DumpGameWindow(void);
void	DumpBackground(void);
void	EraseScreenArea(Rect);
void	BlankScreenArea(Rect);
void	EraseStore(void);
void	BlankEntireScreenArea(void);
void	SetScreenOffsetForArea(void);
void	SetScreenOffsetFor640x480(void);

void PresentIndexedFramebuffer(void);
void DumpIndexedTGA(const char* hostPath, int width, int height, const char* data);
void SetFullscreenMode(void);
void SetOptimalWindowSize(void);
void OnChangeIntegerScaling(void);

void ConvertFramebufferToRGBA(void);
void ShutdownRenderThreads(void);
