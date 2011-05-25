/* Water blanker
 *
 * written 13.-14.02.95, Karlheinz Agsteiner
 * Based on the dragon blanker 
 *
 * This blanker uses some hacks (which don't break our code, of course)
 * 1. unsigned integers are used in function Water() to represent coordinates.
 *    This has consequences:
 *    negative coordinates can occur, ie. overflows take place.
 *    This is actually good for us since our clipping gets easier:
 *     signed: if (x>=0 && x <= width), unsigned: if (x <= width)
 * 2. Constants are set nicely (VARIATIONS=32 allows SAS/C to 
 *    replace a[i][j]=a+i*VARIATIONS+j by a+SomeShiftsAndAdds(i)+j
 * 3. Instead of coloring every dot perfectly with color
 *    variation*colors/<max.variation for this point> we color it 
 *    with variation*colors/<limit of variations>.
 *    This is only an approximation and in some cases (when the
 *    dots "drop" out of the screen) too few colors are used.
 *    But it allows us to precompute this and replace a multiplication
 *    plus division by one array access.
 *    Our new approximation is much better, btw. It uses instead of
 *    the theoretical limit the longest path actually reached by a
 *    drop. Since the paths are roughly of the same length this is
 *    nearly perfect.
 * Altogether, our core loop in Water() only uses some moves, shifts,
 * adds, subs and jumps but nothing else. */

#include <exec/memory.h>
#include <math.h>
#include <stdlib.h>

#include "/includes.h"

#define FLOATRAND() (((double)RangeRand(10000))/10000)

#define bool int
#define TRUE 1
#define FALSE 0

#define STAGES     100
#define VARIATIONS 32
#define XOFFMAX    50
#define YOFFMAX    10
#define ZEIT       800

#define USEUNSIGNED	/* use unsigned instead of signed (hack 1) */
#define ESTIMATE        /* estimate the colors instead of using their real 
                         * values (hack 3) */
/*#define FASTBACK*/    /* don't compute new random paths and coords when 
                         * the drop reaches bottom */

#define LIMIT -10000000

Triplet *ColorTable = 0L;

#ifdef USEUNSIGNED
#define integer unsigned int
#else
#define integer int
#endif

typedef struct { integer x,y; } vector;
typedef struct { int x,y; } signedvector;

signedvector pattern[VARIATIONS][STAGES];
unsigned int maxstage[VARIATIONS];
vector       *point = NULL; /* [POINTS] */
unsigned int *thepattern = NULL; /* [POINTS] */
unsigned int *thestage = NULL; /* [POINTS] */

#ifdef ESTIMATE
int stagecolor[STAGES];
#endif

#define DROPS 0
#define TIME  2
#define MODE  4

VOID Defaults( PrefObject *Prefs )
{
  Prefs[DROPS].po_Level = 600;     
  Prefs[TIME].po_Level = 400;
  Prefs[MODE].po_ModeID = getTopScreenMode();
  Prefs[MODE].po_Depth = 4;
}


LONG init_structure(int width, int height, int points, int *maxoffset)
{ int var,stage,thepoint,xwidth;
  bool onedir,whichdir,dropout,dropdown;
  double vyoff,vxoff,axoff,ymax;
  LONG flg_end;
  onedir = (bool)RangeRand(2);
  whichdir = (bool)RangeRand(2);
  
  if (RangeRand(2))
    vyoff = -25*FLOATRAND();
  else
    vyoff = 0;
  vxoff = (double)RangeRand(5);
  axoff = FLOATRAND()*0.5;
  ymax = (double)RangeRand(10)+500.0;
  dropout = (bool)RangeRand(2);
  dropdown = (bool)RangeRand(2);

  /* Compute VARIATIONS patterns of STAGES stages each */
  for (var = 0; var < VARIATIONS; var++)
    { double ax,ay,vx,vy,x,y;
      bool   finished;

      flg_end = ContinueBlanking();
      if (flg_end != OK)
	{ return flg_end; }

      ax = 0; ay = FLOATRAND()*0.2+axoff+0.2;
      if (onedir)
	{ vx = FLOATRAND()*3+vxoff;
        }
      else
        vx = FLOATRAND()*6-3;
      if (whichdir)
        vx = -1*vx;

      vy = -1*(FLOATRAND()*3)+vyoff;
      x = 0; y = 0;
      finished = FALSE;
      stage = 0;
      while (stage < STAGES && !finished)
        { x = x + vx;
          y = y + vy;
          if (y > ymax)
            { if (dropout)
                { if (!dropdown) finished = TRUE;
                }
              else
                { y = y - vy;
                  vy = - vy/2;
                  x = x - 3*vx;
                }
            }
          vx = vx + ax;
          vy = vy + ay;
          if (finished)
            { pattern[var][stage].x = LIMIT;
              maxstage[var] = stage-1;
            }
          else
            { pattern[var][stage].x = (int)x;
              pattern[var][stage].y = (int)y;
            }
          stage++;
        }
      if (!finished)
	{ maxstage[var] = STAGES;
	}
    }

  /* Normalize the resulting patterns */
  /*   first: find the limits */
  { int minx,maxx,miny,maxy,counter;
    minx = pattern[0][0].x; maxx = pattern[0][0].x;
    miny = pattern[0][0].y; maxy = pattern[0][0].y;
    for (var=0; var<VARIATIONS; var++)
      { stage=0;
        while (stage<STAGES && stage <= maxstage[var])
          { if (minx>pattern[var][stage].x)
              minx = pattern[var][stage].x;
            if (miny>pattern[var][stage].y)
              miny = pattern[var][stage].y;
            if (maxx<pattern[var][stage].x)
              maxx = pattern[var][stage].x;
            if (maxy<pattern[var][stage].y)
              maxy = pattern[var][stage].y;
            stage++;
          }
      }
   
   /*  then: transform [minx..maxx] -> [0..width], y -> [0..height] */
   xwidth = RangeRand(width-10)+10;
   *maxoffset = width-xwidth;
   for (var=0; var<VARIATIONS; var++)
     { stage = 0;
       while (stage<STAGES && stage <= maxstage[var])
         { pattern[var][stage].x = ((pattern[var][stage].x-minx)*xwidth)/(maxx-minx);
           pattern[var][stage].y = ((pattern[var][stage].y-miny)*height)/(maxy-miny);
           stage++;
         }
     }
       
   /* Define the points (initial offset, variation, stage) */
   counter = 0;
   for (thepoint = 0; thepoint < points; thepoint++)
     { point[thepoint].x = RangeRand(20)-10;
       point[thepoint].y = RangeRand(10)-5;
       thepattern[thepoint] = RangeRand(VARIATIONS);
       thestage[thepoint] = RangeRand(maxstage[thepattern[thepoint]]);
       counter++;
       if (counter == 100)
         { flg_end = ContinueBlanking();
           if (flg_end != OK)
	     { return flg_end; }
           counter = 0;
         }
     }
  }
  return OK;
}
          


LONG Water( struct Screen *Scr, SHORT Width, SHORT Height, unsigned int points, unsigned int time)
{ LONG flg_end;
  unsigned int Wid=Width;
  unsigned int Hei=Height;
  unsigned int i,k,offset;
  struct RastPort *rp = &( Scr->RastPort );
  int colors;

  /* Initialize dynamic data structures */
  point = malloc(points * sizeof(vector));
  if (point == NULL) return FAILED;
  thepattern = malloc(points * sizeof(unsigned int));
  if (thepattern == NULL) return FAILED;
  thestage = malloc(points * sizeof(unsigned int));
  if (thestage == NULL) return FAILED;

  colors = (1L << Scr->BitMap.Depth);

  do { flg_end = init_structure(Wid,Hei,points,&offset);
#    ifdef ESTIMATE
       { unsigned int i,maxst;
         maxst = 0;
         for (i=0; i<VARIATIONS; i++)
           { if (maxst < maxstage[i])
       	     maxst = maxstage[i];
           }
         for (i=0; i<STAGES; i++)
           { stagecolor[i] = i*colors/maxst;
             if (stagecolor[i] == 0) stagecolor[i]=1;
           }
       }
#    endif
       offset = RangeRand(offset);
       if (flg_end != OK)
         { free(thestage);
	   free(thepattern);
	   free(point);
	   return flg_end;
         }
       for (k=0; k<time; k++)
         { for (i=0; i<points; i++)
             { integer xoff = point[i].x;
               integer yoff = point[i].y;
               unsigned int whichpat = thepattern[i];
               unsigned stage = thestage[i];
               integer x = xoff + (integer)pattern[whichpat][stage].x + offset;
               integer y = yoff + (integer)pattern[whichpat][stage].y;
#            ifndef ESTIMATE
	       unsigned int mswp = maxstage[whichpat];
#            endif
               stage++;
               SetAPen(rp,0);
#            ifdef USEUNSIGNED
               if (x < Wid && y < Hei)
#            else
               if (x >= 0 && y >= 0 && x < Wid && y < Hei) 
#            endif
                 WritePixel(rp,x,y);
#            ifdef ESTIMATE
               if (stage >=maxstage[whichpat])
#            else
               if (stage >= mswp)
#            endif
                 { 
#                ifndef FASTBACK
                   thepattern[i] = RangeRand(VARIATIONS);
#                endif
                   thestage[i]   = 0;
#                ifndef FASTBACK
                   point[i].x = RangeRand(20)-10;
                   point[i].y = RangeRand(10)-5;
#                endif
                 }
               else
                 { thestage[i] = stage;
                   x = xoff + (integer)pattern[whichpat][stage].x + offset;
                   y = yoff + (integer)pattern[whichpat][stage].y;
#                ifdef ESTIMATE
                   SetAPen(rp,stagecolor[stage]);
#                else
                   SetAPen(rp,(stage*colors)/mswp);
#                endif
#                ifdef USEUNSIGNED
                   if (x < Wid && y < Hei)
#                else
                   if (x >= 0 && y >= 0 && x < Wid && y < Hei) 
#                endif
                     WritePixel(rp,x,y);
                 }
             }
           ScreenToFront( Scr );
           flg_end = ContinueBlanking();
           if (flg_end != OK) 
	     { free(thestage);
	       free(thepattern);
	       free(point);
	       return flg_end;
	     }
         }
       SetRast(rp,0);
     } while (flg_end == OK);
  free(thestage);
  free(thepattern);
  free(point);
  return OK;
}



LONG Blank( PrefObject *Prefs )
{
	struct Screen *Scr;
	struct Window *Wnd;
	LONG RetVal;
        int colors;
	
	if( Scr = OpenScreenTags( NULL, SA_Depth, Prefs[MODE].po_Depth,
							 SA_Quiet, TRUE, SA_DisplayID, Prefs[MODE].po_ModeID,
							 SA_Behind, TRUE, SA_Overscan, OSCAN_STANDARD,
							 TAG_DONE ))
	{
		SetRGB4(&( Scr->ViewPort ), 0, 0, 0, 0 );
                colors = (1 << Prefs[MODE].po_Depth);
		if (colors <= 32)
		  { int max = 7;
                    int i;
		    for (i=1; i<colors; i++)
		      { SetRGB4(&( Scr->ViewPort ), i, (max*i)/colors+8,(max*i)/colors+8,15);
                      } 
                  }
                else
                  { int max = 127;
                    int i;
		    for (i=1; i<colors; i++)
		      { SetRGB32(&( Scr->ViewPort ), i, 256*256*256*((max*i)/colors+128),
                                                        256*256*256*((max*i)/colors+128),256*256*256*255);
                      } 
                  }
		Wnd = BlankMousePointer( Scr );
		
		do
			RetVal = Water( Scr, Scr->Width, Scr->Height , Prefs[DROPS].po_Level,
			                Prefs[TIME].po_Level);
		while( RetVal == OK );
		
		UnblankMousePointer( Wnd );
		CloseScreen( Scr );
	}
	else
		RetVal = FAILED;
	
	return RetVal;
}
