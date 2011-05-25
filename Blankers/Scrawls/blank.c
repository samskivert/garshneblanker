/*
**			Scrawls Blanker
**
**		Copyright © 1995 by Marzio De Biasi
**
**
**	Source:		blank.c
**	Language:	ANSI C
**	Version:	1.0
**	Description:	a module for GarshneBlanker v38.8 modular screen
**			blanker by Micheal D. Bayne, see Scrawls.doc for
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

#define PREF_EQU	0
#define PREF_FAC 	2
#define PREF_ITER 	4
#define PREF_ZOOM	6
#define PREF_DELAY	8
#define MODE		10

LONG Hei, Wid;
Triplet *ColorTable = 0L;
WORD Cl;



struct Screen 	*pubIFS_sc;
struct RastPort *pubIFS_rp;


#define MAX_IFS_ITER	20

struct	IFS_eq {

    float	par[6];
    WORD	prob;
};

typedef struct IFS_eq IFS_eq_t;

LONG    	pubIFS_delay = 0;
LONG		pubIFS_iter = 8000;
IFS_eq_t	pubIFS_eq[MAX_IFS_ITER];
LONG		pubIFS_numeq = 4;
float		pubIFS_fac, pubIFS_zoom;

#include "Scrawls_rev.h"
STATIC const UBYTE VersTag[] = VERSTAG;



VOID Defaults( PrefObject *Prefs )
{
	Prefs[PREF_EQU].po_Level = 4;
	Prefs[PREF_FAC].po_Level = 4;
	Prefs[PREF_ITER].po_Level = 80;
	Prefs[PREF_ZOOM].po_Level = 20;
	Prefs[PREF_DELAY].po_Level = 0;
	Prefs[MODE].po_ModeID = getTopScreenMode();
	Prefs[MODE].po_Depth = 4;
}




/* ------------------------------------------------------------------- */
/* ---- SetUpField() ------------------------------------------------- */
/* ------------------------------------------------------------------- */

void	SetUpField(void) {
/*
**  Clears screen, and builds the iterated function system.
*/

    WORD	i, j, t_prob;

    SetRast( pubIFS_rp, 0L );
    ScreenToFront(pubIFS_sc);

    t_prob = 100 - pubIFS_numeq + 1;

    for (j = 0; j < pubIFS_numeq; j++) {
    	for (i = 0; i < 6; i++) {
	    pubIFS_eq[j].par[i] = (drand48() * 2.0) - 1.;
	    if ((i == 2) || (i == 5)) pubIFS_eq[j].par[i] *= pubIFS_fac;
	}	

	if (j < (pubIFS_numeq - 1)) {
		if (t_prob > 0) pubIFS_eq[j].prob = lrand48() % t_prob + 1;
		else pubIFS_eq[j].prob = 1;

		t_prob = t_prob - pubIFS_eq[j].prob + 1;
	}
	else {
	    pubIFS_eq[j].prob = t_prob;
	}
    }
}




/* ------------------------------------------------------------------- */
/* ---- SetPixel() --------------------------------------------------- */
/* ------------------------------------------------------------------- */

BOOL    SetPixel(float x, float y) {
/*
**  Writes a pixel.
*/

    WORD	px, py, c;

    px = (x * pubIFS_zoom + (Wid / 2)) + 0.5;
    py = (y * pubIFS_zoom + (Hei / 2)) + 0.5;

    if ((px < 0) || (px >= Wid) || (py < 0) || (py >Hei)) {
	return(FALSE);
    } 
    else {

	c = ReadPixel(pubIFS_rp, px, py);
	c = (c < (Cl - 1))? c + 1 : 0;
	SetAPen(pubIFS_rp, c);
	WritePixel(pubIFS_rp, px, py);
	return(TRUE);
    }
}




/* ------------------------------------------------------------------- */
/* ---- DoIFS() ------------------------------------------------------ */
/* ------------------------------------------------------------------- */

void	DoIFS(float *x, float *y) {
/*
**  Randomly chooses a function, then apply it to point (<x>,<y>).
*/

    float	t_x, t_y;
    WORD	n, eq;

    n = lrand48() % 100 + 1;
    eq = 0;

    while ( (n = n - pubIFS_eq[eq].prob) > 0) eq++; 
    if (eq >= pubIFS_numeq) eq = pubIFS_numeq - 1;

    t_x = pubIFS_eq[eq].par[0] * (*x) + pubIFS_eq[eq].par[1] * (*y) + \
	  pubIFS_eq[eq].par[2];
    t_y = pubIFS_eq[eq].par[3] * (*x) + pubIFS_eq[eq].par[4] * (*y) + \
	  pubIFS_eq[eq].par[5];

    *x = t_x;
    *y = t_y;
}




/* ------------------------------------------------------------------- */
/* ---- IFSDriver() -------------------------------------------------- */
/* ------------------------------------------------------------------- */

LONG	IFSDriver( void ) {
/*
**  Main loop: chooses the starting point, transforms it several times, then
**  starts calculating the attractor of the IFS.
*/

    LONG	RetVal = OK;
    LONG	iter_count;
    WORD	i;
    UBYTE	skip_flag;
    LONG	scr2front = 0;
    float	x, y;


    while (RetVal == OK) {

    	ColorTable = RainbowPalette( pubIFS_sc, 0L, 1L, 0L );
    	SetUpField();


	x = drand48() * (Wid / 2.0);
	y = drand48() * (Hei / 2.0);

	iter_count = 0;
	skip_flag = FALSE;

	for (i = 0; i < 20; i++)  DoIFS(&x,&y);

	while (! skip_flag) {

	    if ((scr2front++ % 50) == 0) ScreenToFront(pubIFS_sc);

	    for (i = 0; i < pubIFS_delay; i++) WaitTOF();

	    DoIFS(&x, &y);
	    SetPixel(x,y);

	    if (iter_count++ > pubIFS_iter) skip_flag = TRUE;

	    if ((RetVal = ContinueBlanking()) != OK) skip_flag = TRUE;
	}


	RainbowPalette( 0L, ColorTable, 1L, 0L );
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
**  numbers generator seed, then call <IFSDriver()>.
*/
	struct Window *Wnd;
	LONG  RetVal;

	srand48(time(0L));

	pubIFS_sc = OpenScreenTags( NULL, SA_Depth, Prefs[MODE].po_Depth,
							   SA_Quiet, TRUE, SA_DisplayID,
							   Prefs[MODE].po_ModeID, SA_Behind, TRUE,
							   SA_Overscan, OSCAN_STANDARD, SA_ShowTitle,
							   FALSE, SA_Title, "Garshnescreen", TAG_DONE );

	if( pubIFS_sc )
	{
		pubIFS_rp = &pubIFS_sc->RastPort;

		Cl = 1L << Prefs[MODE].po_Depth;


		Wid = pubIFS_sc->Width;
		Hei = pubIFS_sc->Height;

    		ColorTable = RainbowPalette( pubIFS_sc, 0L, 1L, 0L );
/*		SetRGB4(&(pubIFS_sc->ViewPort),1,0xF,0xF,0xF);
*/		


		Wnd = BlankMousePointer( pubIFS_sc );
		
	
		pubIFS_numeq = Prefs[PREF_EQU].po_Level;
		pubIFS_fac = (float)Prefs[PREF_FAC].po_Level;
		pubIFS_iter = Prefs[PREF_ITER].po_Level * 100;
		pubIFS_zoom = (float)Prefs[PREF_ZOOM].po_Level;
		pubIFS_delay = Prefs[PREF_DELAY].po_Level;

		RetVal = IFSDriver();

		UnblankMousePointer( Wnd );

		CloseScreen( pubIFS_sc );
	}
	else
		RetVal = FAILED;
	
	return(RetVal);
}
