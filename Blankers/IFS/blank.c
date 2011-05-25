/* --------------------------------------------------------
 *   An iterated function system blanker inspired by
 *   Xlock's fire blanker
 *                                           010294 kha.
 * -------------------------------------------------------- */

#include <exec/memory.h>
#include <stdlib.h>
#include <math.h>

#include "/includes.h"

#define WINCO(SIZE,MAX) ((int) (( (int)((SIZE+2.0)*MAX) ) / 4 ) )
#define FLOATRAND() (((double)RangeRand(10000))/10000)
#define bool int
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif


#define FNUM 10
#define MOVEFACTORA 0.6		/* the limit of the transposition is x: */
#define MOVEFACTORB 1.0		/* P(x=0.6)=P(x=2.0)=1/4, p(x=1.0)=1/2 */
#define MOVEFACTORC 2.0		
#define SIZEFACTOR 2.0		/* SIZEFACTOR/2 is the reduction factor */
#define ITER 100	        /* max number of iterations for one plot.
                                 * Setting this too large will cause the
                                 * program to react a bit slow to CTRL-C */

Triplet *ColorTable = 0L;

#define PREF_FUNCTIONS 0
#define PREF_ITERATIONS 2
#define PREF_AREA 4
#define PREF_TRANSPOSE 5
#define PREF_CONTPROB 7
#define PREF_DECREASE 9
#define PREF_MODE 11

VOID Defaults( PrefObject *Prefs )
{
  Prefs[PREF_FUNCTIONS].po_Level = 6;
  Prefs[PREF_ITERATIONS].po_Level = 300;
  Prefs[PREF_AREA].po_Level = 0;
  Prefs[PREF_TRANSPOSE].po_Level = 0;
  Prefs[PREF_CONTPROB].po_Level = 110;
  Prefs[PREF_DECREASE].po_Level = 10;
  Prefs[PREF_MODE].po_ModeID = getTopScreenMode();
  Prefs[PREF_MODE].po_Depth = 3;
}

LONG IFS( struct Screen *scr, SHORT width, SHORT height, PrefObject *Prefs )
{
  LONG flg_end = OK;
  int colors = (1L << scr->BitMap.Depth);
    double x,y,			/* the coordinate which gets computed */
           prob;		/* probability for continuing withot Clscr */
    int num,			/* total number of fctns for the current IFS */
        thef;			/* the currently applied function */
    int i,j,k;			/* multi-purpose counters, array indices */
    int wx,wy;			/* window coordinate of current point */
    int tx,ty;			/* # pixels to transpose the Origin (x,y) */

    double aa[FNUM][4],		/* For each f: a 2x2 matrix and */
           vv[FNUM][2],		/*             a transposition vector */
           comp[FNUM],		/* the compression of each function (1/comp[n]
                                 * is the compression of the function) */
           totcomp;		/* sum of all the size factors. The larger this
                                 * is, the longer we should compute */
    double CONTINUEPROBABILITY,CONTPROBDECREASE;
    bool TRANSPOSE,USEPROBABILITY,oncedone;
    int LENGTH,FUNCTIONS;

    struct RastPort *rp = &( scr->RastPort );

    CONTINUEPROBABILITY = ((double)Prefs[PREF_CONTPROB].po_Level)/100.0;
    CONTPROBDECREASE    = ((double)Prefs[PREF_DECREASE].po_Level)/100.0;
    USEPROBABILITY      = !(Prefs[PREF_AREA].po_Level);
    TRANSPOSE           = !(Prefs[PREF_TRANSPOSE].po_Level);
    LENGTH              = Prefs[PREF_ITERATIONS].po_Level;
    FUNCTIONS           = Prefs[PREF_FUNCTIONS].po_Level;
    
    while (1)
      { Move(rp,0,0);
        SetRast(rp,0);
        SetAPen(rp,1);
        ScreenToFront( scr );

        prob = CONTINUEPROBABILITY;
        oncedone = FALSE;
        while (((double)RangeRand(1000))/1000 < prob || !oncedone) {
        oncedone = TRUE;
        prob = prob - CONTPROBDECREASE;
        num = RangeRand(FUNCTIONS-2)+2;
        
        /* Compute the iterated function */
        totcomp = 0;
        { int moverand = RangeRand(4);

          /* select a move factor with the appropriate probability */
          double MOVEFACTOR;
          switch(moverand)
            { case 0: MOVEFACTOR = MOVEFACTORA; break;
              case 1: 
              case 2: MOVEFACTOR = MOVEFACTORB; break;
              case 3: MOVEFACTOR = MOVEFACTORC; break;
              default: MOVEFACTOR = MOVEFACTORB; break; /* should never happen */
            }

          /* compute the functions and their compression */
          for (k=0; k<num; k++)
              { for (i=0; i<2; i++)
                    { vv[k][i] = (FLOATRAND() - 0.5) * MOVEFACTOR;
                    }
                for (i=0; i<4; i++)
                    { aa[k][i] = (FLOATRAND() - 0.5) * SIZEFACTOR;
                    }
                comp[k] = abs(aa[k][0]*aa[k][2]-aa[k][1]*aa[k][3]);
                totcomp = totcomp + comp[k];
              }
        }

        /* and a probability distribution, eg. 
         * 2,3,1,4 -> 0.2, 0.3, 0.1, 0.4 -> 0.2, 0.5, 0.6, 1.0 */
        for (k=0; k<num; k++)
            { comp[k] = comp[k]/totcomp;
              if (k>0)
                 { comp[k] = comp[k]+comp[k-1];
                 }
            }

        comp[num-1] = 1.1; /* just to be safe of numerical errors, we increment
                            * the final probability a bit */

        /* Initialize the transpositions of the origin */
        if (TRANSPOSE)
          {
            tx = (int)(RangeRand(width/2))-width/4;
            ty = (int)(RangeRand(height/2))-height/4;
          }
        else
          { tx = ty = 0;
          }


        /* set the color for the function */
        SetAPen(rp,(int)(RangeRand(colors-1)+1));

        /* and now: compute the IFS. We do this using several iterations
         * instead of just one big step. This gives a picture which looks
         * a bit more structured. */
        for (i=0; i<(int)(LENGTH*totcomp); i++)
            { x=y=0;
              for (j=0; j<ITER; j++)
                  { double xx;

                    /* Find a function to use */
                    if (USEPROBABILITY)
                       { double p;
                         p = FLOATRAND();
                         thef = 0;
                         while (comp[thef] < p)
                           { thef++;
                           }
                       }
                    else
                       { thef = RangeRand(num);
                       }

                    /* compute (x,y) = (x,y)*aa + vv */
                    xx = (x*aa[thef][0]+y*aa[thef][1])+vv[thef][0];
                    y = (x*aa[thef][2]+y*aa[thef][3])+vv[thef][1];
                    x = xx;
                    
                    /* Convert it to a window coordinate and plot it */
                    wx = WINCO(x,width);
                    wy = WINCO(y,height);
                   
                    /* And transpose it, if necessary */
		    wx = wx + tx;
		    wy = wy + ty;
		    
		    if (wx > 0 && wx < width && wy > 0 && wy < height)
                       WritePixel(rp,wx,wy);
                  }
              ScreenToFront( scr );
              flg_end = ContinueBlanking();
              if (flg_end != OK)
		{ return flg_end;
		}
            }
        }
      }
    return flg_end;
}
  


/* Adopted from the Dragon blanker */
LONG Blank( PrefObject *Prefs )
{
	struct Screen *Scr;
	struct Window *Wnd;
	LONG RetVal;
	
	if( Scr = OpenScreenTags( NULL, SA_Depth, Prefs[PREF_MODE].po_Depth,
							 SA_Quiet, TRUE, SA_DisplayID, Prefs[PREF_MODE].po_ModeID,
							 SA_Behind, TRUE, SA_Overscan, OSCAN_STANDARD,
							 TAG_DONE ))
	{
		SetRGB4(&( Scr->ViewPort ), 0, 0, 0, 0 );
		ColorTable = RainbowPalette( Scr, 0L, 1L, 0L );
		Wnd = BlankMousePointer( Scr );
		
		do
			RetVal = IFS( Scr, Scr->Width, Scr->Height, Prefs );
		while( RetVal == OK );
		
		UnblankMousePointer( Wnd );
		RainbowPalette( 0L, ColorTable, 1L, 0L );
		CloseScreen( Scr );
	}
	else
		RetVal = FAILED;
	
	return RetVal;
}
