/*
 *  Copyright (c) 1995 Andrew Dean
 */

#include <exec/memory.h>
#include "/includes.h"

#define	MAX_SPEED	6

#define SPEED	0
#define SIZE	2
#define MODE	4

Triplet *ColorTable = 0L;

#include "Patch_rev.h"
STATIC const UBYTE VersTag[] = VERSTAG;

LONG	ScreenHeight;
LONG	ScreenWidth;
LONG	Colors;

VOID Defaults (PrefObject *Prefs)
{
	Prefs[SPEED].po_Level = 6;
	Prefs[SIZE].po_Active = 32;
	Prefs[MODE].po_ModeID = getTopScreenMode();
}

VOID DrawPatches (struct RastPort *RP, PrefObject *mP)
{
	LONG	X, Y, Col, Index;
	LONG	PatchSize = mP[SIZE].po_Active;
	LONG	Hrepls; /* horizontal replications */
	LONG	Vrepls; /* vertical replications */

	Hrepls = ScreenWidth / PatchSize;
	Vrepls = ScreenHeight / PatchSize;
	X = RangeRand (PatchSize);
	Y = RangeRand (PatchSize);
	Col = RangeRand (Colors);

	/* draw pixel and its 7 relations */
	SetAPen (RP, Col);
	WritePixel (RP, X, Y);
	WritePixel (RP, X, PatchSize - Y);
	WritePixel (RP, PatchSize - X, Y);
	WritePixel (RP, PatchSize - X, PatchSize - Y);
	WritePixel (RP, Y, X);
	WritePixel (RP, PatchSize - Y, X);
	WritePixel (RP, Y, PatchSize - X);
	WritePixel (RP, PatchSize - Y, PatchSize - X);

	/* replicate bitmap */
	for (Index = 1L; Index < Hrepls; Index++)
	{
		BltBitMap (RP->BitMap, 0, 0, RP->BitMap, PatchSize * Index, 0, PatchSize, PatchSize, 0xc0, 0xff, 0L);
	}
	for (Index = 1L; Index < Vrepls; Index++)
	{
		BltBitMap (RP->BitMap, 0, 0, RP->BitMap, 0, PatchSize * Index, ScreenWidth, PatchSize, 0xc0, 0xff, 0L);
	}
}

LONG Blank (PrefObject *Prefs)
{
	LONG Count, ScrToFrontCnt = 0, RetVal = TRUE;
	struct Screen *PatchScreen;

	PatchScreen = OpenScreenTags (NULL,
					SA_Depth, 4,
					SA_Overscan, OSCAN_STANDARD,
					SA_DisplayID, Prefs[MODE].po_ModeID,
					SA_Quiet, TRUE,
					SA_Behind, TRUE,
					TAG_DONE);
	if (PatchScreen)
	{
		ScreenWidth = PatchScreen->Width;
		ScreenHeight = PatchScreen->Height;

		SetRGB4 (&(PatchScreen->ViewPort), 0, 0, 0, 0);
		SetRast (&(PatchScreen->RastPort), 0);
		Colors = (1L << PatchScreen->RastPort.BitMap->Depth) - 1;
		ColorTable = RainbowPalette (PatchScreen, 0L, 1L, 0L);
		ScreenToFront (PatchScreen);
		Count = Prefs[SPEED].po_Level + 1;

		while (RetVal == TRUE)
		{
			WaitTOF();

			if (!(ScrToFrontCnt++ % 60))
			{
				ScreenToFront (PatchScreen);
			}

			if (++Count > MAX_SPEED)
			{
				Count = Prefs[SPEED].po_Level + 1;
				DrawPatches (&(PatchScreen->RastPort), Prefs);
			}
			RetVal = ContinueBlanking();
		}
		CloseScreen (PatchScreen);
	}
	else
	{
		RetVal = FALSE;
	}

	return RetVal;
}
