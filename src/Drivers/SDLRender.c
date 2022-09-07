#if !(GLRENDER)

#include <SDL.h>
#include "myglobals.h"
#include "externs.h"
#include "misc.h"
#include "renderdrivers.h"
#include "framebufferfilter.h"

#if _DEBUG
#define CHECK_SDL_ERROR(err)											\
	do {					 											\
		if (err)														\
			DoFatalSDLError(err, __func__, __LINE__);					\
	} while(0)

static void DoFatalSDLError(int error, const char* file, int line)
{
	static char alertbuf[1024];
	snprintf(alertbuf, sizeof(alertbuf), "SDL error %d\nin %s:%d\n%s", error, file, line, SDL_GetError());
	DoFatalAlert(alertbuf);
}
#else
#define CHECK_SDL_ERROR(err) (void) (err)
#endif

static SDL_Renderer*	gSDLRenderer		= NULL;
static SDL_Texture*		gSDLTexture			= NULL;
static color_t*			gRGBAFramebufferX2	= NULL;
const char*				gRendererName		= "NULL";

Boolean SDLRender_Init(void)
{
	int rendererFlags = SDL_RENDERER_ACCELERATED;
#if !(NOVSYNC)
	rendererFlags |= SDL_RENDERER_PRESENTVSYNC;
#endif
	gSDLRenderer = SDL_CreateRenderer(gSDLWindow, -1, rendererFlags);
	if (!gSDLRenderer)
		return false;
	// The texture bound to the renderer is created in-game after loading the prefs.

	SDL_RendererInfo rendererInfo;
	if (0 == SDL_GetRendererInfo(gSDLRenderer, &rendererInfo))
	{
		static char rendererName[32];
		snprintf(rendererName, sizeof(rendererName), "SDL/%s", rendererInfo.name);
		gRendererName = rendererName;
	}

	SDL_RenderSetLogicalSize(gSDLRenderer, VISIBLE_WIDTH, VISIBLE_HEIGHT);

	return true;
}

static void SDLRender_NukeTextureAndBuffers(void)
{
	if (gRGBAFramebufferX2)
	{
		DisposePtr((Ptr) gRGBAFramebufferX2);
		gRGBAFramebufferX2 = NULL;
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
	gRGBAFramebufferX2 = (color_t*) NewPtrClear((VISIBLE_WIDTH*2) * (VISIBLE_HEIGHT*2) * 4);
	GAME_ASSERT(gRGBAFramebufferX2);

	// Set scaling quality before creating texture
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, crisp ? "nearest" : "best");

	// Recreate texture
	gSDLTexture = SDL_CreateTexture(
			gSDLRenderer,
			SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_STREAMING,
			VISIBLE_WIDTH * textureSizeMultiplier,
			VISIBLE_HEIGHT * textureSizeMultiplier);
	GAME_ASSERT(gSDLTexture);

	// Set logical size
	SDL_RenderSetLogicalSize(gSDLRenderer, VISIBLE_WIDTH, VISIBLE_HEIGHT);

	// Set integer scaling setting
#if SDL_VERSION_ATLEAST(2,0,5)
	SDL_RenderSetIntegerScale(gSDLRenderer, crisp);
#endif
}

void SDLRender_PresentFramebuffer(void)
{
	int err = 0;

	//-------------------------------------------------------------------------
	// Convert indexed to RGBA, with optional post-processing

	ConvertFramebufferMT(gRGBAFramebufferX2);

	//-------------------------------------------------------------------------
	// Update SDL texture

	int effectiveSize = VISIBLE_WIDTH * 4;

	if (gEffectiveScalingType == kScaling_HQStretch)
		effectiveSize *= 2;

	err = SDL_UpdateTexture(gSDLTexture, NULL, gRGBAFramebufferX2, effectiveSize);
	CHECK_SDL_ERROR(err);

	//-------------------------------------------------------------------------
	// Present it

	SDL_RenderClear(gSDLRenderer);
	err = SDL_RenderCopy(gSDLRenderer, gSDLTexture, NULL, NULL);
	CHECK_SDL_ERROR(err);
	SDL_RenderPresent(gSDLRenderer);
}

#endif
