/*
 *	Rival-Blanker (sould get a blanker)
 */

#include "/includes.h"
#include <stdlib.h>

#define farb(i) ((i)*(0x01000000)) /* for setting RGB32-Colors */

/* The BlackWin parameter decides, if Rival is a blanker or just a
   do-something :-)
   
   0 = Don't care
   1 = Only black moves
   2 = black will win quiet fast
   3 = black will win
   >3 black will win slowly
   and so on... */

#define SIZE 0
#define SPEED 2

void Defaults( PrefObject *Prefs )
{
	Prefs[SIZE].po_Level = 13;
	Prefs[SPEED].po_Active = 4;
}

long Blank( PrefObject *Prefs )
{
	LONG i, Width, Height, x, y, color, xmin, xmax, ymin, ymax, dx, dy;
	LONG Colors, black, RetVal, Pixels, count = 0, BlackWin, BlockSize;
	ULONG Seconds, Micros;
	struct RastPort *RP;
	struct ViewPort *VP;
	struct Screen *Scr;
	
	BlockSize = Prefs[SIZE].po_Level;
	BlackWin = 11 - Prefs[SPEED].po_Active;

	if( Scr = cloneTopScreen( FALSE, FALSE ))
	{
		/* Get all Screen-Information */
		VP=&(Scr->ViewPort);
		RP=&(Scr->RastPort);
   		Colors = 1L << Scr->RastPort.BitMap->Depth;
		Width=Scr->Width;
		Height=Scr->Height;
		Pixels=Width*Height;
		
		/* Find the color that fits best to black
		   and change this color to real black */
		if (BlackWin>0) {
            black=FindColor(VP->ColorMap,farb(0),farb(0),farb(0),Colors-1);
            if (black<0) black=1;
            if (black>=Colors) black=1;
            SetRGB32(VP,black,farb(0),farb(0),farb(0));
		}
		
		CurrentTime(&Seconds,&Micros);
		srand(Seconds+Micros);
		
		do
		{
            count++;
            if ((BlackWin>0)&&(count%BlackWin==0))
			{
				/* Try to find black systematicly */
				/* (a Ping-Pong-Ball will do this for us) */
				dx=1-2*(rand()%2);
				dy=1-2*(rand()%2);
				x=rand()%Width;
				y=rand()%Height;
				
				/* Only search at 1/25th of the Screen
				   because we don`t want to wait long */
				for (i=0;i<(Pixels/25);i++) {
					color=ReadPixel(RP,x,y);
					if (color==black) break;
					x=x+dx;
					y=y+dy;
					if (x<=0) {
						x=0;
						dx=1;
					}
					if (x>=Width-1) {
						x=Width-1;
						dx=-1;
					}
					if (y<=0) {
						y=0;
						dy=1;
						
					}
					if (y>=Height-1) {
						y=Height-1;
						dy=-1;
					}
				}
				color=black;
            }
            else {
				x=rand()%Width;
				y=rand()%Height;
				color=ReadPixel(RP,x,y);
            }
            SetAPen(RP,color);
            xmin=x-BlockSize;
            ymin=y-BlockSize;
            if (xmin<0) xmin=0;
            if (ymin<0) ymin=0;
            xmax=x+BlockSize;
            ymax=y+BlockSize;
            if (xmax>=Width) xmax=Width-1;
            if (ymax>=Height) ymax=Height-1;
			
            /* Draw the square */
            RectFill(RP,xmin,ymin,xmax,ymax);
			
			RetVal = ContinueBlanking();
		}
		while( RetVal == OK );
	}
	else
		RetVal = FAILED;
	
   	if( Scr ) CloseScreen( Scr );
	
	return RetVal;
}
