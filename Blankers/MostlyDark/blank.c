/*
**			 MostlyDark Blanker
**
**		Copyright © 1995 by Marzio De Biasi
**
**
**	Source:		blank.c
**	Language:	ANSI C
**	Version:	1.0
**	Description:	a module for GarshneBlanker v38.8 modular screen
**			blanker by Micheal D. Bayne, see MostlyDark.doc for
**			further details.
**	Last update:	13 Feb 95		
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
#include <stdio.h>
#include <stdlib.h>

#include "MostlyDark_rev.h"
STATIC const UBYTE VersTag[] = VERSTAG;

#define PREF_DENS	0
#define PREF_DELAY	2
#define MODE		4

/* ---- screen-oriented variables ------------------------------------ */

LONG Hei, Wid;
Triplet *ColorTable = 0L;
WORD Cl;

struct Screen 	*pubDrk_sc;
struct RastPort *pubDrk_rp;

WORD	pubDrk_w, pubDrk_h;


/* ---- blanker-oriented variables ----------------------------------- */

#include "MostlyDark.h"

WORD		pubDrk_nb = 0, pubDrk_totb = 80;
struct List	pubDrk_blobs;

LONG    pubDrk_delay = 0;
LONG 	pubDrk_dens;




/* ------------------------------------------------------------------- */
/* ---- Defaults() --------------------------------------------------- */
/* ------------------------------------------------------------------- */

VOID Defaults( PrefObject *Prefs )
{
	Prefs[PREF_DENS].po_Level = 7;
	Prefs[PREF_DELAY].po_Level = 2;
	Prefs[MODE].po_ModeID = (getTopScreenMode() & MONITOR_ID_MASK) | LORES_KEY;
	Prefs[MODE].po_Depth = 5;
}




/* ------------------------------------------------------------------- */
/* ---- RefreshBlob() ------------------------------------------------ */
/* ------------------------------------------------------------------- */

BOOL	RefreshBlob(Blob_t *b) {

    if (b->inc_f) {

	if (b->c < Cl - 1) {
	    b->c++;
	    SetAPen(pubDrk_rp, b->c);
	    WritePixel(pubDrk_rp, b->x, b->y);
	}
	else
	    b->inc_f = FALSE;
	
	return(TRUE);
    }
    else {
	if (b->c > 0) {
	    b->c--;
	    SetAPen(pubDrk_rp, b->c);
	    WritePixel(pubDrk_rp, b->x, b->y);
	    return(TRUE);
	}
	else
	    return(FALSE);
    }
}




/* ------------------------------------------------------------------- */
/* ---- InitBlob() --------------------------------------------------- */
/* ------------------------------------------------------------------- */

void	InitBlob(Blob_t *b) {

    b->nod.ln_Type = NT_USER;

    while(TRUE) {
	b->x = lrand48() % Wid;
    	b->y = lrand48() % Hei;
	if (ReadPixel(pubDrk_rp,b->x,b->y)==0) break;
    }
    b->c = 0;
    b->inc_f = TRUE; 
}




/* ------------------------------------------------------------------- */
/* ---- MostlyDarkDriver() ------------------------------------------- */
/* ------------------------------------------------------------------- */

LONG	MostlyDarkDriver( void ) {

    LONG	RetVal = OK;
    ULONG	scrt, scr2front = 0;
    WORD	i;
    ULONG	*ctab, c;
    UBYTE	rf, gf, bf;
    UWORD	*OldCTab;
    float	dum;
    Blob_t	*nb, *t;

    pubDrk_nb = 0;
   
    scrt = (pubDrk_delay == 0)? 5000 : 500 / pubDrk_delay; 

    pubDrk_blobs.lh_Type = NT_USER;
    pubDrk_blobs.lh_Head = &pubDrk_blobs.lh_Tail;
    pubDrk_blobs.lh_Tail = NULL;
    pubDrk_blobs.lh_TailPred = &pubDrk_blobs.lh_Head;


/*    ctab = NULL; */
    ctab = GetColorTable(pubDrk_sc); 

    rf = 1;
    gf = 1;
    bf = 1;

    if (ctab) {

    	ctab[1] = 0;
    	ctab[2] = 0;
    	ctab[3] = 0;

	dum = 255.0/(Cl-2);

    	for (i = 1; i < Cl ; i++) {

	    c = (i - 1) * dum;
	    c &= 0xFF;

	    c = c | (c << 8) | (c << 16) | (c<<24);

	    ctab[i*3+1] = c * rf; 	        
	    ctab[i*3+2] = c * gf; 	        
	    ctab[i*3+3] = c * bf; 	        
	}

    	LoadRGB32(&(pubDrk_sc->ViewPort), ctab);
    }
    else {

	OldCTab = (pubDrk_sc->ViewPort.ColorMap)->ColorTable;
	OldCTab[0] = 0;

	dum = 15.0/(Cl-2);

	if (dum == 0) dum = 1;

    	for (i = 1; i < Cl; i++) {

	    c = (i - 1) * dum;
	    if (c > 0xF) c = 0xF;
	    c &= 0xF;

	    if (gf)  c |= (c << 4);
	    if (bf)  c |= (c << 8);
	    if (!rf) c &= 0xFF0;
		
	    OldCTab[i] = c; 	        
	}
    	LoadRGB4(&(pubDrk_sc->ViewPort), OldCTab, Cl);
    }

/*
    for (i = 0; i < Cl; i++) {
	SetAPen(pubDrk_rp,i);
	RectFill(pubDrk_rp,i*20,20,i*20+20,40);
	}
*/

    while (RetVal == OK) {


	if ((scr2front++ % scrt) == 0) ScreenToFront(pubDrk_sc); 


	for (i = 0; i < pubDrk_delay; i++) WaitTOF();


	/* shall we add a new blob? */

	if (pubDrk_dens < 6) {
	    if ((lrand48() % (6 - pubDrk_dens)) == 0) {

	    	if ((nb = (Blob_t *)malloc(sizeof(Blob_t))) != NULL) {
		    pubDrk_nb++;
		    InitBlob(nb);
		    AddHead(&pubDrk_blobs, nb);
	    	}
	    }
	}
	else for (i = 0; i < pubDrk_dens - 4; i++) {

	    if ((nb = (Blob_t *)malloc(sizeof(Blob_t))) != NULL) {

		pubDrk_nb++;
		InitBlob(nb);
		AddHead(&pubDrk_blobs, nb);
	    }
	} 


	/* refresh all blobs */

	nb = pubDrk_blobs.lh_Head;

	while (nb->nod.ln_Succ != NULL) {

	    if (!RefreshBlob(nb)) {
		t = nb->nod.ln_Succ;
		Remove(nb);
		free(nb);
		nb = t;
		pubDrk_nb--;
	    }
	    else {
		nb = nb->nod.ln_Succ;
	    }
	}

        if ((RetVal = ContinueBlanking()) != OK) break;
    }

    /* free blob list */
    nb = pubDrk_blobs.lh_Head;
    while (nb->ln_Succ != NULL) {
	t = nb->nod.ln_Succ;
	Remove(nb);
	free(nb);
	nb = t;
	pubDrk_nb--;
    } 

    if (ctab) FreeVec(ctab); 

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


	pubDrk_sc = OpenScreenTags( NULL, SA_Depth, Prefs[MODE].po_Depth, SA_Quiet, TRUE,
						 SA_DisplayID, Prefs[MODE].po_ModeID, SA_Behind, TRUE,
						 SA_Overscan, OSCAN_STANDARD, TAG_DONE );

	if( pubDrk_sc )
	{

		ScreenToFront(pubDrk_sc);

		pubDrk_rp = &pubDrk_sc->RastPort;

		Cl = 1L << Prefs[MODE].po_Depth;

		pubDrk_delay = Prefs[PREF_DELAY].po_Level & 0xFFFF;
		pubDrk_dens = Prefs[PREF_DENS].po_Level;

		
		Wid = pubDrk_sc->Width;
		Hei = pubDrk_sc->Height;


/*    		ColorTable = RainbowPalette( pubDrk_sc, 0L, 1L, 0L );  */

		Wnd = BlankMousePointer( pubDrk_sc );
		
	

		RetVal = MostlyDarkDriver();

		UnblankMousePointer( Wnd );
/*		RainbowPalette( 0L, ColorTable, 1L, 0L );	*/

		CloseScreen( pubDrk_sc );
	}
	else
		RetVal = FAILED;
	
	return(RetVal);
}
