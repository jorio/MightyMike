// FRAMEBUFFER FILTER
// (C) 2021 Iliyas Jorio
// This file is part of Mighty Mike. https://github.com/jorio/mightymike

#include "externs.h"
//#include "window.h"
#include "framebufferfilter.h"

static inline void FilterDithering_Row(const uint8_t* indexedRow, uint8_t* rowSmearFlags);

void IndexedFramebufferToColor_NoFilter(color_t* color, int firstRow, int numRows)
{
	color						= color + firstRow * VISIBLE_WIDTH;
	const uint8_t* indexed		= gIndexedFramebuffer + firstRow * VISIBLE_WIDTH;

	for (int y = 0; y < numRows; y++)
	{
		for (int x = 0; x < VISIBLE_WIDTH; x++)
		{
			*(color++) = gGamePalette.finalColorsXX[*(indexed++)];
		}
	}
}

void IndexedFramebufferToColor_FilterDithering(color_t* color, int threadNum, int firstRow, int numRows)
{
	color						= color + firstRow * VISIBLE_WIDTH;
	const uint8_t* indexed		= gIndexedFramebuffer + firstRow * VISIBLE_WIDTH;
	uint8_t* smearFlags			= gRowDitherStrides + threadNum * VISIBLE_WIDTH;

	for (int y = 0; y < numRows; y++)
	{
		FilterDithering_Row(indexed, smearFlags);

		for (int x = 0; x < VISIBLE_WIDTH-1; x++)
		{
			if (smearFlags[x])
			{
				uint8_t* left32		= (uint8_t*) &gGamePalette.finalColors32[indexed[0]];
				uint8_t* right32	= (uint8_t*) &gGamePalette.finalColors32[indexed[1]];

				uint16_t rmix8 = (left32[1] + right32[1]) >> 1;
				uint16_t gmix8 = (left32[2] + right32[2]) >> 1;
				uint16_t bmix8 = (left32[3] + right32[3]) >> 1;

#if FRAMEBUFFER_COLOR_DEPTH == 16
				*color = (rmix8 >> 3) | ((gmix8 >> 2) << 5) | ((bmix8 >> 3) << 11);
#elif FRAMEBUFFER_COLOR_DEPTH == 32
				((uint8_t*)color)[1] = rmix8;
				((uint8_t*)color)[2] = gmix8;
				((uint8_t*)color)[3] = bmix8;
#else
				_Static_assert(false, "unsupported framebuffer color depth");
#endif

				smearFlags[x] = 0;			// clear for next row
			}
			else
			{
				*color = gGamePalette.finalColorsXX[*indexed];
			}

			color++;
			indexed++;
		}

		*color = gGamePalette.finalColorsXX[*indexed];		// last
		color++;
		indexed++;
	}
}

static inline void FilterDithering_Row(const uint8_t* indexedRow, uint8_t* rowSmearFlags)
{
	static const int THRESH = 2;
	static const int BLEED = 1;

	int prev	= -1;
	int me		= indexedRow[0];
	int next	= indexedRow[1];

	int ditherStart		= 0;
	int ditherEnd		= -1;


#define COMMIT_STRIDE do { \
	int ditherLength = ditherEnd - ditherStart;								\
	if (ditherLength > THRESH)												\
		memset(rowSmearFlags+ditherStart, 1, ditherLength+BLEED);			\
	} while(0)

	for (int x = 0; x < VISIBLE_WIDTH-1; x++)
	{
		next = indexedRow[x+1];

		if (me==next || me==prev)	// contiguous solid color
		{
			COMMIT_STRIDE;			// 			commit current dither stride if any
			ditherEnd = -1;			// 			break dither stride
		}
		else if (prev==next)		// middle of dithered stride
		{
			if (ditherEnd < 0)		// 			no current dither stride yet
				ditherStart = x-1;	// 			start stride on left dither pixel
			ditherEnd = x+1;		// 			extend stride to right dither pixel
		}
		else if (x == ditherEnd)	// pixel was used to dither previous column
		{
			;						// 			let it be -- perhaps next pixel will detect we're still in dither stride
		}
		else						// lone non-dithered pixel
		{
			COMMIT_STRIDE;			// 			commit current dither stride if any
			ditherEnd = -1;			// 			break dither stride
		}

		prev = me;
		me = next;
	}

	// commit last
	COMMIT_STRIDE;

#undef COMMIT_STRIDE
}

void DoublePixels(const color_t* colorx1, color_t* colorx2, int firstRow, int numRows)
{
	colorx1		= colorx1 + firstRow * VISIBLE_WIDTH;
	colorx2		= colorx2 + firstRow * VISIBLE_WIDTH * 2 * 2;

	for (int y = 0; y < numRows; y++)
	{
		color_t* x2RowStart = colorx2;

		for (int x = 0; x < VISIBLE_WIDTH; x++)
		{
			color_t pixel = *(colorx1++);
			*(colorx2++) = pixel;
			*(colorx2++) = pixel;
		}

		memcpy(colorx2, x2RowStart, sizeof(color_t) * VISIBLE_WIDTH * 2);
		colorx2 += VISIBLE_WIDTH * 2;
	}
}
