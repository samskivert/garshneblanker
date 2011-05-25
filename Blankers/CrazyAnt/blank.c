/*
**			Crazy Ant Blanker
**
**		Copyright © 1995 by Marzio De Biasi
**
**
**	Source:		blank.c
**	Language:	ANSI C
**	Version:	1.0
**	Description:	a module for GarshneBlanker v38.8 modular screen
**			blanker by Micheal D. Bayne, see CrazyAnt.doc for
**			further details.
**	Last update:	21 Jan 95		
**	Author:		De Biasi Marzio
**			Address: via Borgo Simoi, 34
**				 31029 Vittorio Veneto (TV)
**				 Italy
**			E-Mail:  debiasi@dimi.uniud.it
**
**	History:	
**
**	->
**
*/

#include "/includes.h"

#include <math.h>
#include <time.h>

#define PREF_SIZE 	0
#define PREF_BLOBS 	2
#define PREF_DELAY	4
#define MODE		6

LONG Hei, Wid;
Triplet *ColorTable = 0L;
WORD Cl;



struct Screen 	*pubAnt_sc;
struct RastPort *pubAnt_rp;

WORD	pubAnt_x, pubAnt_y;
BYTE	pubAnt_brain[256];
LONG    pubAnt_siz = 3;
LONG	pubAnt_lifet = 800;
LONG	pubAnt_spots = 600;
WORD    pubAnt_w, pubAnt_h;
LONG   pubAnt_delay = 5;



#include "CrazyAnt_rev.h"
STATIC const UBYTE VersTag[] = VERSTAG;



VOID Defaults( PrefObject *Prefs )
{
	Prefs[PREF_SIZE].po_Level = 4;
	Prefs[PREF_BLOBS].po_Level = 100;
	Prefs[PREF_DELAY].po_Level = 1;
	Prefs[MODE].po_ModeID = getTopScreenMode();
	Prefs[MODE].po_Depth = 4;
}




/* ------------------------------------------------------------------- */
/* ---- DrawAnt() ---------------------------------------------------- */
/* ------------------------------------------------------------------- */

void	DrawAnt(WORD x, WORD y, WORD color) {
/*
**  Draws a <pubAnt_siz> x <pubAnt_siz> box at (<x>,<y>) using color <color>.
*/

    WORD	ox, oy, t;

    t = pubAnt_siz - 1;
    ox = x * (pubAnt_siz + 1);
    oy = y * (pubAnt_siz + 1);

    SetAPen(pubAnt_rp, color);

    RectFill(pubAnt_rp, ox, oy, ox + t, oy + t);
}




/* ------------------------------------------------------------------- */
/* ---- GetGrassStat() ----------------------------------------------- */
/* ------------------------------------------------------------------- */

WORD	GetGrassStat(void ) {
/*
**  Reads the color of the pixel at (<pubAnt_x>,<pub_Ant_y>).
*/
    WORD	ox, oy, c;

    ox = pubAnt_x * (pubAnt_siz + 1);
    oy = pubAnt_y * (pubAnt_siz + 1);
    c = ReadPixel(pubAnt_rp, ox, oy);

    return(c);
}




/* ------------------------------------------------------------------- */
/* ---- SetUpField() ------------------------------------------------- */
/* ------------------------------------------------------------------- */

void	SetUpField(void) {
/*
**  Clears screen, draws initial spots, and builds ant's brain.
*/

    WORD	i, c;

    SetRast( pubAnt_rp, 0L );
    ScreenToFront(pubAnt_sc);


    for (i = 0; i < 256; i++) pubAnt_brain[i] = (lrand48() >> 3) % 2;

    if (Cl == 2) {
	pubAnt_brain[1] = (pubAnt_brain[0] == 1)? 0 : 1;
    } else if (Cl == 4) {
	pubAnt_brain[(lrand48() % 2) + 2] = (pubAnt_brain[0] == 1)? 0 : 1;
    }

    for (i = 0; i < pubAnt_spots; i++) {

	pubAnt_x = lrand48() % pubAnt_w;
	pubAnt_y = lrand48() % pubAnt_h;
	c = lrand48() % Cl;
	if ((Cl > 2) && (c == 1)) c = 2;
	DrawAnt(pubAnt_x, pubAnt_y, c);
    }


}




/* ------------------------------------------------------------------- */
/* ---- AntDriver() -------------------------------------------------- */
/* ------------------------------------------------------------------- */

LONG	AntDriver( void ) {
/*
**  Main loop: calculates the color of the actual grid-cell, sets new
**  direction, moves and prints ant, checks for return conditions.
*/

    LONG	RetVal = OK;
    LONG	delay_count;
    WORD	c;
    UBYTE	dead_flag;
    WORD	direction = 0;
    WORD	back_x, back_y, back_c;
    LONG	scr2front = 0;

    while (RetVal == OK) {

    	pubAnt_w = Wid / (pubAnt_siz + 1);
    	pubAnt_h = Hei / (pubAnt_siz + 1); 

    	SetUpField();

	pubAnt_x = pubAnt_w / 2;
	pubAnt_y = pubAnt_h / 2;
	back_c = 1;
	back_x = pubAnt_x;
	back_y = pubAnt_y;
	DrawAnt(pubAnt_x, pubAnt_y, 0);


    	delay_count = 0;
	dead_flag = FALSE;

	while (! dead_flag) {

	    if ((scr2front++ % 50) == 0) ScreenToFront(pubAnt_sc);

	    WaitTOF();
	    if (!pubAnt_delay || ((delay_count++ % pubAnt_delay) == 0)) {

	    	c = GetGrassStat();

		direction = (direction + 4 + ((pubAnt_brain[c])? 1 : -1)) % 4;

		if (Cl > 2) {
		    c = (c < (Cl - 1))? c + 1 : 0;
		    if (c == 1) c = 2;
		}
		else {
		   c = (c == 0)? 1: 0;
		}
	
		DrawAnt(pubAnt_x, pubAnt_y, 1);
		DrawAnt(back_x, back_y, back_c);

		back_c = c;
		back_x = pubAnt_x;
		back_y = pubAnt_y; 

		switch (direction) {

		    case 0:	pubAnt_y--;
				break;

		    case 1:	pubAnt_x++;
				break;

		    case 2:	pubAnt_y++;
				break;

		    case 3:	pubAnt_x--;
				break;
		}

		if ((pubAnt_x < 0) || (pubAnt_y < 0) || (pubAnt_x >= pubAnt_w) \
		    || (pubAnt_y >= pubAnt_h)) dead_flag = TRUE;
	    }
	    if ((RetVal = ContinueBlanking()) != OK) dead_flag = TRUE;
	}
    }

    return(RetVal);
}




/* ------------------------------------------------------------------- */
/* ---- Blank() ------------------------------------------------------ */
/* ------------------------------------------------------------------- */

LONG Blank( PrefObject *Prefs )
{
/*
**  Opens screen, initializes color table, preference values and random
**  numbers generator seed, then call <AntDriver()>.
*/
	struct Window *Wnd;
	LONG  RetVal;

	srand48(time(0L));
	
	pubAnt_sc = OpenScreenTags( NULL, SA_Depth, Prefs[MODE].po_Depth, SA_Quiet,
							   TRUE, SA_DisplayID, Prefs[MODE].po_ModeID,
							   SA_Behind, TRUE, SA_Overscan, OSCAN_STANDARD,
							   SA_ShowTitle, FALSE, SA_Title, "Garshnescreen",
							   TAG_DONE );
	if( pubAnt_sc )
	{
		pubAnt_rp = &pubAnt_sc->RastPort;

		Cl = 1L << Prefs[MODE].po_Depth;


		Wid = pubAnt_sc->Width;
		Hei = pubAnt_sc->Height;

    		ColorTable = RainbowPalette( pubAnt_sc, 0L, 1L, 0L );
		SetRGB4(&(pubAnt_sc->ViewPort),1,0xF,0xF,0xF);
		


		Wnd = BlankMousePointer( pubAnt_sc );
		
	
		pubAnt_siz = Prefs[PREF_SIZE].po_Level;
		pubAnt_spots = Prefs[PREF_BLOBS].po_Level;
		pubAnt_delay = Prefs[PREF_DELAY].po_Level;

		RetVal = AntDriver();

		UnblankMousePointer( Wnd );
		RainbowPalette( 0L, ColorTable, 1L, 0L );

		CloseScreen( pubAnt_sc );
	}
	else
		RetVal = FAILED;
	
	return(RetVal);
}
