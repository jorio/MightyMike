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
	if (gGLContext)
	{
		SDL_GL_DeleteContext(gGLContext);
		gGLContext = NULL;
	}
}

#if (kFrameBytesPerPixel == 4)
static void ConvertIndexedFrameTo32(uint32_t* rgba32)
{
	const int yOffset			= 0;
	const uint8_t* indexed		= gIndexedFramebuffer + yOffset * VISIBLE_WIDTH;

	int numRows = VISIBLE_HEIGHT - yOffset;

	for (int y = 0; y < numRows; y++)
	{
		for (int x = 0; x < VISIBLE_WIDTH; x++)
		{
			*(rgba32++) = gGamePalette.finalColors32[*(indexed++)];
		}
	}
}
#else
static void ConvertIndexedFrameTo16(uint16_t* rgb16)
{
	const int yOffset			= 0;
	const uint8_t* indexed		= gIndexedFramebuffer + yOffset * VISIBLE_WIDTH;

	int numRows = VISIBLE_HEIGHT - yOffset;

	for (int y = 0; y < numRows; y++)
	{
		for (int x = 0; x < VISIBLE_WIDTH; x++)
		{
			*(rgb16++) = gGamePalette.finalColors16[*(indexed++)];
		}
	}
}
#endif

void GLRender_PresentFramebuffer(void)
{
	static int vwPrevious = 0;
	static int vhPrevious = 0;

	const int vw = VISIBLE_WIDTH;
	const int vh = VISIBLE_HEIGHT;
	const float umax = vw * (1.0f / kFrameTextureWidth);
	const float vmax = vh * (1.0f / kFrameTextureHeight);

	int mkc = SDL_GL_MakeCurrent(gSDLWindow, gGLContext);
	GAME_ASSERT_MESSAGE(mkc == 0, SDL_GetError());

	int ww = 0;
	int wh = 0;
	SDL_GetWindowSize(gSDLWindow, &ww, &wh);
	glViewport(0, 0, ww, wh);

	GLRender_InitMatrices();

	glBindTexture(GL_TEXTURE_2D, gFrameTexture);

	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, gFramePBO);
	CHECK_GL_ERROR();

	// copy pixels from pbo to texture object
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vwPrevious, vhPrevious, kFramePixelFormat, kFramePixelType, NULL);
	CHECK_GL_ERROR();

	// perhaps we don't need to do this everytime?
	int numBytes = vw * vh * kFrameBytesPerPixel;
	glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, numBytes, NULL, GL_STREAM_DRAW);
	CHECK_GL_ERROR();
	vwPrevious = vw;  // we must remember the PBO's current size until we call glTexSubImage2D in the next iteration
	vhPrevious = vh;  // in case we're changing the size of the texture (user adjusted viewport size in game settings)

	void* mappedBuffer = glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY);
	CHECK_GL_ERROR();
	GAME_ASSERT(mappedBuffer);

	// now write data into the buffer, possibly in another thread
#if (kFrameBytesPerPixel == 2)
	ConvertIndexedFrameTo16(mappedBuffer);
#else
	ConvertIndexedFrameTo32(mappedBuffer);
#endif

	glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB);
	CHECK_GL_ERROR();

//	glClearColor(0, 1, 0, 1);
//	glClear(GL_COLOR_BUFFER_BIT);

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

