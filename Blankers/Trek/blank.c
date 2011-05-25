/* --------------------------------------------------------
 *    Draw important lamps like in star trek
 *                                           010294 kha.
 * -------------------------------------------------------- */

#include <exec/memory.h>
#include <math.h>
#include <stdlib.h>

#ifdef DEBUG
#include <stdio.h>
#endif

#include "/includes.h"

#define bool int
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define MAXLENGTH 10
#define MAXLAMP   8000
#define WHITE_PEN 1
#define BLACK_PEN 2


Triplet *ColorTable = 0L;

/* One entry of a pulsing rhythm */
typedef struct
{ int color;                /* To which color should we switch */
  int delay;                /* How long should we keep this color */
} rhentry;

/* a complete rhytm sequence */
typedef struct 
{ int length;               /* How many items of data are filled */
  rhentry data[MAXLENGTH];  /* Array of the items */
} rhythm;
  
/* The properties of any lamp */
typedef struct
{ int    x,y,width,height;  /* The size on the screen */
  rhythm *rh;               /* A pointer to the rhythm we want */
  int    stage;             /* which data element of our rhythm is current */
  int    count;             /* How many delay steps have passed since the
                             * activation of this data element */
} lamp;


#ifdef DEBUG
int theheight;
int thewidth;
#endif


#define RHYTHMS 0
#define DURATION 2
#define DELAY 4
#define WIDTH 6
#define HEIGHT 8
#define SUBMODE 10
#define GREYBG 11
#define FREEMODE 13
#define MODE 15


void breakme(void)
{ while(1){}
}

VOID Defaults( PrefObject *Prefs )
{
        Prefs[RHYTHMS].po_Level = 20;
        Prefs[DURATION].po_Level = 500;
        Prefs[DELAY].po_Level = 1;
        Prefs[WIDTH].po_Level = 50;
        Prefs[HEIGHT].po_Level = 50;
        Prefs[SUBMODE].po_Level = 1;
        Prefs[GREYBG].po_Level = 1;
        Prefs[FREEMODE].po_Level = 50;
	Prefs[MODE].po_ModeID = getTopScreenMode();
	Prefs[MODE].po_Depth = 4;
}

int MINWIDTH;
int MINHEIGHT;
int submode;
int freemode;

/* Generate a set of num rhythm sequences for lamps with up to colors colors */
void generate_rhythms(rhythm *r, int num, int colors)
{ int i;
  rhythm *curr_r = r;
  rhentry *curr_e;

  for (i=0; i<num; i++)
    { curr_e = curr_r->data;
      switch(RangeRand(4))
	{ case 0:
	    /* Ungleichmaessig schnelles blinken */
	    { int d1 = RangeRand(10)+10;
	      int d2 = RangeRand(10)+10;
	      int color = RangeRand(colors-3)+3;
	      curr_r->length = 2;
	      curr_e->color = color;
	      curr_e->delay = d1;
	      curr_e++;
	      curr_e->color = 0;
	      curr_e->delay = d2;
	      break;
	    }
	  case 1:
	    /* Gleichmaessiges blinken */
	    { int d1 = RangeRand(10)+10;
	      int color = RangeRand(colors-3)+3;
	      curr_r->length = 2;
	      curr_e->color = color;
	      curr_e->delay = d1;
	      curr_e++;
	      curr_e->color = 0;
	      curr_e->delay = d1;
	      break;
	    } 
	  case 2:
	    /* Farbwechsel (regelmaessig) */
	    { int d1 = RangeRand(10)+10;
	      int i;
	      curr_r->length = MAXLENGTH;
	      for (i=0; i<MAXLENGTH; i++)
		{ curr_e->color = (i % 2 ? RangeRand(colors-3)+3 : 0);
		  curr_e->delay = d1;
		  curr_e++;
		}
	      break;
	    } 
	  } /* switch */
      curr_r++;
    } /* for */
}


int generate_lamp_row(lamp *l, struct RastPort *rp, int limit, bool vertical,
		      int x, int y, int width, int height, int rhythms, rhythm *r)
{ if (limit <= 0)
    { return 0;
    }
  else if (width < MINWIDTH || height < MINHEIGHT)
    { if (RangeRand(100)<freemode)
        { return 0;
        }
      else if (submode && (width/height > 1 || height/width > 1))
        { if (width/height > 1)
            { int factor;
              int remainder;
              int cx,cy,i;
              cx = x; cy = y;

              factor = width/height;
              if (factor > limit)
                { factor = limit;
                }
              remainder = width - factor * height;

              for (i=0; i<factor; i++)
                { int x1,y1,x2,y2;
                  x1 = x + i*height + (remainder*i)/factor;
                  y1 = y;
                  x2 = x1 + height-1;
                  y2 = y1 + height-1;
                  l->x = x1+1; l->y = y1+1;
                  l->width = height-2; l->height = height-2;
                  l->stage = 0;
                  l->count = 0;
                  l->rh = r+RangeRand(rhythms);
                  l++;
#ifdef DEBUG
                  if (x<0 || y<0 || x>thewidth || y>theheight)
              	    { printf("error! x,y = %d,%d\n",x,y);
                      breakme();
                    }
                  if (x+width<0 || x+width>thewidth || y+height<0 || y+height>theheight)
                    { printf("error! x+w,y+h = %d,%d\n",x+width,y+height);
                      breakme();
                      while(1) { printf(" "); }
                    }
#endif
                  SetAPen(rp,WHITE_PEN);
                  Move(rp,x1,y1);
                  Draw(rp,x1,y2);
                  Move(rp,x1,y1);
                  Draw(rp,x2,y1);
                  SetAPen(rp,BLACK_PEN);
                  Move(rp,x1,y2);
                  Draw(rp,x2,y2);
                  Move(rp,x2,y1);
                  Draw(rp,x2,y2);
                }
              return factor;
            }
          else
            { int factor;
              int remainder;
              int cx,cy,i;
              cx = x; cy = y;

              factor = height/width;
              if (factor > limit)
                { factor = limit;
                }
              remainder = height - factor * width;

              for (i=0; i<factor; i++)
                { int x1,y1,x2,y2;
                  y1 = y + i*width + (remainder*i)/factor;
                  x1 = x;
                  x2 = x1 + width-1;
                  y2 = y1 + width-1;
                  l->x = x1+1; l->y = y1+1;
                  l->width = width-2; l->height = width-2;
                  l->stage = 0;
                  l->count = 0;
                  l->rh = r+RangeRand(rhythms);
		  l++;
#ifdef DEBUG
                  if (x<0 || y<0 || x>thewidth || y>theheight)
              	    { printf("error! x,y = %d,%d\n",x,y);
                      breakme();
                    }
                  if (x+width<0 || x+width>thewidth || y+height<0 || y+height>theheight)
                    { printf("error! x+w,y+h = %d,%d\n",x+width,y+height);
                      breakme();
                      while(1) { printf(" "); }
                    }
#endif
                  SetAPen(rp,WHITE_PEN);
                  Move(rp,x1,y1);
                  Draw(rp,x1,y2);
                  Move(rp,x1,y1);
                  Draw(rp,x2,y1);
                  SetAPen(rp,BLACK_PEN);
                  Move(rp,x1,y2);
                  Draw(rp,x2,y2);
                  Move(rp,x2,y1);
                  Draw(rp,x2,y2);
                }
              return factor;
            }
        }
      else
        { int x2,y2;
          x2 = x+width-1;
          y2 = y+height-1;
          l->x = x+1; l->y = y+1;
          l->width = width-2; l->height = height-2;
          l->stage = 0;
          l->count = 0;
          l->rh = r+RangeRand(rhythms);
#ifdef DEBUG
          if (x<0 || y<0 || x>thewidth || y>theheight)
   	    { printf("error! x,y = %d,%d\n",x,y);
              breakme();
            }
          if (x+width<0 || x+width>thewidth || y+height<0 || y+height>theheight)
            { printf("error! x+w,y+h = %d,%d\n",x+width,y+height);
              breakme();
              while(1) { printf(" "); }
            }
#endif
          SetAPen(rp,WHITE_PEN);
          Move(rp,x,y);
          Draw(rp,x,y2);
          Move(rp,x,y);
          Draw(rp,x2,y);
          SetAPen(rp,BLACK_PEN);
          Move(rp,x,y2);
          Draw(rp,x2,y2);
          Move(rp,x2,y);
          Draw(rp,x2,y2);
          return 1;
        }
    }
  else
    { if (vertical)
	{ int xm = x + width/2 -1;
	  int x2 = x + width-1;
	  int y2 = y + height-1;
	  int num;

	  if (RangeRand(2))
	    { /* ziehe eine Vertikale Linie */
	      int yd1 = y + height/7;
	      int yd2 = y + (height*6)/7-1;
#ifdef DEBUG 
              if (xm<0 || yd1<0 || xm>thewidth || yd1>theheight)
	        { printf("error! xm,yd1 = %d,%d\n",xm,yd1);
          breakme();
                  while(1) { printf(" "); }
                }
              if (xm+1<0 || xm+1>thewidth || yd2<0 || yd2>theheight)
                { printf("error! xm+1,yd = %d,%d\n",xm+1,yd2);
          breakme();
                  while(1) { printf(" "); }
                }
#endif
	      SetAPen(rp,BLACK_PEN);
	      Move(rp,xm,yd1);
	      Draw(rp,xm,yd2);
	      SetAPen(rp,WHITE_PEN);
	      Move(rp,xm+1,yd1);
	      Draw(rp,xm+1,yd2);
	    }
	  num = generate_lamp_row(l,rp,limit, RangeRand(2),
				  x+1,y+1,width/2-4,height-2,rhythms,r);
	  num = num + generate_lamp_row(l+num,rp,limit-num,RangeRand(2),
					x+width/2+2,y+1,width/2-4,height-2,rhythms,r);
	  return num;
	}
      else
	{ int ym = y + height/2 -1;
	  int x2 = x + width-1;
	  int y2 = y + height-1;
	  int num;

	  if (RangeRand(2))
	    { /* ziehe eine horizontale Linie */
	      int xd1 = x + width/7;
	      int xd2 = x + (width*6)/7-1;
#ifdef DEBUG 
              if (ym<0 || xd1<0 || ym>theheight || xd1>thewidth)
	        { printf("error! xd1,ym = %d,%d\n",xd1,ym);
          breakme();
                  while(1) { printf(" "); }
                }
              if (xd2<0 || xd2>thewidth || ym+1<0 || ym+1>theheight)
                { printf("error! xd2,ym = %d,%d\n",xd2,ym+1);
          breakme();
                  while(1) { printf(" "); }
                }
#endif
	      SetAPen(rp,BLACK_PEN);
	      Move(rp,xd1,ym);
	      Draw(rp,xd2,ym);
	      SetAPen(rp,WHITE_PEN);
	      Move(rp,xd1,ym+1);
	      Draw(rp,xd2,ym+1);
	    }
	  num = generate_lamp_row(l,rp,limit, RangeRand(2),
				  x+1,y+1,width-2,height/2-4,rhythms,r);
	  num = num + generate_lamp_row(l+num,rp,limit-num,RangeRand(2),
					x+1,y+height/2+2,width-2,height/2-4,rhythms,r);
	  return num;
	}
    }
}
	   


LONG Trek( struct Screen *scr, SHORT width, SHORT height, PrefObject *Prefs)
{
  LONG flg_end        = OK;
  int colors          = 1L << scr->BitMap.Depth;
  int rhythms         = Prefs[RHYTHMS].po_Level;
  int duration        = Prefs[DURATION].po_Level;
  struct RastPort *rp = &( scr->RastPort );

  rhythm *slave, *curr_rhythm;
  lamp   *show, *curr_lamp;
  rhentry *curr_entry;

  int i,j,lamps,thedelay;

  MINWIDTH = Prefs[WIDTH].po_Level;
  MINHEIGHT = Prefs[HEIGHT].po_Level;
  submode = Prefs[SUBMODE].po_Level;
  freemode = Prefs[FREEMODE].po_Level;
  thedelay = Prefs[DELAY].po_Level;

#ifdef DEBUG
  thewidth = width;
  theheight = height;
#endif

  /* Initialize data */
  slave = malloc(rhythms * sizeof(rhythm));
  show  = malloc(MAXLAMP * sizeof(lamp));

  while (flg_end == OK)
    { generate_rhythms(slave,rhythms,colors);
      SetRast( rp, 0 );
      ScreenToFront( scr );
      lamps = generate_lamp_row(show,rp,MAXLAMP,RangeRand(2),
				0,0,width,height,rhythms,slave);

      /* Display the current sequence */
      for (i=0; i<duration && flg_end == OK; i++)
	{ curr_lamp = show;

          /* simulate one clock cycle of each lamp */
	  for (j=0; j<lamps; j++)
	    { curr_rhythm = curr_lamp->rh;
	      curr_entry  = curr_rhythm->data+(curr_lamp->stage);
	      (curr_lamp->count)++;
	      if (curr_lamp->count >= curr_entry->delay)
		{ /* Lamp reaches a new stage */
		  curr_lamp->stage++;
		  curr_lamp->count = 0;
		  curr_entry++;
		  if (curr_lamp->stage >= curr_rhythm->length)
		    { /* all stages done, start with first stage */
		      curr_lamp->stage = 0;
		      curr_entry = curr_rhythm->data;
		    }
		  /* paint our lamp */
		  SetAPen(rp,curr_entry->color);
#ifdef DEBUG
                  if (curr_lamp->x < 0 || curr_lamp->y < 0 ||
                      curr_lamp->width > width || curr_lamp->height > height)
                    { printf("error! x,y = %d,%d, w,h = %d, %d\n",curr_lamp->x, curr_lamp->y,
                             curr_lamp->width, curr_lamp->height);
          breakme();
                      while(1) { printf(" "); }
                    }
#endif
		  RectFill(rp,curr_lamp->x,curr_lamp->y,
			      curr_lamp->x+curr_lamp->width-1,
			      curr_lamp->y+curr_lamp->height-1);
		}
              curr_lamp++;
	    } /* for j */
	  ScreenToFront(scr);
	  flg_end = ContinueBlanking();
          for (j=0; j<thedelay && flg_end == OK; j++) 
            { Delay(1);
	      ScreenToFront(scr);
  	      flg_end = ContinueBlanking();
            }
	} /* for i */
    } /* while */
  free(slave);
  free(show);
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
                if (Prefs[MODE].po_Depth > 1)
                  {
                    if (Prefs[GREYBG].po_Level)
	    	      { SetRGB4(&( Scr->ViewPort ), 2, 0, 0, 0 );
  		        SetRGB4(&( Scr->ViewPort ), 1, 15,15,15 );
		        SetRGB4(&( Scr->ViewPort ), 0, 8,8,8);
                      } 
                    else
                      { SetRGB4(&( Scr->ViewPort ), 1, 15,15,15 );
		        SetRGB4(&( Scr->ViewPort ), 2, 8,8,8);
                      }
                  }
		Wnd = BlankMousePointer( Scr );
		
		do
		  RetVal = Trek( Scr, Scr->Width, Scr->Height, Prefs );
		while( RetVal == OK );
		
		UnblankMousePointer( Wnd );
		RainbowPalette( 0L, ColorTable, 1L, 0L );
		CloseScreen( Scr );
	}
	else
		RetVal = FAILED;
	
	return RetVal;
}
