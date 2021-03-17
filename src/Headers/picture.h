//
// Picture.h
//

enum
{
	SHOW_IMAGE_FLAG_FADEIN		= 1 << 0,
	SHOW_IMAGE_FLAG_ALIGNBOTTOM = 1 << 1,
};


void	LoadBackground(const char* filename, Boolean getPalFlag);
void	LoadIMAGE(const char* filename, short showMode);
void	LoadBorderImage(void);
