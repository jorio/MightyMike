#pragma once

enum
{
	kTextFlags_AsObject			= 1 << 0,
	kTextFlags_BounceUp			= 1 << 1,
	kTextFlags_Jitter			= 1 << 2,
};

Byte ASCIIToBigFont(char);
int MakeText(const char* label, int x, int y, int flags, long stringID);
void DeleteAllText(void);
void DeleteText(long stringID);
