/* --------------------------------------------------------
 *    Draw nice pictures based on a simple mathematical
 *    formula
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

Triplet *ColorTable = 0L;

#define LINEPICS 0
#define VALRANGE 2
#define DURATION 4
#define DELAY 6
#define MODE 8

VOID Defaults( PrefObject *Prefs )
{
	Prefs[LINEPICS].po_Level = 3;
	Prefs[DURATION].po_Level = 100;
        Prefs[VALRANGE].po_Level = 10;
	Prefs[DELAY].po_Level = 0;
	Prefs[MODE].po_ModeID = getTopScreenMode();
	Prefs[MODE].po_Depth = 4;
}

LONG Sinus( struct Screen *scr, SHORT width, SHORT height, PrefObject *Prefs)
{
  LONG flg_end = OK;
  int colors = 1L << scr->BitMap.Depth;
  int bigc, smallc;
  int durationfactor,delay,lineprobability;  /* P = lineprobability / 7 */
  int minval,valrange;

  struct RastPort *rp = &( scr->RastPort );

  durationfactor  = Prefs[DURATION].po_Level * 100 + 1000;
  valrange        = Prefs[VALRANGE].po_Level;
  delay           = Prefs[DELAY].po_Level;
  lineprobability = Prefs[LINEPICS].po_Level;
  width--; height--;
  SetRast( rp, 0 );
  ScreenToFront( scr );

  minval = valrange/2;
  
  bigc = 0;
  while (bigc != 4096 && flg_end == OK)
    { double a,b,c,d,step;
      double x,y,z,delta,lasty,lastz;
      int wx,wy,limit,dbl;
      bigc++;
      a = (double) (RangeRand(valrange)-minval);
      b = (double) (RangeRand(valrange)-minval);
      c = (double) (RangeRand(valrange)-minval);
      d = (double) (RangeRand(valrange)-minval);
      step = ((double)RangeRand(1000))/10000 + 0.05;
        
      if (RangeRand(7) >= lineprobability)
        { if (RangeRand(2) == 0)
            step =  (PI/2) - step;
          else
	    step = PI + 0.05 + ((double)RangeRand(1000))/10000;
        }

      x = 0.0;
      limit = durationfactor / scr->BitMap.Depth;
      if (step > 1) limit = limit / 2;
      
      wx = WINCO(1,width);
      wy = WINCO(1,height);
      Move(rp,wx,wy);
      lasty = 1.0; lastz = 1.0;
      
      smallc=0;
      SetRast(rp,0);
      while (smallc<limit && flg_end == OK)
        { y = sin(a*x)+cos(b*x);
          z = sin(c*x)+cos(d*x);
          delta = fabs(y-lasty) + fabs(z-lastz);
	  SetAPen(rp,(int)(delta/4*(colors-1)+1));
          wx = WINCO(y,width);
          wy = WINCO(z,height);
          Draw(rp,wx,wy);
          x = x + step;
          lasty = y; lastz = z;
          if (smallc%64 == 0)
            { ScreenToFront(scr);
              flg_end = ContinueBlanking();
            }
	  smallc++;
        }
      if (flg_end == OK && delay > 0)
	{ int curr = 0;
          while (curr < delay * 10 && flg_end == OK)
	    { Delay(5);
	      ScreenToFront(scr);
	      flg_end = ContinueBlanking();
	      curr++;
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

	if( Scr = OpenScreenTags( NULL, SA_Depth, Prefs[MODE].po_Depth,
							 SA_Quiet, TRUE, SA_DisplayID, Prefs[MODE].po_ModeID,
							 SA_Behind, TRUE, SA_Overscan, OSCAN_STANDARD,
							 TAG_DONE ))
	{
		SetRGB4(&( Scr->ViewPort ), 0, 0, 0, 0 );
		ColorTable = RainbowPalette( Scr, 0L, 1L, 0L );
		Wnd = BlankMousePointer( Scr );
		
		do
			RetVal = Sinus( Scr, Scr->Width, Scr->Height, Prefs );
		while( RetVal == OK );
		
		UnblankMousePointer( Wnd );
		RainbowPalette( 0L, ColorTable, 1L, 0L );
		CloseScreen( Scr );
	}
	else
		RetVal = FAILED;
	
	return RetVal;
}
