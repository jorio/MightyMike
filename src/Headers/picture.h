//
// Picture.h
//

enum
{
	SHOW_IMAGE_FLAG_FADEIN		= 1 << 0,
	SHOW_IMAGE_FLAG_ALIGNBOTTOM = 1 << 1,
};


extern void	LoadBackground(Str255, Boolean);
extern void	LoadIMAGE(Str255, short);
extern void	LoadBorderImage(void);
