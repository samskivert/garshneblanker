/*
 *  Copyright (c) 1995 Andrew Dean
 *  All rights reserved, probably.
 *
 */

#include <exec/memory.h>
#include "/includes.h"

#define MAX_SPEED	6

#define SPEED		0
#define NO_LINES	2
#define MODE		4

Triplet *ColorTable = 0L;

#include "Kal_rev.h"
STATIC const UBYTE VersTag[] = VERSTAG;

typedef struct	Line_t
{
	LONG	X;
	LONG	Y;
	LONG	X2;
	LONG	Y2;
} LINET;

LONG	HalfHeight;
LONG	HalfWidth;
LONG	Colors;
LINET	*Line;

VOID Defaults (PrefObject *Prefs)
{
	Prefs[SPEED].po_Level = 6;
	Prefs[NO_LINES].po_Active = 12;
	Prefs[MODE].po_ModeID = getTopScreenMode();
}

VOID KillLine (struct RastPort *RP, PrefObject *mp, LONG Index)
{
	/* assuming ink 0 is black */
	SetAPen (RP, 0);
	Move (RP, HalfWidth + Line[Index].X, HalfHeight + Line[Index].Y);
	Draw (RP, HalfWidth + Line[Index].X2, HalfHeight + Line[Index].Y2);
	Move (RP, HalfWidth - Line[Index].X, HalfHeight + Line[Index].Y);
	Draw (RP, HalfWidth - Line[Index].X2, HalfHeight + Line[Index].Y2);
	Move (RP, HalfWidth + Line[Index].X, HalfHeight - Line[Index].Y);
	Draw (RP, HalfWidth + Line[Index].X2, HalfHeight - Line[Index].Y2);
	Move (RP, HalfWidth - Line[Index].X, HalfHeight - Line[Index].Y);
	Draw (RP, HalfWidth - Line[Index].X2, HalfHeight - Line[Index].Y2);
	Move (RP, HalfWidth + Line[Index].Y, HalfHeight + Line[Index].X);
	Draw (RP, HalfWidth + Line[Index].Y2, HalfHeight + Line[Index].X2);
	Move (RP, HalfWidth - Line[Index].Y, HalfHeight + Line[Index].X);
	Draw (RP, HalfWidth - Line[Index].Y2, HalfHeight + Line[Index].X2);
	Move (RP, HalfWidth + Line[Index].Y, HalfHeight - Line[Index].X);
	Draw (RP, HalfWidth + Line[Index].Y2, HalfHeight - Line[Index].X2);
	Move (RP, HalfWidth - Line[Index].Y, HalfHeight - Line[Index].X);
	Draw (RP, HalfWidth - Line[Index].Y2, HalfHeight - Line[Index].X2);
}

VOID DrawLine (struct RastPort *RP, PrefObject *mP, LONG Size, LONG Index)
{
	LONG	Col;

	Line[Index].X = RangeRand (Size * 2) - Size;
	Line[Index].Y = RangeRand (Size * 2) - Size;
	Line[Index].X2 = RangeRand (Size * 2) - Size;
	Line[Index].Y2 = RangeRand (Size * 2) - Size;
	Col = RangeRand (Colors);

	SetAPen (RP, Col);
	Move (RP, HalfWidth + Line[Index].X, HalfHeight + Line[Index].Y);
	Draw (RP, HalfWidth + Line[Index].X2, HalfHeight + Line[Index].Y2);
	Move (RP, HalfWidth - Line[Index].X, HalfHeight + Line[Index].Y);
	Draw (RP, HalfWidth - Line[Index].X2, HalfHeight + Line[Index].Y2);
	Move (RP, HalfWidth + Line[Index].X, HalfHeight - Line[Index].Y);
	Draw (RP, HalfWidth + Line[Index].X2, HalfHeight - Line[Index].Y2);
	Move (RP, HalfWidth - Line[Index].X, HalfHeight - Line[Index].Y);
	Draw (RP, HalfWidth - Line[Index].X2, HalfHeight - Line[Index].Y2);
	Move (RP, HalfWidth + Line[Index].Y, HalfHeight + Line[Index].X);
	Draw (RP, HalfWidth + Line[Index].Y2, HalfHeight + Line[Index].X2);
	Move (RP, HalfWidth - Line[Index].Y, HalfHeight + Line[Index].X);
	Draw (RP, HalfWidth - Line[Index].Y2, HalfHeight + Line[Index].X2);
	Move (RP, HalfWidth + Line[Index].Y, HalfHeight - Line[Index].X);
	Draw (RP, HalfWidth + Line[Index].Y2, HalfHeight - Line[Index].X2);
	Move (RP, HalfWidth - Line[Index].Y, HalfHeight - Line[Index].X);
	Draw (RP, HalfWidth - Line[Index].Y2, HalfHeight - Line[Index].X2);
}

LONG Blank (PrefObject *Prefs)
{
	LONG Count, ScrToFrontCnt = 0, RetVal = TRUE, Size;
	LONG	Number;
	struct Screen *PatchScreen;

	PatchScreen = OpenScreenTags (NULL,
					SA_Depth, 3,
					SA_Overscan, OSCAN_STANDARD,
					SA_DisplayID, Prefs[MODE].po_ModeID,
					SA_Quiet, TRUE,
					SA_Behind, TRUE,
					TAG_DONE);
	Size = sizeof (LINET) * Prefs[NO_LINES].po_Active;
	Line = (LINET *) AllocVec (Size, 0L);

	if (PatchScreen && Line)
	{
		HalfWidth = PatchScreen->Width / 2;
		HalfHeight = PatchScreen->Height / 2;
		if (HalfWidth < HalfHeight)
		{
			Size = HalfWidth - 1;
		}
		else
		{
			Size = HalfHeight - 1;
		}

		SetRGB4 (&(PatchScreen->ViewPort), 0, 0, 0, 0);
		SetRast (&(PatchScreen->RastPort), 0);
		Colors = (1L << PatchScreen->RastPort.BitMap->Depth) - 1;
		ColorTable = RainbowPalette (PatchScreen, 0L, 1L, 0L);
		ScreenToFront (PatchScreen);
		Count = Prefs[SPEED].po_Level + 1;

		for (Number = 0 ; Number < Prefs[NO_LINES].po_Active ; Number++)
		{
			DrawLine (&(PatchScreen->RastPort), Prefs, Size, Number);
		}
		Number = 0;
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
				Number++;
				if (Number == Prefs[NO_LINES].po_Active)
				{
					Number = 0;
				}
				KillLine (&(PatchScreen->RastPort), Prefs, Number);
				DrawLine (&(PatchScreen->RastPort), Prefs, Size, Number);
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
