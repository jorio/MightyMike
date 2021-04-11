// FONT
// (C) 2021 Iliyas Jorio
// This file is part of Mighty Mike. https://github.com/jorio/mightymike

#include "myglobals.h"
#include "objecttypes.h"
#include "object.h"
#include "misc.h"
#include "shape.h"
#include "io.h"
#include "main.h"
#include "externs.h"
#include "font.h"

#define kSpecialLetterMagicValue 'LTTR'		// stored in an ObjNode's Special0 to signify that it is a letter node

#define SpecialLetterMagic		Special0
#define SpecialLetterStringID	Special1
#define SpecialLetterBaseY		Special2
#define FlagLetterJitter		Flag0

static const int8_t kLetterWidths[] =
{
	20,20,20,20, 20,20,20,20, 20,20,20,20, 20,20,20,20,
	20,20,20,20, 20,20,20,20, 20,20,20,20, 20,20,20,20,
	 6, 5,13,13, 13,13,13, 5, 13,13,13,13,  5,13, 5,13,	//  !"# $%&' ()*+ ,-./
	12,12,12,12, 12,12,12,12, 12,12, 5, 5, 13,13,13,13,	// 0123 4567 89:; <=>?
	20,20,20,20, 20,20,20,20, 20,20,20,20, 20,20,20,20,	// @ABC DEFG HIJK LMNO
	20,20,20,20, 20,20,20,20, 20,20,20,20, 20,20,20,20,	// PQRS TUVW XYZ
	18,13,13,11, 15,11,11,18, 13, 5, 9,13, 10,19,15,18,	//  abc defg hijk lmno
	13,18,13,12, 11,13,15,19, 14,12,13,20, 20,20,20,20,	// pqrs tuvw xyz
};

/******************* ASCII TO BIG FONT **********************/
//
// Converts ASCII char to BigFont subtype #
//

Byte ASCIIToBigFont(char c)
{
	switch (c)
	{
		case '!':		return 80;
		case '?':		return 83;
		case '.':		return 38;
		case ':':		return 79;
		case '@':		return 78;
		case ',':		return 82;
	}

	if (c >= '0' && c <= '9')
		return c - '0' + 68;

	if (c >= 'A' && c <= 'Z')			// multicolor letters
		return c - 'A' + 11;

	if (c >= 'a' && c <= 'z')			// red letters
		return c - 'a' + 42;

	return 0;
}

static void MoveText(void)
{
	GetObjectInfo();

	ObjNode* theNode = gThisNodePtr;

	theNode->DY += 0x10000L / 4;						// add gravity

	theNode->YOffset.L += theNode->DY;					// move it

	if (theNode->YOffset.Int > 0)						// see if bounce
	{
		theNode->YOffset.Int = 0;						// rebound

		if (theNode->FlagLetterJitter)
		{
			theNode->DY = -0x10000 * RandomRange(2, 4) / 2;
		}
		else
		{
			theNode->DY = -theNode->DY / 2;
			if (theNode->DY > -0x10000)					// don't rebound forever
			{
				theNode->DY = 0;
				theNode->MoveFlag = false;
			}
		}
	}

	gThisNodePtr->Y.L = theNode->SpecialLetterBaseY + theNode->YOffset.L;		// move up

//	UpdateObject();
}

int MakeText(const char* label, int x, int y, int flags, long stringID)
{
	if (!label)
		label = "NULL???";

	bool asObject		= flags & kTextFlags_AsObject;
	bool bounceUp		= flags & kTextFlags_BounceUp;
	bool jitter			= flags & kTextFlags_Jitter;

	GAME_ASSERT(asObject || !bounceUp);

	for (const char* c = label; *c; c++)
	{
		unsigned char cc = *c;

		if (cc == ' ')
		{
			x += kLetterWidths[' '];
			continue;
		}

		Byte frameID = ASCIIToBigFont(cc);
		if (frameID != 0)	// TODO: why do we have to decrement frameID to draw non-animated frames, but not for MakeNewShape?
			frameID--;

		const FrameHeader* fh = GetFrameHeader(GroupNum_BigFont, ObjType_BigFont, frameID, nil, nil);

		int myWidth = 13;
		int cancelXOff = -fh->x;
		int cancelYOff = 0;

		if (cc < sizeof(kLetterWidths) / sizeof(kLetterWidths[0]))
		{
			myWidth = kLetterWidths[cc];
		}

		if (cc >= 'a' && cc <= 'z')
		{
			cancelXOff = -fh->x;
			cancelYOff = -fh->y - 9;
		}
		else if (cc >= 'A' && cc <= 'Z')
		{
			cancelXOff = 0;
			cancelYOff = 0;
		}

		if (asObject)
		{
			ObjNode* newObj = MakeNewShape(
					GroupNum_BigFont,
					ObjType_BigFont,
					ASCIIToBigFont(cc),
					x + cancelXOff,
					y + cancelYOff,
					0x0080,
					MoveText,
					false);
			GAME_ASSERT_MESSAGE(newObj, "Too many objects on screen!");

			newObj->SpecialLetterMagic		= kSpecialLetterMagicValue;
			newObj->SpecialLetterStringID	= stringID;
			newObj->SpecialLetterBaseY		= (y + cancelYOff) << 16;
			newObj->FlagLetterJitter		= jitter;

			if (bounceUp)
			{
				newObj->DY = -0x10000 * RandomRange(3, 6) / 2;
			}
		}
		else
		{
			DrawFrameToBackground(x + cancelXOff, y + cancelYOff, GroupNum_BigFont, ObjType_BigFont, frameID);
		}

		x += myWidth + 1;
	}

	return x;
}

void DeleteAllText(void)
{
	ObjNode* theNode = FirstNodePtr;

	while (theNode)
	{
		ObjNode* next = theNode->NextNode;

		if (theNode->SpecialLetterMagic == kSpecialLetterMagicValue)
		{
			DeleteObject(theNode);
		}

		theNode = next;
	}
}

void DeleteText(long stringID)
{
	ObjNode* theNode = FirstNodePtr;

	while (theNode)
	{
		ObjNode* next = theNode->NextNode;

		if (theNode->SpecialLetterMagic == kSpecialLetterMagicValue
			&& theNode->SpecialLetterStringID == stringID)
		{
			DeleteObject(theNode);
		}

		theNode = next;
	}
}
