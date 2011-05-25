/*
 *	Bouncing Balls Blanker   V38.1  (12.2.95)
 *
 *		by Stefano Reksten and Andrew Richards
 *
 *
 *		Based on Stefano Reksten Bouncing Balls module v1.2 for
 *      his BServer screen blanker. Ported and modified by Andrew
 *      Richards.
 *
 */

#include <exec/memory.h>
#include <math.h>
#include "/includes.h"

#include "Balls_rev.h"

STATIC const UBYTE VersTag[] = VERSTAG;


#define DELAY   0
#define MODE	2


#define MAXBALLS 8


UWORD chip ball_imgdata[116] = {
 0x1F, 0xC000,
 0xE0, 0x3800,
 0x103, 0xC400,
 0x60F, 0xF300,
 0x800, 0x7C80,
 0x1000, 0xE40,
 0x1000, 0x740,
 0x2000, 0x3A0,
 0x4000, 0x1D0,
 0x5000, 0xD0,
 0x5000, 0xF0,
 0xB000, 0xE8,
 0xB000, 0x68,
 0xB000, 0x68,
 0xB800, 0x68,
 0xB800, 0x48,
 0xB800, 0x48,
 0xB800, 0x8,
 0x7800, 0x10,
 0x5C00, 0x10,
 0x5C00, 0x10,
 0x2E00, 0x20,
 0x1700, 0x40,
 0x13C0, 0x40,
 0x9FE, 0x80,
 0x6FF, 0xE300,
 0x13F, 0xC400,
 0xE0, 0x3800,
 0x1F, 0xC000,
 0x1F, 0xC000,
 0xFF, 0xF800,
 0x1FC, 0x3C00,
 0x7F0, 0xF00,
 0xFFF, 0x8380,
 0x1FFF, 0xF1C0,
 0x1FFF, 0xF8C0,
 0x3FFF, 0xFC60,
 0x7FFF, 0xFE30,
 0x7FFF, 0xFF30,
 0x7FFF, 0xFF10,
 0xFFFF, 0xFF18,
 0xFFFF, 0xFF98,
 0xFFFF, 0xFF98,
 0xFFFF, 0xFF98,
 0xFFFF, 0xFFB8,
 0xFFFF, 0xFFB8,
 0xFFFF, 0xFFF8,
 0x7FFF, 0xFFF0,
 0x7FFF, 0xFFF0,
 0x7FFF, 0xFFF0,
 0x3FFF, 0xFFE0,
 0x1FFF, 0xFFC0,
 0x1FFF, 0xFFC0,
 0xFFF, 0xFF80,
 0x7FF, 0xFF00,
 0x1FF, 0xFC00,
 0xFF, 0xF800,
 0x1F, 0xC000
 };

UWORD chip ball_mask[58] = {
 0x1F, 0xC000,
 0xFF, 0xF800,
 0x1FF, 0xFC00,
 0x7FF, 0xFF00,
 0xFFF, 0xFF80,
 0x1FFF, 0xFFC0,
 0x1FFF, 0xFFC0,
 0x3FFF, 0xFFE0,
 0x7FFF, 0xFFF0,
 0x7FFF, 0xFFF0,
 0x7FFF, 0xFFF0,
 0xFFFF, 0xFFF8,
 0xFFFF, 0xFFF8,
 0xFFFF, 0xFFF8,
 0xFFFF, 0xFFF8,
 0xFFFF, 0xFFF8,
 0xFFFF, 0xFFF8,
 0xFFFF, 0xFFF8,
 0x7FFF, 0xFFF0,
 0x7FFF, 0xFFF0,
 0x7FFF, 0xFFF0,
 0x3FFF, 0xFFE0,
 0x1FFF, 0xFFC0,
 0x1FFF, 0xFFC0,
 0xFFF, 0xFF80,
 0x7FF, 0xFF00,
 0x1FF, 0xFC00,
 0xFF, 0xF800,
 0x1F, 0xC000
 };


struct Ball {
	WORD X, Y;
	WORD OldX, OldY;
	BYTE DirX;
	BYTE DirY;
	UWORD VelX;
	UWORD VelY;
	} balls[MAXBALLS];

struct BitMap ballbmap;


VOID Defaults( PrefObject *Prefs ) {
	Prefs[DELAY].po_Level = 1;
	Prefs[MODE].po_Depth = 2;
	Prefs[MODE].po_ModeID = getTopScreenMode();
}



void SetAllBalls( struct Screen *scrn )
{
register UBYTE n;

for ( n = 0; n < MAXBALLS; n++ )
	{
	balls[n].X = RangeRand( ( scrn->Width ) - 29 );
	balls[n].Y = RangeRand( ( scrn->Height ) - 29 );
	balls[n].OldX = balls[n].OldY = 0;
	balls[n].DirX = ( RangeRand( 2 ) ? -1 : 1 );
	balls[n].DirY = 1;
	balls[n].VelX = RangeRand( 8 );
	balls[n].VelY = 0;
	}
}


void MoveAllBalls( struct Screen *actscr, struct Screen *other )
{
register UBYTE n, m;
UWORD temp, thisBallX, thisBallY, otherBallX, otherBallY;

for ( n = 0; n < MAXBALLS; n++ )
	BltMaskBitMapRastPort( &ballbmap, 0, 0,
			&actscr->RastPort, balls[n].X, balls[n].Y,
			0x1D, 0x1D, (ABC|ABNC|ANBC), (PLANEPTR)ball_mask );
ScreenToFront( actscr );
for ( n = 0; n < MAXBALLS; n++ )
	RectFill( &other->RastPort, balls[n].OldX, balls[n].OldY, balls[n].OldX + 29, balls[n].OldY + 29 );

/* SpritesOff(); */


/* Check if ALL balls are moving along bottom of the screen. 		*/
/* If so, give them all random vertical velocities. 	- Andrew	*/
if (( balls[0].VelY < 2 ) && ( balls[1].VelY < 2 ) && ( balls[2].VelY < 2 ) &&
		( balls[3].VelY < 2 ) && ( balls[4].VelY < 2 ) && ( balls[5].VelY < 2 ) &&
 		( balls[6].VelY < 2 ) && ( balls[7].VelY < 2 )) {
	/* Give new random vertical velocities for each ball */
	balls[0].VelY = RangeRand( 20 ) + 1;
	balls[1].VelY = RangeRand( 20 ) + 1;
	balls[2].VelY = RangeRand( 20 ) + 1;
	balls[3].VelY = RangeRand( 20 ) + 1;
	balls[4].VelY = RangeRand( 20 ) + 1;
	balls[5].VelY = RangeRand( 20 ) + 1;
	balls[6].VelY = RangeRand( 20 ) + 1;
	balls[7].VelY = RangeRand( 20 ) + 1;
}


for ( n = 0; n < MAXBALLS; n++ )
	{
	/* Nell'ipotesi di urti con le pareti perfettamente elastici ;-) */

	balls[n].OldX = balls[n].X;
	balls[n].OldY = balls[n].Y;

	balls[n].X += balls[n].VelX * balls[n].DirX;
	if ( balls[n].X < 0 )
		{
		balls[n].DirX = 1;
		balls[n].X = 0;
		}
	if ( balls[n].X > ( actscr->Width ) - 30 )
		{
		balls[n].DirX = -1;
		balls[n].X = ( actscr->Width ) - 30;
		}

	/* Stessa cosa per il pavimento - Ancora piu' incredibile! :-o */

	balls[n].VelY += balls[n].DirY;
	balls[n].Y += balls[n].VelY * balls[n].DirY;
	if ( balls[n].Y > (( actscr->Height ) - 30) )
		{
		balls[n].DirY = -1;
		balls[n].Y = (( actscr->Height ) - 30);
		}
	if ( balls[n].Y < 0 )
		{
		balls[n].Y = 0;
		balls[n].DirY = 1;
		}

	if ( balls[n].VelY <= 0 )
		{
		balls[n].VelY = 0;
		balls[n].DirY = 1;
		}

	/* Urto perfettamente elastico pure tra le palle !!! =8-O */
	/* ...Seppur con qualche semplificazione (qualche???) ;-) */

	thisBallX = balls[n].X + 15;
	thisBallY = balls[n].Y + 15;

	for ( m = 0; m < MAXBALLS; m == n-1 ? m+=2 : m++ )
		{
		otherBallX = balls[m].X + 15;
		otherBallY = balls[m].Y + 15;

		if ( (thisBallX - 0x1D <= otherBallX && otherBallX <= thisBallX + 0x1D) &&
		     (thisBallY - 0x1D <= otherBallY && otherBallY <= thisBallY + 0x1D) )
			{
			temp = balls[n].VelX;
			balls[n].VelX = balls[m].VelX;
			balls[m].VelX = temp;

			temp = balls[n].DirX;
			balls[n].DirX = balls[m].DirX;
			balls[m].DirX = temp;

			temp = balls[n].VelY;
			balls[n].VelY = balls[m].VelY;
			balls[m].VelY = temp;

			temp = balls[n].DirY;
			balls[n].DirY = balls[m].DirY;
			balls[m].DirY = temp;
			}
		}
	}
}



LONG Blank( PrefObject *Prefs ) {
	struct Screen *scr1, *scr2;
	struct Window *wnd;
	LONG retval = OK;
	UBYTE activescr = 0;
	int i;

	scr1 = OpenScreenTags( NULL, SA_Depth, Prefs[MODE].po_Depth, SA_Quiet, TRUE,
				SA_DisplayID, Prefs[MODE].po_ModeID, SA_Behind, FALSE,
				SA_Overscan, OSCAN_STANDARD, TAG_DONE );
	if ( scr1 ) {
		scr2 = OpenScreenTags( NULL, SA_Depth, Prefs[MODE].po_Depth, SA_Quiet, TRUE,
					 SA_DisplayID, Prefs[MODE].po_ModeID, SA_Behind, TRUE,
					 SA_Overscan, OSCAN_STANDARD, TAG_DONE );

		if ( scr2 ) {
			wnd = BlankMousePointer( scr1 );

			SetRGB4( &scr1->ViewPort, 0, 0, 0, 0 );
			SetRGB4( &scr1->ViewPort, 1, 0, 8, 15 );
			SetRGB4( &scr1->ViewPort, 2, 0, 4, 10 );
			SetRGB4( &scr1->ViewPort, 3, 0, 0, 8 );
			SetRGB4( &scr2->ViewPort, 0, 0, 0, 0 );
			SetRGB4( &scr2->ViewPort, 1, 0, 8, 15 );
			SetRGB4( &scr2->ViewPort, 2, 0, 4, 10 );
			SetRGB4( &scr2->ViewPort, 3, 0, 0, 8 );

			InitBitMap( &ballbmap, 2, 0x1D, 0x1D );
			ballbmap.Planes[0] = (PLANEPTR)( ball_imgdata );
			ballbmap.Planes[1] = (PLANEPTR)( ball_imgdata + 58 );

			SetAllBalls( scr1 );
			SetAPen( &scr1->RastPort, 0 );
			SetAPen( &scr2->RastPort, 0 );

			while( ContinueBlanking() == OK ) {
				for ( i = 0; i < Prefs[DELAY].po_Level; i++ )
					WaitTOF();

				if ( activescr == 1 )
					MoveAllBalls( scr1, scr2 );
				else
					MoveAllBalls( scr2, scr1 );
					
			activescr ^= 1;
			}

			UnblankMousePointer( wnd );
			CloseScreen( scr1 );
			CloseScreen( scr2 );
		}
	}
	else {
		retval = FAILED;
	} /* else */

	return retval;
}

