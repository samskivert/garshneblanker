/*
 *  Copyright (c) 1994 Michael D. Bayne.
 *  All rights reserved.
 *
 *  Please see the documentation accompanying the distribution for distribution
 *  and disclaimer information.
 */

#include <exec/memory.h>
#include <math.h>

#include "/includes.h"

Triplet *ColorTable = 0L;

VOID Defaults( PrefObject *Prefs )
{
	Prefs[0].po_ModeID = getTopScreenMode();
	Prefs[0].po_Depth = 2;
}

LONG Dragon( struct Screen *Scr, SHORT Wid, SHORT Hei )
{
	LONG Color = 1, mod = (( 1L << Scr->BitMap.Depth ) - 1 ) * 5;
	LONG i, RetVal = OK, xd, yd;
	float a = PI-(( float )RangeRand( 100 )+10.0)/5000, xx, yy, x = 0, y = 0;
	struct RastPort *RP = &( Scr->RastPort );
	
	SetRast( RP, 0 );
	ScreenToFront( Scr );
	
	for( i = 0; i < 1000000 && RetVal == OK; i++ )
	{
		if(!( i % 100 ))
		{
			RetVal = ContinueBlanking();
			ScreenToFront( Scr );
		}
		
		xx = y - sin( x );
		yy = a - x;
		
		xd = Wid / 2 + (SHORT)( 2.0 * x );
		yd = Hei / 2 + (SHORT)( 2.0 * y );

		Color = ( Color + 1 ) % mod;

		if(( xd >= 0 )&&( yd >= 0 )&&( xd < Wid )&&( yd < Hei ))
		{
			SetAPen( RP, Color/5 + 1 );
			WritePixel( RP, xd, yd );
		}

		x = xx;
		y = yy;
	}

	return RetVal;
}

LONG Blank( PrefObject *Prefs )
{
	struct Screen *Scr;
	struct Window *Wnd;
	LONG RetVal;
	
	if( Scr = OpenScreenTags( NULL, SA_Depth, Prefs[0].po_Depth,
							 SA_Quiet, TRUE, SA_DisplayID, Prefs[0].po_ModeID,
							 SA_Behind, TRUE, SA_Overscan, OSCAN_STANDARD,
							 SA_ShowTitle, FALSE, SA_Title, "Garshnescreen",
							 TAG_DONE ))
	{
		SetRGB4(&( Scr->ViewPort ), 0, 0, 0, 0 );
		ColorTable = RainbowPalette( Scr, 0L, 1L, 0L );
		Wnd = BlankMousePointer( Scr );
		
		do
			RetVal = Dragon( Scr, Scr->Width, Scr->Height );
		while( RetVal == OK );
		
		UnblankMousePointer( Wnd );
		RainbowPalette( 0L, ColorTable, 1L, 0L );
		CloseScreen( Scr );
	}
	else
		RetVal = FAILED;
	
	return RetVal;
}
