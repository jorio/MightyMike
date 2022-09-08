// OPENGL RENDERING DRIVER
// (C) 2022 Iliyas Jorio
// This file is part of Mighty Mike. https://github.com/jorio/mightymike
//
// On PowerPC Macs, SDL 2.0.3's 2D renderer doesn't produce very fast results,
// especially if Altivec isn't available (G3 CPUs).
// So, on PPC, we bypass their renderer and use our own.

#if GLRENDER

#include <SDL.h>
#include "myglobals.h"
#include "externs.h"
#include "misc.h"
#include "renderdrivers.h"
#include "framebufferfilter.h"

#if __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#else
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>
PFNGLGENBUFFERSARBPROC glGenBuffersARB;
PFNGLBINDBUFFERARBPROC glBindBufferARB;
PFNGLMAPBUFFERARBPROC glMapBufferARB;
PFNGLBUFFERDATAARBPROC glBufferDataARB;
PFNGLUNMAPBUFFERARBPROC glUnmapBufferARB;
#endif

// RGB 5-6-5 appears to be the fastest format for streaming textures
// on graphics cards that ship with ancient PPC hardware
#define kFramePixelType GL_UNSIGNED_SHORT_5_6_5

#define kFrameTextureWidth 1024
#define kFrameTextureHeight 512

#if (kFramePixelType == GL_UNSIGNED_SHORT_5_6_5)
	#define kFrameInternalFormat	GL_RGB
	#define kFramePixelFormat		GL_RGB
	#define kFramePixelType			GL_UNSIGNED_SHORT_5_6_5
	#define kFrameBytesPerPixel		2
#elif (kFramePixelType == GL_UNSIGNED_SHORT_5_5_5_1)
	#define kFrameInternalFormat	GL_RGBA
	#define kFramePixelFormat		GL_RGBA
	#define kFramePixelType			GL_UNSIGNED_SHORT_5_5_5_1
	#define kFrameBytesPerPixel		2
#else
	#define kFrameInternalFormat	GL_RGBA
	#define kFramePixelFormat		GL_RGBA
	#define kFramePixelType			GL_UNSIGNED_BYTE
	#define kFrameBytesPerPixel		4
#endif

static SDL_GLContext gGLContext = NULL;
static GLuint gFrameTexture = 0;
static GLuint gFramePBO = 0;

const char* gRendererName = "NULL";

#if _DEBUG
#define CHECK_GL_ERROR()												\
	do {					 											\
		GLenum err = glGetError();										\
		if (err != GL_NO_ERROR)											\
			DoFatalGLError(err, __func__, __LINE__);					\
	} while(0)

static void DoFatalGLError(GLenum error, const char* file, int line)
{
	static char alertbuf[1024];
	snprintf(alertbuf, sizeof(alertbuf), "OpenGL error 0x%x\nin %s:%d", error, file, line);
	DoFatalAlert(alertbuf);
}
#else
#define CHECK_GL_ERROR() do {} while(0)
#endif

SDL_Point FitRectKeepAR(
		int logicalWidth,
		int logicalHeight,
		int displayWidth,
		int displayHeight)
{
	float displayAR = (float)displayWidth / (float)displayHeight;
	float logicalAR = (float)logicalWidth / (float)logicalHeight;

	if (displayAR >= logicalAR)
	{
		return (SDL_Point) { displayHeight * logicalAR, displayHeight };
	}
	else
	{
		return (SDL_Point) { displayWidth, displayWidth / logicalAR };
	}
}

static void GLRender_InitMatrices(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, VISIBLE_WIDTH, VISIBLE_HEIGHT, 0, 0, 1000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

#define GL_GET_PROC_ADDRESS(t, proc) \
do { \
    (proc) = SDL_GL_GetProcAddress(#proc); \
    GAME_ASSERT_MESSAGE((proc), "Missing OpenGL procedure " #proc); \
} while(0)

void GLRender_Init(void)
{
	puts("Using special PPC renderer!");

	gRendererName = "PPCGL";

	gGLContext = SDL_GL_CreateContext(gSDLWindow);
	GAME_ASSERT(gGLContext);

	int mkc = SDL_GL_MakeCurrent(gSDLWindow, gGLContext);
	GAME_ASSERT_MESSAGE(mkc == 0, SDL_GetError());

	GLint maxTextureSize = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	printf("Max texture size: %d\n", maxTextureSize);

	if (maxTextureSize < kFrameTextureWidth)
	{
		char message[128];
		snprintf(message, sizeof(message), "Your graphics card's max texture size (%d)\nis below the game's requirements (%d).", maxTextureSize, kFrameTextureWidth);
		DoAlert(message);
	}

#if !__APPLE__
	GL_GET_PROC_ADDRESS(PFNGLGENBUFFERSARBPROC, glGenBuffersARB);
	GL_GET_PROC_ADDRESS(PFNGLBINDBUFFERARBPROC, glBindBufferARB);
	GL_GET_PROC_ADDRESS(PFNGLUNMAPBUFFERPROC, glUnmapBufferARB);
	GL_GET_PROC_ADDRESS(PFNGLMAPBUFFERARBPROC, glMapBufferARB);
	GL_GET_PROC_ADDRESS(PFNGLBUFFERDATAARBPROC, glBufferDataARB);
#endif

#if !(NOVSYNC)
	SDL_GL_SetSwapInterval(1);
#else
	SDL_GL_SetSwapInterval(0);
#endif

	GLRender_InitMatrices();

	glDisable(GL_FOG);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
//	glEnable(GL_COLOR_MATERIAL);
	glDepthMask(false);

	glColor4f(1,1,1,1);

	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	CHECK_GL_ERROR();

	glGenTextures(1, &gFrameTexture);
	CHECK_GL_ERROR();

	glGenBuffersARB(1, &gFramePBO);
	CHECK_GL_ERROR();
	puts("PBO enabled!");

	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, gFramePBO);
	// perhaps we don't need to do this everytime?
	glBufferDataARB(
		GL_PIXEL_UNPACK_BUFFER_ARB,
		kFrameTextureWidth * kFrameTextureHeight * kFrameBytesPerPixel,
		NULL,
		GL_STREAM_DRAW);
	CHECK_GL_ERROR();

	glBindTexture(GL_TEXTURE_2D, gFrameTexture);
	CHECK_GL_ERROR();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		kFrameInternalFormat,
		kFrameTextureWidth,
		kFrameTextureHeight,
		0,
		kFramePixelFormat,
		kFramePixelType,
		NULL // need initial call with NULL so glTexSubImage2D works later on
	);
	CHECK_GL_ERROR();
}

void GLRender_Shutdown(void)
{
	ShutdownRenderThreads();

	if (gGLContext)
	{
		SDL_GL_DeleteContext(gGLContext);
		gGLContext = NULL;
	}
}

static SDL_Rect GetViewportSize(void)
{
	const int vw = VISIBLE_WIDTH;
	const int vh = VISIBLE_HEIGHT;

	int dw = 0;
	int dh = 0;
	SDL_GL_GetDrawableSize(gSDLWindow, &dw, &dh);	// DON'T use SDL_GetWindowSize as it returns fake scaled pixels in HiDPI displays

	SDL_Point size;
	if (gEffectiveScalingType == kScaling_PixelPerfect)
	{
		int zoom = GetMaxIntegerZoom(dw, dh);
		size = (SDL_Point) { zoom*vw, zoom*vh };
	}
	else
	{
		size = FitRectKeepAR(vw, vh, dw, dh);
	}

	return (SDL_Rect)
	{
		.x = (dw-size.x) / 2,
		.y = (dh-size.y) / 2,
		.w = size.x,
		.h = size.y
	};
}

void GLRender_PresentFramebuffer(void)
{
	static int vwPrevious = 0;
	static int vhPrevious = 0;
	static SDL_Rect previousViewportRect = {0};
	static int needClear = 60;

	const int vw = VISIBLE_WIDTH;
	const int vh = VISIBLE_HEIGHT;
	const float umax = vw * (1.0f / kFrameTextureWidth);
	const float vmax = vh * (1.0f / kFrameTextureHeight);

	int mkc = SDL_GL_MakeCurrent(gSDLWindow, gGLContext);
	GAME_ASSERT_MESSAGE(mkc == 0, SDL_GetError());

	int dw = 0;
	int dh = 0;
	SDL_GL_GetDrawableSize(gSDLWindow, &dw, &dh);	// DON'T use SDL_GetWindowSize as it returns fake scaled pixels in HiDPI displays

	SDL_Rect viewportRect = GetViewportSize();
	if (!SDL_RectEquals(&viewportRect, &previousViewportRect))
	{
		previousViewportRect = viewportRect;
		glViewport(viewportRect.x, viewportRect.y, viewportRect.w, viewportRect.h);
		needClear = 60;
	}

	GLRender_InitMatrices();

	glBindTexture(GL_TEXTURE_2D, gFrameTexture);

	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, gFramePBO);
	CHECK_GL_ERROR();

	// copy pixels from pbo to texture object
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vwPrevious, vhPrevious, kFramePixelFormat, kFramePixelType, NULL);
	CHECK_GL_ERROR();

	// get new PBO
	int numBytes = vw * vh * kFrameBytesPerPixel;
	glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, numBytes, NULL, GL_STREAM_DRAW);
	CHECK_GL_ERROR();
	vwPrevious = vw;  // we must remember the PBO's current size until we call glTexSubImage2D in the next iteration
	vhPrevious = vh;  // in case we're changing the size of the texture (user adjusted viewport size in game settings)

	void* mappedBuffer = glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY);
	CHECK_GL_ERROR();
	GAME_ASSERT(mappedBuffer);

	// now write data into the buffer, possibly in another thread
	ConvertFramebufferMT(mappedBuffer);

	glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB);
	CHECK_GL_ERROR();

	// On a Mini G4, NOT clearing the screen increases the framerate by 8%
	// so don't do it unless the viewport rectangle has recently changed.
	if (needClear > 0)
	{
		needClear--;
		glClearColor(0,0,0,1);
#if _DEBUG
		if (needClear > 4)
			glClearColor(0,0,1,1);
#endif
		glClear(GL_COLOR_BUFFER_BIT);
	}

	glBegin(GL_QUADS);
	glTexCoord2f(   0, vmax); glVertex3f( 0, vh, 0);
	glTexCoord2f(umax, vmax); glVertex3f(vw, vh, 0);
	glTexCoord2f(umax,    0); glVertex3f(vw,  0, 0);
	glTexCoord2f(   0,    0); glVertex3f( 0,  0, 0);
	glEnd();
	CHECK_GL_ERROR();

	SDL_GL_SwapWindow(gSDLWindow);
}

#endif // OSXPPC

