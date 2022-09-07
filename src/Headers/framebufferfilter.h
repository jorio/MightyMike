#pragma once

#include <stdint.h>

#if GLRENDER
	#define FRAMEBUFFER_COLOR_DEPTH 16
#else
	#define FRAMEBUFFER_COLOR_DEPTH 32
#endif

#if FRAMEBUFFER_COLOR_DEPTH == 32
	typedef uint32_t color_t;
	#define finalColorsXX finalColors32

	typedef struct
	{
		uint8_t a;
		uint8_t r;
		uint8_t g;
		uint8_t b;
	} PackedColor;
#elif FRAMEBUFFER_COLOR_DEPTH == 16
	typedef uint16_t color_t;
	#define finalColorsXX finalColors16

	typedef struct
	{
		uint16_t r : 5;
		uint16_t g : 6;
		uint16_t b : 5;
	} PackedColor;
#else
	_Static_assert(false, "unsupported framebuffer color depth!");
#endif

void IndexedFramebufferToColor_NoFilter(color_t* color, int firstRow, int numRows);
void IndexedFramebufferToColor_FilterDithering(color_t* color, int threadNum, int firstRow, int numRows);
void DoublePixels(const color_t* colorx1, color_t* colorx2, int firstRow, int numRows);

void ConvertFramebufferMT(color_t* colorBuffer);
void ShutdownRenderThreads(void);
