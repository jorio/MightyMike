#pragma once

#include <stdint.h>

#define MAX_RENDER_THREADS	32

#if GLRENDER
	#define FRAMEBUFFER_COLOR_DEPTH 16
#else
	#define FRAMEBUFFER_COLOR_DEPTH 32
#endif

#if FRAMEBUFFER_COLOR_DEPTH == 32
	typedef uint32_t color_t;
	#define finalColorsXX finalColors32
#elif FRAMEBUFFER_COLOR_DEPTH == 16
	typedef uint16_t color_t;
	#define finalColorsXX finalColors16
#else
	_Static_assert(false, "unsupported framebuffer color depth!");
#endif

void IndexedFramebufferToColor_NoFilter(color_t* color, int firstRow, int numRows);
void IndexedFramebufferToColor_FilterDithering(color_t* color, int threadNum, int firstRow, int numRows);
void DoublePixels(const color_t* colorx1, color_t* colorx2, int firstRow, int numRows);

void ConvertFramebufferMT(color_t* colorBuffer);
void InitRenderThreads(void);
void ShutdownRenderThreads(void);
