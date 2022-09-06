#include <SDL.h>
#include "myglobals.h"
#include "externs.h"
#include "misc.h"
#include "renderdrivers.h"

SDL_Renderer*		gSDLRenderer	= NULL;
SDL_Texture*		gSDLTexture		= NULL;

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

void SDLRender_Shutdown(void)
{
	if (gSDLTexture)
	{
		SDL_DestroyTexture(gSDLTexture);
		gSDLTexture = NULL;
	}

	if (gSDLRenderer)
	{
		SDL_DestroyRenderer(gSDLRenderer);
		gSDLRenderer = NULL;
	}
}

void SDLRender_InitTexture(void)
{
	bool crisp = (gEffectiveScalingType == kScaling_PixelPerfect);
	int textureSizeMultiplier = (gEffectiveScalingType == kScaling_HQStretch) ? 2 : 1;

	// Nuke old texture
	if (gSDLTexture)
	{
		SDL_DestroyTexture(gSDLTexture);
		gSDLTexture = NULL;
	}

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

	// Set integer scaling setting
#if SDL_VERSION_ATLEAST(2,0,5)
	SDL_RenderSetIntegerScale(gSDLRenderer, crisp);
#endif
}

void SDLRender_PresentFramebuffer(void)
{
	int updateTextureRC = 0;
	if (gEffectiveScalingType == kScaling_HQStretch)
		updateTextureRC = SDL_UpdateTexture(gSDLTexture, NULL, gRGBAFramebufferX2, VISIBLE_WIDTH*4*2);
	else
		updateTextureRC = SDL_UpdateTexture(gSDLTexture, NULL, gRGBAFramebuffer, VISIBLE_WIDTH*4);
	SDL_RenderClear(gSDLRenderer);
	SDL_RenderCopy(gSDLRenderer, gSDLTexture, NULL, NULL);
	SDL_RenderPresent(gSDLRenderer);
}
