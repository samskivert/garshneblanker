/*
 *  Copyright (c) 1994 Michael D. Bayne.
 *  All rights reserved.
 *
 *  Please see the documentation accompanying the distribution for distribution
 *  and disclaimer information.
 */

#include <exec/memory.h>
#include <hardware/custom.h>
#include "/includes.h"

#define TRAIL  0
#define LINES  2
#define XSPEED 4
#define YSPEED 6
#define COLORS 8
#define MODE   10

extern __far struct Custom custom;
Triplet *ColorTable = 0L;

#define RAND( base, offset ) (( LONG )( RangeRand( base ) + offset ))

#include "Moiré_rev.h"
STATIC const UBYTE VersTag[] = VERSTAG;

VOID Defaults( PrefObject *Prefs )
{
	Prefs[TRAIL].po_Level = 64;
	Prefs[LINES].po_Level = 2;
	Prefs[XSPEED].po_Level = 16;
	Prefs[YSPEED].po_Level = 16;
	Prefs[COLORS].po_Active = 3;
	Prefs[MODE].po_ModeID = getTopScreenMode();
}

LONG Blank( PrefObject *Prefs )
{
	LONG numCols, i, j, k, n, Trail, xSpeed, ySpeed, Wid, Hei, **cx, **cy, *dx;
	LONG ToFrontCount = 0, RetVal = OK,	*dy, nl;
	struct Screen *Scr;
	struct Window *Wnd;
	struct RastPort *rp;
	struct ViewPort *vp;

	xSpeed = Prefs[XSPEED].po_Level;
	ySpeed = Prefs[YSPEED].po_Level;
	nl = Prefs[LINES].po_Level;
	Trail = Prefs[TRAIL].po_Level;

	cx = AllocVec( nl * sizeof( LONG * ), MEMF_CLEAR );
	cy = AllocVec( nl * sizeof( LONG * ), MEMF_CLEAR );
	dx = AllocVec( nl * sizeof( LONG ), MEMF_CLEAR );
	dy = AllocVec( nl * sizeof( LONG ), MEMF_CLEAR );

	if( cx && cy )
	{
		for( k = 0; k < nl; k++ )
		{
			if(!( cx[k] = AllocVec( Trail * sizeof( LONG ), MEMF_CLEAR )))
				break;
			if(!( cy[k] = AllocVec( Trail * sizeof( LONG ), MEMF_CLEAR )))
				break;
		}
	}
		
	Scr = OpenScreenTags( 0l, SA_DisplayID, Prefs[MODE].po_ModeID,
						 SA_Depth, Prefs[MODE].po_Depth, SA_Quiet, TRUE,
						 SA_Behind, TRUE, SA_Overscan, OSCAN_STANDARD,
						 SA_ShowTitle, FALSE, SA_Title, "Garshnescreen",
						 TAG_DONE );
	
	if( cx && cy && dx && dy &&( k == nl )&& Scr )
	{
		rp = &( Scr->RastPort );
		vp = &( Scr->ViewPort );
		Wid = Scr->Width;
		Hei = Scr->Height;
		
		numCols = 1 << rp->BitMap->Depth;
		
		SetRGB4( vp, 0, 0, 0, 0 );

		switch( Prefs[COLORS].po_Active )
		{
		case 0:
			ColorTable = RainbowPalette( Scr, 0L, 1L, 0L );
			break;
		case 1:
			SetRGB4( vp, 1, RAND( 14, 1 ), RAND( 14, 1 ), RAND( 14, 1 ));
			break;
		case 3:
			setCopperList( Hei, 1, vp, &custom );
        case 2:
			SetRGB4( vp, 1, 15, 15, 15 );
			break;
		}
		
		for( k = 0; k < nl; k++ )
		{
			cx[k][1] = RAND( Wid - 2, 1 );
			cy[k][1]= RAND( Hei - 2, 1 );
			dx[k] = RAND( xSpeed, 1 );
			dy[k] = RAND( ySpeed, 1 );
		}
		i = 0;
		j = 0;
		
		Wnd = BlankMousePointer( Scr );
		ScreenToFront( Scr );
		
		while( RetVal == OK )
		{
			if(!( ++ToFrontCount % 5 ))
				WaitTOF();

			if(!( ToFrontCount % 300 ))
				ScreenToFront( Scr );
			
			i = (i+1) % Trail;
			n = (i+1) % Trail;
			j = (j+1) % 255;
			
			if( !Prefs[COLORS].po_Active )
				SetAPen( rp, ( j * ( numCols - 1 )) / 255 + 1 );
			else
				SetAPen( rp, 1 );
			
			for( k = 0; k < nl; k++ )
			{
				if( cx[k][i] >= Wid )
				{
					dx[k] = -1 * RAND( xSpeed, 1 );
					cx[k][i] = Wid-1;
				}
				else
					if( cx[k][i] < 0 )
					{
						dx[k] = RAND( xSpeed, 1 );
						cx[k][i] = 0;
					}
				if( cy[k][i] >= Hei )
				{
					dy[k] = -1 * RAND( ySpeed, 1 );
					cy[k][i] = Hei-1;
				}
				else
					if( cy[k][i] < 0 )
					{
						dy[k] = RAND( ySpeed, 1 );
						cy[k][i] = 0;
					}
			}
			
			Move( rp, cx[0][i], cy[0][i] );
			for( k = 1; k < nl; k++ )
				Draw( rp, cx[k][i], cy[k][i] );
			if( nl > 2 )
				Draw( rp, cx[0][i], cy[0][i] );
			
			SetAPen( rp, 0 );
			
			Move( rp, cx[0][n], cy[0][n] );
			for( k = 1; k < nl; k++ )
				Draw( rp, cx[k][n], cy[k][n] );
			if( nl > 2 )
				Draw( rp, cx[0][n], cy[0][n] );
			
			for( k = 0; k < nl; k++ )
			{
				cx[k][n] = cx[k][i] + dx[k];
				cy[k][n] = cy[k][i] + dy[k];
			}

			if(!( ToFrontCount % 25 ))
				RetVal = ContinueBlanking();
		}
		UnblankMousePointer( Wnd );

		if( Prefs[COLORS].po_Active == 3 )
			clearCopperList( vp );
	}
	else
		RetVal = FAILED;

	if( cx )
	{
		for( k = 0; k < nl; k++ )
			if( cx[k] )
				FreeVec( cx[k] );
		FreeVec( cx );
	}

	if( cy )
	{
		for( k = 0; k < nl; k++ )
			if( cy[k] )
				FreeVec( cy[k] );
		FreeVec( cy );
	}

	if( dx )
		FreeVec( dx );
	if( dy )
		FreeVec( dy );

	if(!( Prefs[COLORS].po_Active ))
		RainbowPalette( 0L, ColorTable, 1L, 0L );
	
	if( Scr )
		CloseScreen( Scr );

	return RetVal;
}
