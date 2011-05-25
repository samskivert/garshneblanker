/* --------------------------------------------------------
 *   Recursively draws cubes.
 *   
 *                                           010294 kha.
 * -------------------------------------------------------- */

#include <exec/memory.h>
#include <math.h>

#include "/includes.h"


#define WINCO(SIZE,MAX) ((int) (( (int)((SIZE+2.0)*MAX) ) / 4 ) )
#define bool int
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

static LONG male(int factor, int p, int xlimit, int ylimit, int wx1, int wy1, int wx2, int wy2, struct RastPort *rp);
static  __inline void malequadrat(int x1,int y1, int x2, int y2, struct RastPort *rp);


Triplet *ColorTable = 0L;

int COLORS;
int OFFSET;
int DEPTH;
int RANDOMBORDER;
int MAXBORDER;

VOID Defaults( PrefObject *Prefs )
{
	Prefs[0].po_ModeID = getTopScreenMode();
	Prefs[0].po_Depth = 4;
}

LONG Cubes( struct Screen *scr, SHORT width, SHORT height )
{
  LONG flg_end = OK;
  struct RastPort *rp = &( scr->RastPort );
  int wx,wy,wxsize,wysize;
    int factor;
    int minxsize;
    int minysize;
    int p;
    int i;
	
  wxsize = width-1; wysize = height-1;
  SetRast( rp, 0 );
  ScreenToFront( scr );

    while (1)
      { /* bestimme naechste Quadrate */
        switch(RangeRand(8))
          { case 0: case 1: case 2: case 3:
              { /* the default configuration */
                factor = RangeRand(3) + 2;  /* [x/factor] vs. [factor-x/factor] */
                minxsize = RangeRand(20)+3; /* end computation if x-size < minxsize */
                minysize = RangeRand(12)+2; /* or if y-size < minysize (plus borders) */
                p = RangeRand(10) + 5;      /* probability of "random computation abort"
                                       * = 1/p */
                break;
              }
            case 4: case 5: case 6:
              { /* BIG rectangles */
                factor = RangeRand(4) + 2;
                minxsize = RangeRand(30)+3;
                minysize = RangeRand(15)+2;
                p = RangeRand(5) + 5;
                break;
              }
            case 7: 
              { /* tiny rectangles */
                factor = RangeRand(4) + 2;
                minxsize = RangeRand(5) + 2;
                minysize = RangeRand(3) + 1;
                p = RangeRand(20) + 10;
                break;
              }
            default:
              { /* OOOPS */
                factor = 3;
                minxsize = 100;
                minysize = 100;
                p = 2;
                break;
              }
          }

        OFFSET = 0;
        COLORS = (1 << scr->BitMap.Depth);
        /* Color options */
	if (COLORS > 8 && RangeRand(4))
           { if (RangeRand(2))
	       { /* select the first 8 colors */
                 OFFSET = RangeRand(COLORS-8);
                 COLORS = 8;
               }
             else
               { OFFSET = RangeRand(COLORS*3/4);
                 COLORS = COLORS / 4;
               }
           }

        RANDOMBORDER = RangeRand(2);
        MAXBORDER = 3;
        if (!RANDOMBORDER) { MAXBORDER = RangeRand(4); }

        DEPTH=0;
        if ((flg_end = male(factor,p,minxsize,minysize,0,0,wxsize,wysize,rp)) != OK)
           { return flg_end;
           }
        for (i=0; i<10; i++)
          { Delay(25); /* mehr als eine halbe Sekunde merkt man */
            ScreenToFront(scr);
	    flg_end = ContinueBlanking();
	    if (flg_end != OK) return flg_end;
          }
      }
  }



static LONG male(int factor, int p, int xlimit, int ylimit, int wx1, int wy1, int wx2, int wy2, struct RastPort *rp)
  { LONG result = ContinueBlanking();
    if (result != OK) return result;

    if (wx2-wx1 < xlimit+2*MAXBORDER || wy2 - wy1 < ylimit+2*MAXBORDER || !(RangeRand(p)||DEPTH==0))
       { malequadrat(wx1,wy1,wx2,wy2,rp);
         return OK;
       }
    else
       { int xmid, ymid, q;
         DEPTH++; 
         if (DEPTH>200)
           { malequadrat(wx1,wy1,wx2,wy2,rp);
             return OK;
           }
         q = RangeRand(factor)+1; xmid = (wx1 * q + wx2 * (factor - q)) / factor;
         q = RangeRand(factor)+1; ymid = (wy1 * q + wy2 * (factor - q)) / factor;
         if ((result = male(factor,p,xlimit,ylimit,wx1,wy1,xmid-1,ymid-1,rp)) != OK)
            { return result;
            }
         if ((result = male(factor,p,xlimit,ylimit,xmid,ymid,wx2,wy2,rp)) != OK)
            { return result;
            }
         if ((result = male(factor,p,xlimit,ylimit,xmid,wy1,wx2,ymid-1,rp)) != OK)
            { return result;
            }
         result = male(factor,p,xlimit,ylimit,wx1,ymid,xmid-1,wy2,rp);
	 DEPTH--;
         return result;
       }
  }

static __inline void malequadrat(int x1,int y1, int x2, int y2, struct RastPort *rp)
  { int border;
    int color;
    
    if (RANDOMBORDER)
       { border = RangeRand(MAXBORDER);
       }
    else
       { border = MAXBORDER;
       }
    if ((x2-x1)>border*2 && (y2-y1)>border*2) {
    if (border)
       { SetAPen(rp,0);
         RectFill(rp,x1,y1,x2,y2);
         x1 = x1 + border; y1 = y1 + border;
         x2 = x2 - border; y2 = y2 - border;
       }
    color = RangeRand(COLORS-1)+1 + OFFSET; /* nicht farbe 0 */
    SetAPen(rp,color);
    RectFill(rp,x1,y1,x2,y2);
   }
 else if (x2>x1 && y2>y1)
       { SetAPen(rp,0);
         RectFill(rp,x1,y1,x2,y2);
       }
  }


/* Adopted from the Dragon blanker */
LONG Blank( PrefObject *Prefs )
{
	struct Screen *Scr;
	struct Window *Wnd;
	LONG RetVal;
	
	if( Scr = OpenScreenTags( NULL, SA_Depth, Prefs[0].po_Depth,
							 SA_Quiet, TRUE, SA_DisplayID, Prefs[0].po_ModeID,
							 SA_Behind, TRUE, SA_Overscan, OSCAN_STANDARD,
							 TAG_DONE ))
	{
		SetRGB4(&( Scr->ViewPort ), 0, 0, 0, 0 );
		ColorTable = RainbowPalette( Scr, 0L, 1L, 0L );
		Wnd = BlankMousePointer( Scr );
		
		do
			RetVal = Cubes( Scr, Scr->Width, Scr->Height );
		while( RetVal == OK );
		
		UnblankMousePointer( Wnd );
		RainbowPalette( 0L, ColorTable, 1L, 0L );
		CloseScreen( Scr );
	}
	else
		RetVal = FAILED;
	
	return RetVal;
}
