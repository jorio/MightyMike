#include <SDL.h>
#include "myglobals.h"
#include "externs.h"
#include "misc.h"
#include "renderdrivers.h"

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
uint8_t*				gRGBAFramebuffer	= NULL;			// [VISIBLE_WIDTH * VISIBLE_HEIGHT * 4]
uint8_t*				gRGBAFramebufferX2	= NULL;			// [VISIBLE_WIDTH * VISIBLE_HEIGHT * 4 * 4]

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

	SDL_RenderSetLogicalSize(gSDLRenderer, VISIBLE_WIDTH, VISIBLE_HEIGHT);

	return true;
}

static void SDLRender_NukeTextureAndBuffers(void)
{
	if (gRGBAFramebuffer)
	{
		DisposePtr((Ptr) gRGBAFramebuffer);
		gRGBAFramebuffer = NULL;
	}

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

	// Allocate buffers
	gRGBAFramebuffer = (uint8_t*) NewPtrClear(VISIBLE_WIDTH * VISIBLE_HEIGHT * 4);
	GAME_ASSERT(gRGBAFramebuffer);

	gRGBAFramebufferX2 = (uint8_t*) NewPtrClear((VISIBLE_WIDTH*2) * (VISIBLE_HEIGHT*2) * 4);
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

	ConvertFramebufferToRGBA();

	//-------------------------------------------------------------------------
	// Update SDL texture

	if (gEffectiveScalingType == kScaling_HQStretch)
		err = SDL_UpdateTexture(gSDLTexture, NULL, gRGBAFramebufferX2, VISIBLE_WIDTH*4*2);
	else
		err = SDL_UpdateTexture(gSDLTexture, NULL, gRGBAFramebuffer, VISIBLE_WIDTH*4);
	CHECK_SDL_ERROR(err);

	//-------------------------------------------------------------------------
	// Present it

	SDL_RenderClear(gSDLRenderer);
	err = SDL_RenderCopy(gSDLRenderer, gSDLTexture, NULL, NULL);
	CHECK_SDL_ERROR(err);
	SDL_RenderPresent(gSDLRenderer);
}
