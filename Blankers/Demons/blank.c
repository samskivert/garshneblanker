/*
**			   Demons Blanker
**
**		Copyright © 1995 by Marzio De Biasi
**
**
**	Source:		blank.c
**	Language:	ANSI C
**	Version:	1.0
**	Description:	a module for GarshneBlanker v38.8 modular screen
**			blanker by Micheal D. Bayne, see Demons.doc for
**			further details.
**	Last update:	24 Jan 95		
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
#include <stdlib.h>
#include <exec/memory.h>

#define PREF_SIZE 	0
#define PREF_COLORS	2
#define PREF_WIDTH	4
#define PREF_HEIGHT	6
#define MODE		8


#define NSC	2

#define OTHER(s) 	((s + 1) & 1)

LONG Hei, Wid;
Triplet *ColorTable[NSC];
ULONG	*ctab;
WORD Cl;

WORD	*mem[3];


struct Screen 	*pubDem_sc[NSC];
struct RastPort *pubDem_rp[NSC];

LONG    pubDem_siz = 3;
WORD    pubDem_w, pubDem_h;
LONG   	pubDem_delay = 5;
WORD    pubDem_uw, pubDem_uh, pubDem_cl;
WORD	pubDem_topx, pubDem_topy;


#include "Demons_rev.h"
STATIC const UBYTE VersTag[] = VERSTAG;




VOID Defaults( PrefObject *Prefs )
{
	Prefs[PREF_SIZE].po_Level = 6;
	Prefs[PREF_COLORS].po_Level = 12;
	Prefs[PREF_WIDTH].po_Level = 40;
	Prefs[PREF_HEIGHT].po_Level = 25;
	Prefs[MODE].po_ModeID = getTopScreenMode();
	Prefs[MODE].po_Depth = 4;
}




/* ------------------------------------------------------------------- */
/* ---- DrawCell() --------------------------------------------------- */
/* ------------------------------------------------------------------- */

void	DrawCell(struct RastPort *rp, WORD x, WORD y, WORD color) {
/*
**  Draws a <pubDem_siz> x <pubDem_siz> box at (<x>,<y>) using color <color>.
*/

    WORD	ox, oy, t;

    SetAPen(rp, color);

    ox = x * pubDem_siz + pubDem_topx;
    oy = y * pubDem_siz + pubDem_topy;

    if (pubDem_siz > 1) {
	t = pubDem_siz - 1;
	RectFill(rp, ox, oy, ox + t, oy + t);
    }
    else WritePixel(rp, ox, oy);
}




/* ------------------------------------------------------------------- */
/* ---- GetCellStat() ------------------------------------------------ */
/* ------------------------------------------------------------------- */

WORD	GetCellStat(struct RastPort *rp, WORD x, WORD y ) {
/*
**  Reads the color of the pixel at (<x>,<y>).
*/
    WORD	ox, oy;
    WORD	c;

    ox = x * pubDem_siz + pubDem_topx;
    oy = y * pubDem_siz + pubDem_topy;
    c = ReadPixel(rp, ox, oy);

    return(c);
}




/* ------------------------------------------------------------------- */
/* ---- SetUpField() ------------------------------------------------- */
/* ------------------------------------------------------------------- */

LONG	SetUpField(LONG src) {
/*
**  Clears screen, then sets each cell to a random color.
*/

    WORD	i, c;
    WORD	x, y;
    LONG	RetV = OK;

    for (i = 0; i < NSC; i++) SetRast( pubDem_rp[i], 0L );

    ScreenToFront(pubDem_sc[OTHER(src)]);

    for (x = 0; x < pubDem_w; x++) {
	for (y = 0; y < pubDem_h; y++) {
	
	    c = lrand48() % pubDem_cl;
	    DrawCell(pubDem_rp[src], x, y, c);
	}
	
        if ((RetV = ContinueBlanking()) != OK) goto l_skip_it;
    }

    ScreenToFront(pubDem_sc[src]);

l_skip_it:
    return(RetV);
}




/* ------------------------------------------------------------------- */
/* ---- RedrawCells -------------------------------------------------- */
/* ------------------------------------------------------------------- */

LONG	RedrawCells(LONG src, LONG dest) {
/*
**  Checks the neighbours of each cell, then changes its state. 
*/

    WORD	x, y, xx;
    WORD	i, act, bot, l1, l2, l3;
    WORD	c, cpp, nc /*, tx */;
    UBYTE	no_ch = FALSE;
    LONG	RetV = OK;
    struct BitMap *bm;

    for (i = 0; i < (bm = (pubDem_rp[src]->BitMap))->Depth; i++) {

	CopyMem(bm->Planes[i],(pubDem_rp[dest]->BitMap)->Planes[i], \
		bm->BytesPerRow * bm->Rows);
    }

   

	for (xx = 0; xx < pubDem_w; xx++) {
	    mem[0][xx] = GetCellStat(pubDem_rp[src], xx, pubDem_h - 1);
	    mem[1][xx] = GetCellStat(pubDem_rp[src], xx, 0); 
	}

        act = 1;

    	for (y = 0; y < pubDem_h; y++) {

	    l1 = (act + 2) % 3;
	    l2 = act % 3;
	    l3 = (act + 1) % 3;

	    bot = (y + 1) % pubDem_h;

	    for (xx = 0; xx < pubDem_w; xx++) {
	    	mem[l3][xx] = GetCellStat(pubDem_rp[src], xx, bot);

	    }


	    for (x = 0; x < pubDem_w; x++) {


		c = mem[l2][x];
		cpp = (c + 1) % pubDem_cl;
		nc = c;

/*
		tx = (x > 0)? (x - 1) : (pubDem_w - 1);

		if ((mem[l1][tx] == cpp) || (mem[l2][tx] == cpp) || (mem[l3][tx] == cpp)) {

		    nc = cpp;
		    goto l_draw_it;
		}

		tx = (x + 1) % pubDem_w;

		if ((mem[l1][tx] == cpp) || (mem[l2][tx] == cpp) || (mem[l3][tx] == cpp)) {
		    nc = cpp;
		    goto l_draw_it;
		}

		if ((mem[l1][x] == cpp) || (mem[l3][x] == cpp)) nc = cpp;

*/

		if (mem[l2][((x > 0)? (x - 1) : (pubDem_w - 1))] == cpp) {

		    nc = cpp;
		    goto l_draw_it;
		}

		if (mem[l2][((x + 1) % pubDem_w)] == cpp) {
		    nc = cpp;
		    goto l_draw_it;
		}

		if ((mem[l1][x] == cpp) || (mem[l3][x] == cpp)) nc = cpp;



l_draw_it:
		if (nc == cpp) {
		    no_ch = TRUE;
		    DrawCell(pubDem_rp[dest], x, y, nc); 		
		}

	    if ((RetV = ContinueBlanking()) != OK) goto l_skip_blank;

	    }


	    act++;

    	}
	
	ScreenToFront(pubDem_sc[dest]);

	if (!no_ch) RetV = SetUpField(dest);


l_skip_blank:


    return(RetV);
}




/* ------------------------------------------------------------------- */
/* ---- DemonDriver() -------------------------------------------------- */
/* ------------------------------------------------------------------- */

LONG	DemonDriver( void ) {
/*
**  Main loop: calls RedrawCells the swaps screens.
*/

    LONG	RetVal = OK;
    WORD	i;
    LONG	this_screen = 0;


    pubDem_w = Wid / pubDem_siz;
    pubDem_h = Hei / pubDem_siz; 

    if (pubDem_uw < pubDem_w) pubDem_w = pubDem_uw;
    if (pubDem_uh < pubDem_h) pubDem_h = pubDem_uh;

    pubDem_topx = (Wid - pubDem_w * pubDem_siz) / 2;
    pubDem_topy = (Hei - pubDem_h * pubDem_siz) / 2;


    for (i = 0; i < 3; i++) {
	if ((mem[i] = (WORD *)AllocMem(pubDem_w * sizeof(WORD), MEMF_FAST)) == NULL) break;
    }

    if (i == 3) {

	RetVal = SetUpField(this_screen & 1);

    	while (RetVal == OK) {
	    RetVal = RedrawCells(this_screen & 1, (this_screen + 1) & 1);
	this_screen++;
	}
    }
    else {
	while((RetVal == ContinueBlanking()) == OK) WaitTOF();
    }

    for (i = 0; i < 3; i++) {
	if (mem[i]) FreeMem(mem[i], pubDem_w * sizeof(WORD) );
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
**  numbers generator seed, then call <DemonDriver()>.
*/
	struct Window *Wnd[NSC];
	LONG  RetVal;
	WORD	i, j;

	srand48(time(0L));

	if (Prefs[MODE].po_Depth == 1) Prefs[MODE].po_Depth = 2;

	for (i = 0; i < NSC; i++) {
		pubDem_sc[i] = OpenScreenTags( NULL, SA_Depth, Prefs[MODE].po_Depth,
									  SA_Quiet, TRUE, SA_DisplayID,
									  Prefs[MODE].po_ModeID, SA_Behind, TRUE,
									  SA_Overscan, OSCAN_STANDARD,
									  SA_ShowTitle, FALSE, SA_Title,
									  "Garshnescreen", TAG_DONE );
	   if (pubDem_sc[i] == NULL) break;
	} 

	if( i == NSC )
	{
		Cl = 1L << Prefs[MODE].po_Depth;

		Wid = pubDem_sc[0]->Width;
		Hei = pubDem_sc[0]->Height;

		for (j = 0; j < NSC; j++) {
			pubDem_rp[j] = &(pubDem_sc[j]->RastPort);
			Wnd[j] = BlankMousePointer( pubDem_sc[j] );
		}

   		ColorTable[0] = RainbowPalette( pubDem_sc[0], 0L, 1L, 0L );
		ctab = GetColorTable(pubDem_sc[0]);

		for (j = 1; j < NSC; j++) {
		
		    if ( ctab != NULL) {
			LoadRGB32(&(pubDem_sc[j]->ViewPort), ctab );
		    } 
		    else {
			LoadRGB4(&(pubDem_sc[j]->ViewPort),(pubDem_sc[j]->ViewPort.ColorMap)->ColorTable, Cl);
		    }
		}

		pubDem_siz = Prefs[PREF_SIZE].po_Level;
		pubDem_cl = Prefs[PREF_COLORS].po_Level;
		pubDem_uw = Prefs[PREF_WIDTH].po_Level;
		pubDem_uh = Prefs[PREF_HEIGHT].po_Level;

		if (pubDem_cl > Cl) pubDem_cl = Cl;

		RetVal = DemonDriver();

		for (j = 0; j < NSC; j++) {
			UnblankMousePointer( Wnd[j] );
		}

		RainbowPalette( 0L, ColorTable[0], 1L, 0L );


	}
	else RetVal = FAILED;

	for (i = 0; i < NSC; i++) if (pubDem_sc[i]) CloseScreen(pubDem_sc[i]);
	
	return(RetVal);
}
