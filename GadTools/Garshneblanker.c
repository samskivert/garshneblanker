/*
 *  Source machine generated by GadToolsBox V2.0b
 *  which is (c) Copyright 1991-1993 Jaba Development
 *
 *  GUI Designed by : Michael D. Bayne
 */

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/imageclass.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <graphics/displayinfo.h>
#include <graphics/gfxbase.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <string.h>
#include <clib/diskfont_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include "Garshneblanker.h"

struct Screen         *Scr = NULL;
UBYTE                 *PubScreenName = NULL;
APTR                   VisualInfo = NULL;
struct Window         *BlankerWnd = NULL;
struct Gadget         *BlankerGList = NULL;
struct IntuiMessage    BlankerMsg;
struct Gadget         *BlankerGadgets[6];
UWORD                  BlankerLeft = 0;
UWORD                  BlankerTop = 16;
UWORD                  BlankerWidth = 214;
UWORD                  BlankerHeight = 99;
UBYTE                 *BlankerWdt = (UBYTE *)"Garshneblanker";
struct TextAttr       *Font, Attr;
UWORD                  FontX, FontY;
UWORD                  OffX, OffY;
struct TextFont       *BlankerFont = NULL;

UWORD BlankerGTypes[] = {
	BUTTON_KIND,
	BUTTON_KIND,
	LISTVIEW_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND
};

struct NewGadget BlankerNGad[] = {
	159, 61, 51, 15, (UBYTE *)"_Hide", NULL, GD_BT_HIDE, PLACETEXT_IN, NULL, (APTR)BT_HIDEClicked,
	159, 80, 51, 15, (UBYTE *)"_Quit", NULL, GD_BT_QUIT, PLACETEXT_IN, NULL, (APTR)BT_QUITClicked,
	4, 4, 151, 90, NULL, NULL, GD_LV_BLANKERS, 0, NULL, (APTR)LV_BLANKERSClicked,
	159, 4, 51, 15, (UBYTE *)"_Prefs", NULL, GD_BT_PREFS, PLACETEXT_IN, NULL, (APTR)BT_PREFSClicked,
	159, 23, 51, 15, (UBYTE *)"_Info", NULL, GD_BT_INFO, PLACETEXT_IN, NULL, (APTR)BT_INFOClicked,
	159, 42, 51, 15, (UBYTE *)"_Toggle", NULL, GD_BT_TOGGLE, PLACETEXT_IN, NULL, (APTR)BT_TOGGLEClicked
};

extern struct Hook RenderHook;

ULONG BlankerGTags[] = {
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GTLV_ShowSelected), NULL, (GTLV_CallBack), &RenderHook, (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE)
};

static UWORD ComputeX( UWORD value )
{
	return(( UWORD )((( FontX * value ) + 3 ) / 6 ));
}

static UWORD ComputeY( UWORD value )
{
	return(( UWORD )((( FontY * value ) + 4 ) / 9 ));
}

static void ComputeFont( UWORD width, UWORD height )
{
	Forbid();
	Font = &Attr;
	Font->ta_Name = (STRPTR)GfxBase->DefaultFont->tf_Message.mn_Node.ln_Name;
	Font->ta_YSize = FontY = GfxBase->DefaultFont->tf_YSize;
	FontX = GfxBase->DefaultFont->tf_XSize;
	Permit();

	OffX = Scr->WBorLeft;
	OffY = Scr->RastPort.TxHeight + Scr->WBorTop + 1;

	if ( width && height ) {
		if (( ComputeX( width ) + OffX + Scr->WBorRight ) > Scr->Width )
			goto UseTopaz;
		if (( ComputeY( height ) + OffY + Scr->WBorBottom ) > Scr->Height )
			goto UseTopaz;
	}
	return;

UseTopaz:
	Font->ta_Name = (STRPTR)"topaz.font";
	FontX = FontY = Font->ta_YSize = 8;
}

int SetupScreen( void )
{
	if ( ! ( Scr = LockPubScreen( PubScreenName )))
		return( 1L );

	ComputeFont( 0, 0 );

	if ( ! ( VisualInfo = GetVisualInfo( Scr, TAG_DONE )))
		return( 2L );

	return( 0L );
}

void CloseDownScreen( void )
{
	if ( VisualInfo ) {
		FreeVisualInfo( VisualInfo );
		VisualInfo = NULL;
	}

	if ( Scr        ) {
		UnlockPubScreen( NULL, Scr );
		Scr = NULL;
	}
}

int HandleBlankerIDCMP( void )
{
	struct IntuiMessage	*m;
	int			(*func)();
	BOOL			running = TRUE;

	while( m = GT_GetIMsg( BlankerWnd->UserPort )) {

		CopyMem(( char * )m, ( char * )&BlankerMsg, (long)sizeof( struct IntuiMessage ));

		GT_ReplyIMsg( m );

		switch ( BlankerMsg.Class ) {

			case	IDCMP_REFRESHWINDOW:
				GT_BeginRefresh( BlankerWnd );
				GT_EndRefresh( BlankerWnd, TRUE );
				break;

			case	IDCMP_CLOSEWINDOW:
				running = BlankerCloseWindow();
				break;

			case	IDCMP_VANILLAKEY:
				running = BlankerVanillaKey();
				break;

			case	IDCMP_GADGETUP:
			case	IDCMP_GADGETDOWN:
				func = ( void * )(( struct Gadget * )BlankerMsg.IAddress )->UserData;
				running = func();
				break;

			case	IDCMP_MENUPICK:
				break;
		}
	}
	return( running );
}

int OpenBlankerWindow( void )
{
	struct NewGadget	ng;
	struct Gadget	*g;
	UWORD		lc, tc;
	UWORD		wleft = BlankerLeft, wtop = BlankerTop, ww, wh;

	ComputeFont( BlankerWidth, BlankerHeight );

	ww = ComputeX( BlankerWidth );
	wh = ComputeY( BlankerHeight );

	if (( wleft + ww + OffX + Scr->WBorRight ) > Scr->Width ) wleft = Scr->Width - ww;
	if (( wtop + wh + OffY + Scr->WBorBottom ) > Scr->Height ) wtop = Scr->Height - wh;

	if ( ! ( BlankerFont = OpenDiskFont( Font )))
		return( 5L );

	if ( ! ( g = CreateContext( &BlankerGList )))
		return( 1L );

	for( lc = 0, tc = 0; lc < Blanker_CNT; lc++ ) {

		CopyMem((char * )&BlankerNGad[ lc ], (char * )&ng, (long)sizeof( struct NewGadget ));

		ng.ng_VisualInfo = VisualInfo;
		ng.ng_TextAttr   = Font;
		ng.ng_LeftEdge   = OffX + ComputeX( ng.ng_LeftEdge );
		ng.ng_TopEdge    = OffY + ComputeY( ng.ng_TopEdge );
		ng.ng_Width      = ComputeX( ng.ng_Width );
		ng.ng_Height     = ComputeY( ng.ng_Height);

		BlankerGadgets[ lc ] = g = CreateGadgetA((ULONG)BlankerGTypes[ lc ], g, &ng, ( struct TagItem * )&BlankerGTags[ tc ] );

		while( BlankerGTags[ tc ] ) tc += 2;
		tc++;

		if ( NOT g )
			return( 2L );
	}

	if ( ! ( BlankerWnd = OpenWindowTags( NULL,
				WA_Left,	wleft,
				WA_Top,		wtop,
				WA_Width,	ww + OffX + Scr->WBorRight,
				WA_Height,	wh + OffY + Scr->WBorBottom,
				WA_IDCMP,	BUTTONIDCMP|LISTVIEWIDCMP|IDCMP_MENUPICK|IDCMP_CLOSEWINDOW|IDCMP_VANILLAKEY|IDCMP_REFRESHWINDOW,
				WA_Flags,	WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_CLOSEGADGET|WFLG_SIZEBBOTTOM|WFLG_SMART_REFRESH|WFLG_ACTIVATE|WFLG_RMBTRAP,
				WA_Gadgets,	BlankerGList,
				WA_Title,	BlankerWdt,
				WA_ScreenTitle,	"Garshneblanker",
				WA_PubScreen,	Scr,
				TAG_DONE )))
	return( 4L );

	GT_RefreshWindow( BlankerWnd, NULL );

	return( 0L );
}

void CloseBlankerWindow( void )
{
	if ( BlankerWnd        ) {
		CloseWindow( BlankerWnd );
		BlankerWnd = NULL;
	}

	if ( BlankerGList      ) {
		FreeGadgets( BlankerGList );
		BlankerGList = NULL;
	}

	if ( BlankerFont ) {
		CloseFont( BlankerFont );
		BlankerFont = NULL;
	}
}

