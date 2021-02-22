/****************************/
/*    PALETTE MANAGER       */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/
#include "myglobals.h"
//#include <pictutils.h>
#include "misc.h"

extern	Handle		gShapeTableHandle[];
extern	char  			gMMUMode;

/****************************/
/*    CONSTANTS             */
/****************************/

#define	PALOFF	0

/**********************/
/*     VARIABLES      */
/**********************/

PaletteHandle	gGamePalette = nil;
static	PaletteHandle	gBackUpPalette = nil;

static	ColorSpec 		gDefaultCLUT[256];

short			gColorListSize;

static	Boolean			gSceenBlankedFlag = false;


/********************** INIT PALETTE STUFF ****************/

void InitPaletteStuff(void)
{
short	i;
short err;

	gGamePalette = NewPalette(256,nil,pmTolerant+pmExplicit,0);		// create empty palette
	gBackUpPalette = NewPalette(256,nil,pmTolerant+pmExplicit,0);	// create empty palette

	for (i=0; i<255; i++)				// deprotect all colors
	{
		ReserveEntry(i,false);
		err = QDError();
		if (err)
			ShowSystemErr(err);

		ProtectEntry(i,false);
		err = QDError();
		if (err)
			ShowSystemErr(err);
	}
}


/****************************** BUILD SHAPE PALETTE ********************/
//
// This must be the 1st thing done to the new game palette.  Assumes all clear.
//

void BuildShapePalette(Byte	groupNum)
{
long 	*cOffPtr;
short		i,*intPtr;
RGBColor	rgbColor,*colorEntryPtr;

	cOffPtr = (long *)(*gShapeTableHandle[groupNum]);				// get ptr to Color Table offset

	intPtr = (short *)((*cOffPtr)+(*gShapeTableHandle[groupNum]));	// get ptr to color header
	gColorListSize = *intPtr++;							// # entries in color list
	colorEntryPtr = (RGBColor *)intPtr;					// point to color list


					/* BUILD THE PALETTE */

	for (i=0; i<gColorListSize; i++)
	{
		rgbColor = *colorEntryPtr++;					// get color
		SetEntryColor(gGamePalette,i,&rgbColor);		// set color
	}

				/* IF <256, THEN FORCE LAST COLOR TO BLACK */

	if (gColorListSize<256)
	{
		rgbColor.red = rgbColor.blue = rgbColor.green = 0x0000;
		SetEntryColor(gGamePalette,255,&rgbColor);		// set color
	}
}



/************************ RESTORE DEFAULT CLUT **********************/
//
// Actually just de-reserves all of the CLUT entries
//

void	RestoreDefaultCLUT()
{
short	i;

	for (i=0; i<256; i++)
	{
		ReserveEntry(i,false);
		ProtectEntry(i,false);							// make accessable
	}

	RestoreDeviceClut(nil);
	gSceenBlankedFlag = false;
}


/************************ ACTIVATE CLUT ********************/

void ActivateCLUT(void)
{
ColorSpec aTable[256];
short		i;
RGBColor	rgbColor;
short	err;


					/* BUILD THE CLUT */

	for (i=0; i<255; i++)
	{
		ReserveEntry(i,false);
		err = QDError();
		if (err)
			ShowSystemErr(err);

		ProtectEntry(i,false);							// make accessable
		err = QDError();
		if (err)
			ShowSystemErr(err);

		GetEntryColor(gGamePalette,i,&rgbColor);		// get color from palette
		aTable[i].rgb = rgbColor;
		aTable[i].value = i;
	}
se1:
		SetEntries(0,255-1,aTable);						// use it
		err = QDError();
		if (err)
		{
			ResetScreen();
			goto se1;
		}

				/* LOCK THE CLUT */

	for (i=0; i<255; i++)
	{
		ProtectEntry(i,true);							// lock it there!
		err = QDError();
		if (err)
			ShowSystemErr(err);
	}

gSceenBlankedFlag = false;
}


/************************ FADE IN GAME CLUT ********************/

void FadeInGameCLUT(void)
{
ColorSpec 	aTable[256];
short			i;
RGBColor	rgbColor;
short		err;
short 		brightness;
long		red,green,blue;

						/* FADE IN THE CLUT */

	for	(brightness = 4; brightness <= 100; brightness += 8)
	{
		for (i=0; i<255; i++)
		{
			ReserveEntry(i,false);
			err = QDError();
			if (err)
				ShowSystemErr(err);

			ProtectEntry(i,false);							// make accessable
			err = QDError();
			if (err)
				ShowSystemErr(err);

			GetEntryColor(gGamePalette,i,&rgbColor);		// get color from palette
			red = rgbColor.red;
			green = rgbColor.green;
			blue = rgbColor.blue;

			rgbColor.red = (short)((red*brightness)/100);
			rgbColor.green = (short)((green*brightness)/100);
			rgbColor.blue = (short)((blue*brightness)/100);
			aTable[i].rgb = rgbColor;
			aTable[i].value = i;

		}
se1:
		SetEntries(0,255-1,aTable);							// use it
		err = QDError();
		if (err)
		{
			ResetScreen();
			goto se1;
		}
	}

					/* LOCK THE CLUT */

	for (i=0; i<255; i++)
	{
		ProtectEntry(i,true);								// lock it there!
		err = QDError();
		if (err)
			ShowSystemErr(err);
	}

						/* RESET GAME WINDOW'S PALETTE */

	gSceenBlankedFlag = false;
}

/*********************** ERASE CLUT **********************/

void EraseCLUT(void)
{
ColorSpec	aTable[256];
short			i;
RGBColor	rgbColor;
short		err;

#if PALOFF
	return;	//----------
#endif
					/* ZAP 0..254 */

	rgbColor.red = rgbColor.green = rgbColor.blue = 0;

	for (i=0; i<255; i++)
	{
		ReserveEntry(i,false);
		err = QDError();
		if (err)
			ShowSystemErr(err);

		ProtectEntry(i,false);					// make accessable
		err = QDError();
		if (err)
			ShowSystemErr(err);
		aTable[i].rgb = rgbColor;				// assign color
		aTable[i].value = 0;
	}
se1:
	SetEntries(0,255-1,aTable);					// use them
	err = QDError();
	if (err)
	{
		ResetScreen();
		goto se1;
	}

	gSceenBlankedFlag = true;
}



/******************* SEE IF COLOR IS IN GAME PALETTE *******************/

Boolean ColorInGamePalette(RGBColor rgb)
{
static	short	i;
static	RGBColor rgb2;

	for (i=0; i<gColorListSize; i++)			// see if color already in palette
	{
		GetEntryColor(gGamePalette,i,&rgb2);	// get gamepal color
		if ((rgb2.red == rgb.red) &&
			(rgb2.green == rgb.green) &&
			(rgb.blue == rgb2.blue))
			return(true);
	}

	return(false);
}


/************************ FADE OUT GAME CLUT ********************/
//
// BASES IT ON THE BACKUP PALETTE!!!!!!!!
//

void FadeOutGameCLUT(void)
{
register  long	red,green,blue;
ColorSpec 	aTable[256];
short			i;
RGBColor	rgbColor;
short		err;
short 		brightness;

#if PALOFF
	return;	//----------
#endif

	if (gSceenBlankedFlag)									// see if already out
		return;


	MakeBackUpPalette();									// (needs backup pal to do fade)

						/* FADE OUT THE CLUT */

	for	(brightness = 96; brightness >= 0; brightness -= 8)
	{
		for (i=0; i<255; i++)
		{
			ReserveEntry(i,false);
			err = QDError();
			if (err)
				ShowSystemErr(err);

			ProtectEntry(i,false);							// make accessable
			err = QDError();
			if (err)
				ShowSystemErr(err);

			GetEntryColor(gBackUpPalette,i,&rgbColor);		// get color from palette
			red = rgbColor.red;
			green = rgbColor.green;
			blue = rgbColor.blue;

			rgbColor.red = (short)((red*brightness)/100);
			rgbColor.green = (short)((green*brightness)/100);
			rgbColor.blue = (short)((blue*brightness)/100);
			aTable[i].rgb = rgbColor;
			aTable[i].value = i;

		}
se1:
		SetEntries(0,255-1,aTable);			// use it
		err = QDError();
		if (err)
		{
			ResetScreen();
			goto se1;
		}
	}


					/* LOCK THE CLUT */

	for (i=0; i<255; i++)
	{
		ProtectEntry(i,true);								// lock it there!
		err = QDError();
		if (err)
			ShowSystemErr(err);
	}


						/* RESET GAME WINDOW'S PALETTE */

	gSceenBlankedFlag = true;

}


/********************* MAKE BACKUP PALETTE *******************/

void MakeBackUpPalette(void)
{

	CopyPalette(gGamePalette,gBackUpPalette,0,0,255);

}






