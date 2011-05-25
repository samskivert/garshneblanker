/*
 *  Copyright (c) 1994 Michael D. Bayne.
 *  All rights reserved.
 *
 *  Please see the documentation accompanying the distribution for distribution
 *  and disclaimer information.
 */

#include <exec/memory.h>
#include "/includes.h"

UWORD Table4[] = {
	0x0000, 0x0B06, 0x0909, 0x060B,
	0x030E, 0x003E, 0x006B, 0x0099,
	0x00B6, 0x00E3, 0x03E0, 0x06B0,
	0x0990, 0x0B60, 0x0E30, 0x0E03
	};

#ifndef min
#define min( x, y ) ( x < y ? x : y )
#endif

#define BLOCK_DRAW 1
#ifdef BLOCK_DRAW
#define DrawSquare( i, j )\
	RectFill( &Scr->RastPort, offx + side * (i-1), offy + side * (j-1),\
			 offx + side * (i-1) + side - 2, offy + side * (j-1) + side - 2 )
#else
#define DrawSquare( i, j )\
	WritePixel( &Scr->RastPort, offx + side * (i-1), offy + side * (j-1))
#endif


VOID Defaults( PrefObject *Prefs )
{
	Prefs[0].po_Level = 4;
	Prefs[2].po_ModeID = getTopScreenMode();
}

LONG Blank( PrefObject *Prefs )
{
	LONG i, j, hei, wid, ewid, ehei, size, stable = 0, side, offx, offy;
	LONG *CurPhase, *OldPhase, *TmpPhase, Pos, Species, Parent;
	LONG Width, Height, RetVal = OK;
	struct Screen *Scr;
	struct Window *Wnd;
	
	Scr = OpenScreenTags( 0L, SA_DisplayID, Prefs[2].po_ModeID, SA_Quiet, TRUE,
						 SA_Overscan, OSCAN_STANDARD, SA_Depth, 4,
						 SA_ShowTitle, FALSE, SA_Title, "Garshnescreen",
						 TAG_DONE );
	if( !Scr )
	{
		RetVal = FAILED;
		goto FAIL;
	}
	
	LoadRGB4( &Scr->ViewPort, Table4, 16 );
	Width = Scr->Width;
	Height = Scr->Height;
	hei = Height / Prefs[0].po_Level;
	wid = Width / Prefs[0].po_Level;
	ewid = wid + 2;
	ehei = hei + 2;
	size = ewid * ehei;
	side = Height / hei;
	offx = ( Width - side * wid ) / 2;
	offy = ( Height - side * hei ) / 2;
	
	CurPhase = AllocVec( sizeof( LONG ) * size, MEMF_ANY );
	OldPhase = AllocVec( sizeof( LONG ) * size, MEMF_ANY );
	
	if( !CurPhase || !OldPhase )
	{
		RetVal = FAILED;
		goto FAIL;
	}

	Wnd = BlankMousePointer( Scr );
	ScreenToFront( Scr );
		
	while( RetVal == OK )
	{
		for( j = 1; ( j <= hei )&&( RetVal == OK ); j++ )
		{
			for( i = 1; i <= wid; i++ )
			{
				Pos = j * ewid + i;
				CurPhase[Pos] = OldPhase[Pos] = RangeRand( 15 ) + 1;
				SetAPen( &Scr->RastPort, CurPhase[Pos] );
				DrawSquare( i, j );
				if(!( Pos % 200 ))
					if(( RetVal = ContinueBlanking()) != OK )
						break;
			}
		}	

		do
		{
			stable++;
			CopyMemQuick( OldPhase+size-(ewid<<1), CurPhase, ewid<<2 );
			CopyMemQuick( OldPhase+ewid, CurPhase+ewid, (size-(ewid<<1))<<2 );
			CopyMemQuick( OldPhase+ewid, CurPhase+size-ewid, ewid<<2 );
			for( i = 0, j = ewid-2; i < size; i += ewid, j += ewid )
				CurPhase[i] = OldPhase[j];
			for( i = ewid-1, j = 1; i < size; i += ewid, j += ewid )
				CurPhase[i] = OldPhase[j];

			for( j = 1; ( j <= hei )&&( RetVal == OK ); j++ )
			{
				for( i = 1; i <= wid; i++ )
				{
					Pos = j * ewid + i;
					Species = CurPhase[Pos];
					Parent = (( Species+1 )%15 )+1;
					if(( CurPhase[Pos-1] == Parent )||
					   ( CurPhase[Pos+1] == Parent )||
					   ( CurPhase[Pos-ewid] == Parent )||
					   ( CurPhase[Pos+ewid] == Parent ))
					{
						SetAPen( &Scr->RastPort, OldPhase[Pos] = Parent );
						DrawSquare( i, j );
						stable = 0;
					}
				}
				RetVal = ContinueBlanking();
			}
			TmpPhase = OldPhase;
			OldPhase = CurPhase;
			CurPhase = TmpPhase;
		}
		while(( stable < 2 )&&( RetVal == OK ));

		for( i = 0; i < 50 && RetVal == OK; i++ )
		{
			Delay( 2 );
			RetVal = ContinueBlanking();
		}
	}
	UnblankMousePointer( Wnd );
	
 FAIL:
	if( CurPhase )
		FreeVec( CurPhase );
	if( OldPhase )
		FreeVec( OldPhase );	
	if( Scr )
		CloseScreen( Scr );

	return RetVal;
}
