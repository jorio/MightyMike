//
// Picture.h
//

enum {
	SHOW_IMAGE_MODE_FADEIN,
	SHOW_IMAGE_MODE_QUICK,
	SHOW_IMAGE_MODE_NOSHOW
};


extern void	LoadBackground(Str255, Boolean);
extern void	LoadBackground_Direct(Str255, Boolean);
extern void	LoadIMAGE(Str255, short);
extern void	LoadBorderImage(void);
