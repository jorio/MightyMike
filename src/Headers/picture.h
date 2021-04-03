//
// Picture.h
//

enum
{
	LOADIMAGE_FADEIN		= 1 << 0,
	LOADIMAGE_ALIGNBOTTOM	= 1 << 1,
	LOADIMAGE_BACKGROUND	= 1 << 2,
};


void	LoadBackground(const char* filename);
void	LoadImage(const char* filename, short showMode);
void	LoadBorderImage(void);
