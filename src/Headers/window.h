//
// WINDOWS.h
//


					/* OFFSCREEN DEFINES */

#define WIDESCREEN 0

#if WIDESCREEN
#define	VISIBLE_WIDTH		(26*32L)	// dimensions of visible area (MULTIPLE OF 4!!!)
#define	VISIBLE_HEIGHT		(16*32L)
#else
#define	VISIBLE_WIDTH		640L	// dimensions of visible area
#define	VISIBLE_HEIGHT		480L
#endif

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

extern void	EraseOffscreenBuffer(void);
extern void	EraseBackgroundBuffer(void);
extern void	MakeGameWindow(void);
extern void	EraseGameWindow(void);
extern void	WindowToBlack(void);
extern void	DumpGameWindow(void);
extern void	DumpBackground(void);
extern void	EraseScreenArea(Rect);
extern void	BlankScreenArea(Rect);
extern void	WipeScreenBuffers(void);
extern void	InitScreenBuffers(void);
extern void	EraseStore(void);
extern void	DisplayStoreBuffer(void);
extern void	BlankEntireScreenArea(void);

void PresentIndexedFramebuffer(void);
void DumpIndexedTGA(const char* hostPath, int width, int height, const char* data);
void SetFullscreenMode(void);
void OnChangeIntegerScaling(void);