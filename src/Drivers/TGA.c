// TGA.C
// (C) 2020 Iliyas Jorio
// This file is part of Bugdom. https://github.com/jorio/bugdom

#include "tga.h"
#include "misc.h"
#include "externs.h"

static void DecompressRLE(short refNum, TGAHeader* header, Handle handle)
{
	OSErr err = noErr;

	// Get number of bytes until EOF
	long pos = 0;
	long eof = 0;
	long compressedLength = 0;
	GetFPos(refNum, &pos);
	GetEOF(refNum, &eof);
	compressedLength = eof - pos;

	// Prep compressed data buffer
	Ptr compressedData = NewPtr(compressedLength);

	// Read rest of file into compressed data buffer
	err = FSRead(refNum, &compressedLength, compressedData);

	GAME_ASSERT(err == noErr);
	GAME_ASSERT(compressedLength == eof-pos);	// Ensure we got as many bytes as we asked for

	const long bytesPerPixel	= header->bpp / 8;
	const long pixelCount		= header->width * header->height;
	long pixelsProcessed		= 0;

	const uint8_t*			in  = (uint8_t*) compressedData;
	uint8_t*				out = (uint8_t*) *handle;
	const uint8_t* const	eod = in + compressedLength;

	while (pixelsProcessed < pixelCount)
	{
		GAME_ASSERT(in < eod);

		uint8_t packetHeader = *(in++);
		uint8_t packetLength = 1 + (packetHeader & 0x7F);

		GAME_ASSERT(pixelsProcessed + packetLength <= pixelCount);

		if (packetHeader & 0x80)		// Run-length packet
		{
			GAME_ASSERT(in + bytesPerPixel <= eod);
			for (int i = 0; i < packetLength; i++)
			{
				BlockMove(in, out, bytesPerPixel);
				out += bytesPerPixel;
				pixelsProcessed++;
			}
			in += bytesPerPixel;
		}
		else							// Raw packet
		{
			long packetBytes = packetLength * bytesPerPixel;
			GAME_ASSERT(in + packetBytes <= eod);
			BlockMove(in, out, packetBytes);
			in  += packetBytes;
			out += packetBytes;
			pixelsProcessed += packetLength;
		}
	}

	DisposePtr(compressedData);
}

static void FlipPixelData(Handle handle, TGAHeader* header)
{
	int rowBytes = header->width * (header->bpp / 8);

	Ptr topRow = *handle;
	Ptr bottomRow = topRow + rowBytes * (header->height - 1);
	Ptr topRowCopy = NewPtr(rowBytes);
	while (topRow < bottomRow)
	{
		BlockMove(topRow, topRowCopy, rowBytes);
		BlockMove(bottomRow, topRow, rowBytes);
		BlockMove(topRowCopy, bottomRow, rowBytes);
		topRow += rowBytes;
		bottomRow -= rowBytes;
	}
	DisposePtr(topRowCopy);
}

Handle LoadTGA(
		const char* path,
		bool loadPalette,
		int* outWidth,
		int* outHeight)
{
	short		refNum;
	OSErr		err;
	long		readCount;
	TGAHeader	header;
	Handle		pixelDataHandle;

	// Open data fork
	FSSpec spec;
	FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, path, &spec);
	err = FSpOpenDF(&spec, fsRdPerm, &refNum);
	if (err != noErr)
		return nil;

	// Read header
	readCount = sizeof(TGAHeader);
	err = FSRead(refNum, &readCount, (Ptr) &header);
	if (err != noErr || readCount != sizeof(TGAHeader))
	{
		FSClose(refNum);
		return nil;
	}

	// Make sure we support the format
	switch (header.imageType)
	{
		case TGA_IMAGETYPE_RAW_CMAP:
		case TGA_IMAGETYPE_RLE_CMAP:
			break;
		default:
			FSClose(refNum);
			DoFatalAlert2("TGA files must be colormapped!", path);
			return nil;
	}

	// Extract some info from the header
	Boolean compressed		= header.imageType & 8;
	Boolean needFlip		= 0 == (header.imageDescriptor & (1u << 5u));
	long pixelDataLength	= header.width * header.height * (header.bpp / 8);

	// Ensure there's no identification field -- we don't support that
	GAME_ASSERT(header.idFieldLength == 0);

	// If there's palette data, load it in
	if (header.imageType == TGA_IMAGETYPE_RAW_CMAP || header.imageType == TGA_IMAGETYPE_RLE_CMAP)
	{
		uint16_t paletteColorCount	= header.paletteColorCountLo | ((uint16_t)header.paletteColorCountHi << 8);
		const long paletteBytes		= paletteColorCount * (header.paletteBitsPerColor / 8);

		GAME_ASSERT(8 == header.bpp);
		GAME_ASSERT(header.paletteOriginLo == 0 && header.paletteOriginHi == 0);
		GAME_ASSERT(paletteColorCount <= 256);

		if (!loadPalette)
		{
			SetFPos(refNum, fsFromMark, paletteBytes);
		}
		else
		{
			Ptr palette = NewPtr(paletteBytes);

			readCount = paletteBytes;
			FSRead(refNum, &readCount, palette);
			GAME_ASSERT(readCount == paletteBytes);

			for (int i = 0; i < paletteColorCount; i++)
			{
				uint8_t red   = palette[i*3 + 2];
				uint8_t green = palette[i*3 + 1];
				uint8_t blue  = palette[i*3 + 0];
				gGamePalette[i] = 0x000000FF
								  | (red << 24)
								  | (green << 16)
								  | (blue << 8);
			}

			DisposePtr(palette);
		}
	}

	// Allocate pixel data
	pixelDataHandle = NewHandle(pixelDataLength);

	// Read pixel data; decompress it if needed
	if (compressed)
	{
		DecompressRLE(refNum, &header, pixelDataHandle);
		header.imageType &= ~8;		// flip compressed bit
	}
	else
	{
		readCount = pixelDataLength;
		err = FSRead(refNum, &readCount, *pixelDataHandle);
		GAME_ASSERT(readCount == pixelDataLength);
		GAME_ASSERT(err == noErr);
	}

	// Close file -- we don't need it anymore
	FSClose(refNum);

	// If pixel data is stored bottom-up, flip it vertically.
	if (needFlip)
	{
		FlipPixelData(pixelDataHandle, &header);

		// Set top-left origin bit
		header.imageDescriptor |= (1u << 5u);
	}

	// Store result
	if (outWidth != nil)
		*outWidth = header.width;
	if (outHeight != nil)
		*outHeight = header.height;

	return pixelDataHandle;
}
