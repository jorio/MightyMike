#if !(GLRENDER)

#include <SDL3/SDL.h>
#include "myglobals.h"
#include "externs.h"
#include "misc.h"
#include "renderdrivers.h"
#include "framebufferfilter.h"

#if _DEBUG
#define CHECK_SDL_ERROR(success)										\
	do {					 											\
		if (!success)													\
			DoFatalSDLError(success, __func__, __LINE__);				\
	} while(0)

static void DoFatalSDLError(int error, const char* file, int line)
{
	static char alertbuf[1024];
	SDL_snprintf(alertbuf, sizeof(alertbuf), "SDL error %d\nin %s:%d\n%s", error, file, line, SDL_GetError());
	DoFatalAlert(alertbuf);
}
#else
#define CHECK_SDL_ERROR(success) (void) (success)
#endif

static SDL_Renderer*	gSDLRenderer		= NULL;
static SDL_Texture*		gSDLTexture			= NULL;
static color_t*			gFinalFramebuffer	= NULL;
const char*				gRendererName		= "NULL";
Boolean					gCanDoHQStretch		= true;

Boolean SDLRender_Init(void)
{
	gSDLRenderer = SDL_CreateRenderer(gSDLWindow, NULL);
	if (!gSDLRenderer)
		return false;
	// The texture bound to the renderer is created in-game after loading the prefs.

#if !(NOVSYNC)
	SDL_SetRenderVSync(gSDLRenderer, 1);
#endif
	
	const char* sdlRendererName = SDL_GetRendererName(gSDLRenderer);
	if (sdlRendererName)
	{
		static char rendererName[32];
		SDL_snprintf(rendererName, sizeof(rendererName), "sdl-%s-%d", sdlRendererName, (int) sizeof(color_t) * 8);
		gRendererName = rendererName;
	}
	
	SDL_SetRenderLogicalPresentation(gSDLRenderer, VISIBLE_WIDTH, VISIBLE_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);

	return true;
}

static void SDLRender_NukeTextureAndBuffers(void)
{
	if (gFinalFramebuffer)
	{
		DisposePtr((Ptr) gFinalFramebuffer);
		gFinalFramebuffer = NULL;
	}

	if (gSDLTexture)
	{
		SDL_DestroyTexture(gSDLTexture);
		gSDLTexture = NULL;
	}
}

void SDLRender_Shutdown(void)
{
	ShutdownRenderThreads();

	SDLRender_NukeTextureAndBuffers();

	if (gSDLRenderer)
	{
		SDL_DestroyRenderer(gSDLRenderer);
		gSDLRenderer = NULL;
	}
}

void SDLRender_InitTexture(void)
{
	// Nuke old texture and RGBA buffers
	SDLRender_NukeTextureAndBuffers();

	bool crisp = (gEffectiveScalingType == kScaling_PixelPerfect);
	int textureSizeMultiplier = (gEffectiveScalingType == kScaling_HQStretch) ? 2 : 1;

	// Allocate buffer
	gFinalFramebuffer = (color_t*) NewPtrClear((VISIBLE_WIDTH * 2) * (VISIBLE_HEIGHT * 2) * (int) sizeof(color_t));
	GAME_ASSERT(gFinalFramebuffer);

	// Recreate texture
	gSDLTexture = SDL_CreateTexture(
			gSDLRenderer,
#if FRAMEBUFFER_COLOR_DEPTH == 16
			SDL_PIXELFORMAT_RGB565,
#else
			SDL_PIXELFORMAT_RGBA8888,
#endif
			SDL_TEXTUREACCESS_STREAMING,
			VISIBLE_WIDTH * textureSizeMultiplier,
			VISIBLE_HEIGHT * textureSizeMultiplier);
	GAME_ASSERT(gSDLTexture);

	// Set scaling quality
	SDL_SetTextureScaleMode(gSDLTexture, crisp ? SDL_SCALEMODE_NEAREST : SDL_SCALEMODE_LINEAR);

	// Set logical size
	SDL_SetRenderLogicalPresentation(gSDLRenderer, VISIBLE_WIDTH, VISIBLE_HEIGHT, crisp ? SDL_LOGICAL_PRESENTATION_INTEGER_SCALE : SDL_LOGICAL_PRESENTATION_LETTERBOX);
}

void SDLRender_PresentFramebuffer(void)
{
	bool success = true;

	//-------------------------------------------------------------------------
	// Convert indexed to RGBA, with optional post-processing

	ConvertFramebufferMT(gFinalFramebuffer);

	//-------------------------------------------------------------------------
	// Update SDL texture

	int pitch = VISIBLE_WIDTH * (int) sizeof(color_t);

	if (gEffectiveScalingType == kScaling_HQStretch)
		pitch *= 2;

	success = SDL_UpdateTexture(gSDLTexture, NULL, gFinalFramebuffer, pitch);
	CHECK_SDL_ERROR(success);

	//-------------------------------------------------------------------------
	// Present it

	SDL_RenderClear(gSDLRenderer);
	success = SDL_RenderTexture(gSDLRenderer, gSDLTexture, NULL, NULL);
	CHECK_SDL_ERROR(success);
	SDL_RenderPresent(gSDLRenderer);
}

#endif
