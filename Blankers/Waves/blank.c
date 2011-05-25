/*
 **                        Waves Blanker
 **
 **                Copyright © 1995 by Marzio De Biasi
 **
 **
 **        Source:       blank.c
 **        Language:     ANSI C
 **        Version:      1.0
 **        Description:  a module for GarshneBlanker v38.8 modular screen
 **                      blanker by Michael D. Bayne, see Waves.doc for
 **                      further details.
 **        Last update:  21 Feb 95
 **        Author:       De Biasi Marzio
 **                      Address: via Borgo Simoi, 34
 **                               31029 Vittorio Veneto (TV)
 **                               Italy
 **                      E-Mail:  debiasi@dimi.uniud.it
 **
 **        History:
 **
 **        ->
 **
 */

#include "/includes.h"

#include <graphics/rastport.h>
#include <graphics/gfx.h>
#include <clib/exec_protos.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

#define PREF_SPOTS 	0
#define PREF_WLEN	2
#define PREF_SIZ	4
#define PREF_DELAY	6
#define MODE		8

LONG Hei, Wid;
Triplet *ColorTable = 0L;
WORD Cl;

struct Screen 	*pubWav_sc;
struct RastPort *pubWav_rp;
struct RastPort *pubWav_trp;
struct BitMap	*pubWav_tbm;

#define MAX_SPOTS	5

struct Spot {
    WORD	x;
    WORD	y;
};
typedef struct Spot Spot_t;

#define PIx2		6.2831853
#define MAX_WLEN	120
#define MAX_WABS	100

WORD	pubWav_tdpth;
WORD	pubWav_tw;
LONG	pubWav_siz = 4;
LONG	pubWav_wlen = 10;
LONG	pubWav_spots;
Spot_t  pubWav_s[MAX_SPOTS];
WORD	pubWav_sin[MAX_WLEN];

ULONG    pubWav_delay = 0;

#include "Waves_rev.h"
STATIC const UBYTE VersTag[] = VERSTAG;

/* ------------------------------------------------------------------- */
/* ---- Defaults() --------------------------------------------------- */
/* ------------------------------------------------------------------- */

VOID Defaults( PrefObject *Prefs )
{
    Prefs[PREF_SPOTS].po_Level = 3;
    Prefs[PREF_WLEN].po_Level = 50;
    Prefs[PREF_SIZ].po_Level = 4;
    Prefs[PREF_DELAY].po_Level = 15;
    Prefs[MODE].po_ModeID = getTopScreenMode();
    Prefs[MODE].po_Depth = 5;
}

/* ------------------------------------------------------------------- */
/* ---- BuildTmpRaster() --------------------------------------------- */
/* ------------------------------------------------------------------- */

BOOL	BuildTmpRaster( void ) {
    
    WORD	i;
    
    if (pubWav_trp = (struct RastPort *)malloc(sizeof(struct RastPort))) {
		
		InitRastPort(pubWav_trp);
		
		pubWav_trp->Layer = NULL;
		
		if (pubWav_tbm = (struct BitMap *)malloc(sizeof(struct BitMap))) {
			
			InitBitMap(pubWav_tbm, pubWav_tdpth, pubWav_tw, 1);
			
			/* alloc planes */
			for (i = 0; i < pubWav_tdpth; i++)
				if (! (pubWav_tbm->Planes[i] = AllocRaster(pubWav_tw,4)))
					break; 
			
			if (i != pubWav_tdpth)
				return(FALSE);
			else  {
				pubWav_trp->BitMap = pubWav_tbm;
				return(TRUE);
			}
			
		}
		else
			return(FALSE);
		
    }
    else
		return(FALSE);
}

/* ------------------------------------------------------------------- */
/* ---- FreeTmpRaster() ---------------------------------------------- */
/* ------------------------------------------------------------------- */

VOID	FreeTmpRaster( void ) {
    
    WORD	i;
    
    if (pubWav_tbm) {
		
		for (i = 0; i < pubWav_tdpth; i++)
			if (pubWav_tbm->Planes[i]) FreeRaster(pubWav_tbm->Planes[i],pubWav_tw,4); 
		
		free(pubWav_tbm);
    }
    
    if (pubWav_trp) free(pubWav_tbm);
}

/* ------------------------------------------------------------------- */
/* ---- SetUpField() ------------------------------------------------- */
/* ------------------------------------------------------------------- */

LONG	SetUpField( void ) {
    /*
     **  Make waves.
     */
    register	UBYTE	*line;
    register 	WORD x;
    register 	WORD y;
    register 	WORD i;
    
    LONG	RetVal;
    WORD	c, oldc = -1;
    float	t, dum;
    LONG	t1, t2, dist, wsum, max_sum, wp;
    
    
    if ((line = (UBYTE *)malloc(pubWav_tw)) == NULL) return(FAILED);
    
    t = PIx2 / pubWav_wlen;
    
    dum = 0.;
    i = 0;
    while (i < pubWav_wlen) {
		pubWav_sin[i] = sin(dum) * MAX_WABS + 0.5;
		dum += t;
		i++;
    } 
    
    for (i = 0; i < pubWav_spots; i++) {
		pubWav_s[i].x = lrand48() % Wid;
		pubWav_s[i].y = lrand48() % Hei;
    }
    
    wp = MAX_WABS * pubWav_spots;
    max_sum = wp * 2;
    
    for (y = 0; y < Hei; y++) {
		for (x =0; x < Wid; x++) {
			wsum = 0;
			for (i = 0; i < pubWav_spots; i++) {
				
				t1 = (pubWav_s[i].x - x);
				t2 = (pubWav_s[i].y - y);
				
				dist = t1*t1 + t2*t2;
				
				dist = sqrt((float)dist) + 0.5;
				/*
				 **		t1 = 0;
				 **		while ((t1 * t1) < dist) t1++;
				 **		dist = t1 - 1;
				 */
				
				wsum += pubWav_sin[dist % pubWav_wlen];
			}
			wsum += wp;
			
			/*	    c = ((float)wsum * (Cl-1)) / (float)max_sum + 0.5;  */
			
			c = (wsum * (Cl-2)) / max_sum + 1;
			
			/* old engine 
			 **	    if (pubWav_siz == 1) {
			 **		line[x] = c;
			 **	    }
			 **	    else {
			 **		if (c != oldc) { oldc = c; SetAPen(pubWav_rp,c); }
			 **		t1 = x * pubWav_siz;
			 **	    	t2 = y * pubWav_siz;
			 **		RectFill(pubWav_rp,t1,t2,t1+pubWav_siz-1,t2+pubWav_siz-1);
			 **	    }
			 **
			 **	    if ((RetVal = ContinueBlanking()) != OK) goto lwav_dskip;
			 **	}
			 **
			 **	if (pubWav_siz == 1)
			 **	    WritePixelLine8(pubWav_rp,0,y,pubWav_tw,line,pubWav_trp); 
			 */
			
			
			/* new engine */
			
			t1 = x * pubWav_siz;
			for (i = 0; i < pubWav_siz; i++) line[t1 + i] = c;
			
			if ((RetVal = ContinueBlanking()) != OK) goto lwav_dskip;
		}
		
		t2 = y * pubWav_siz;
		WritePixelLine8(pubWav_rp,0,t2,pubWav_tw,line,pubWav_trp); 
		for (i = 1; i < pubWav_siz; i++) 
			BltBitMap(pubWav_rp->BitMap,0,t2,pubWav_rp->BitMap,0,t2+i, \
					  pubWav_tw,1,0x0C0,0xFF,NULL);
    }
    
 lwav_dskip:
    if (line) free(line);
    
    return(RetVal);
}




/* ------------------------------------------------------------------- */
/* ---- WavesDriver() ------------------------------------------------ */
/* ------------------------------------------------------------------- */

LONG	WavesDriver( void ) {
    
    LONG	RetVal;
    ULONG	scr2front = 0, delay_count;
    WORD	i;
    ULONG	*ctab, tr, tg, tb, c;
    UBYTE	rf, gf, bf;
    UWORD	*OldCTab;
    float	dum;
    
    if (!BuildTmpRaster()) { RetVal = FAILED; goto lwav_err;  }
    
    
    /*    ctab = NULL; */
    ctab = GetColorTable(pubWav_sc); 
    
    rf = lrand48() & 1;
    gf = lrand48() & 1;
    bf = lrand48() & 1;
    if ((rf + gf +bf) == 0) bf = 1;
    
    
    if (ctab) {
		
    	ctab[1] = 0;
    	ctab[2] = 0;
    	ctab[3] = 0;
		
		dum = 255.0/(Cl-2);
		
    	for (i = 1; i < (Cl/2 + 1); i++) {
			
			c = (i -1) * 2.0 * dum;
			if (c > 0xFF) c = 0xFF;
			c &= 0xFF;
			
			c = c | (c << 8) | (c << 16) | (c<<24);
			
			ctab[i*3+1] = c * rf; 	        
			ctab[i*3+2] = c * gf; 	        
			ctab[i*3+3] = c * bf; 	        
			
			ctab[(Cl-i)*3+1] = c * rf; 	        
			ctab[(Cl-i)*3+2] = c * gf; 	        
			ctab[(Cl-i)*3+3] = c * bf; 	        
		}
    	LoadRGB32(&(pubWav_sc->ViewPort), ctab);
    }
    else {
		
		OldCTab = (pubWav_sc->ViewPort.ColorMap)->ColorTable;
		OldCTab[0] = 0;
		
		dum = 15.0/(Cl-2);
		
		if (dum == 0) dum = 1;
		
    	for (i = 1; i < (Cl/2 + 1); i++) {
			
			c = (i -1) * 2 * dum;
			if (c > 0xF) c = 0xF;
			c &= 0xF;
			
			if (gf)  c |= (c << 4);
			if (bf)  c |= (c << 8);
			if (!rf) c &= 0xFF0;
			
			OldCTab[i] = c; 	        
			OldCTab[Cl-i] = c; 	        
		}
    	LoadRGB4(&(pubWav_sc->ViewPort), OldCTab, Cl);
    }
    
    RetVal = SetUpField(); 
    
    delay_count = 0;
    while (RetVal == OK) {
		
		if ((scr2front++ % 100) == 0) ScreenToFront(pubWav_sc); 
		
		WaitTOF(); 
		
		
		if (!pubWav_delay || ((delay_count++ % pubWav_delay) == 0)) {
			
			if (ctab != NULL) {
				
				tr = ctab[4];
				tg = ctab[5];
				tb = ctab[6];
				for (i = 1; i < (Cl-1); i++) {
					ctab[i*3 + 1] = ctab[(i+1)*3 +1];
					ctab[i*3 + 2] = ctab[(i+1)*3 +2];
					ctab[i*3 + 3] = ctab[(i+1)*3 +3];
				}
				ctab[Cl*3 - 2] = tr;
				ctab[Cl*3 - 1] = tg;
				ctab[Cl*3 ] = tb;
    	        LoadRGB32(&(pubWav_sc->ViewPort), ctab);
			}
			else {
				tr = OldCTab[1];
				for (i = 1; i < (Cl-1); i++) OldCTab[i] = OldCTab[i+1];
				OldCTab[Cl - 1] = tr;
				LoadRGB4(&(pubWav_sc->ViewPort), OldCTab, Cl);
			}
		}	
        if ((RetVal = ContinueBlanking()) != OK) break;
    }
    
    if (ctab) FreeVec(ctab);
    
 lwav_err:
    FreeTmpRaster();
    
    return(RetVal);
}




/* ------------------------------------------------------------------- */
/* ---- Blank() ------------------------------------------------------ */
/* ------------------------------------------------------------------- */

LONG Blank( PrefObject *Prefs )
{
    /*
     **  Opens screen, initializes color table, preference values and random
     **  numbers generator seed, then calls <WaveDriver()>.
     */
    struct Window *Wnd;
    LONG  RetVal;
    
    srand48(time(0L));
    
    if (Prefs[MODE].po_Depth < 3) Prefs[MODE].po_Depth = 3;
    
    pubWav_sc = OpenScreenTags( NULL,
							   SA_Depth, Prefs[MODE].po_Depth,
							   SA_Quiet, TRUE,
							   SA_DisplayID, Prefs[MODE].po_ModeID,
							   SA_Behind, TRUE,
							   SA_Overscan, OSCAN_STANDARD,
							   TAG_DONE );
    if( pubWav_sc )
    {
		
		ScreenToFront(pubWav_sc);
		
		pubWav_rp = &pubWav_sc->RastPort;
		
		Cl = 1L << Prefs[MODE].po_Depth;
		
		pubWav_tdpth = Prefs[MODE].po_Depth;
		pubWav_tw = pubWav_sc->Width;
		
		pubWav_spots = Prefs[PREF_SPOTS].po_Level;
		pubWav_delay = Prefs[PREF_DELAY].po_Level & 0xFFFF;
		pubWav_wlen = Prefs[PREF_WLEN].po_Level;
		pubWav_siz = Prefs[PREF_SIZ].po_Level;
		
		
		Wid = pubWav_sc->Width / pubWav_siz;
		Hei = pubWav_sc->Height / pubWav_siz;
		
		/*    		ColorTable = RainbowPalette( pubWav_sc, 0L, 1L, 0L );  */
		
		Wnd = BlankMousePointer( pubWav_sc );
		
		RetVal = WavesDriver();
		
		UnblankMousePointer( Wnd );
		
		/*		RainbowPalette( 0L, ColorTable, 1L, 0L );	*/
		
		CloseScreen( pubWav_sc );
    }
    else
		RetVal = FAILED;
    
    return(RetVal);
}
