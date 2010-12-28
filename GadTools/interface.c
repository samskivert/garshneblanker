/*
 *  Copyright (c) 1994 Michael D. Bayne.
 *  All rights reserved.
 *
 *  Please see the documentation accompanying the distribution for distribution
 *  and disclaimer information.
 */

#include <exec/memory.h>
#include <dos/dostags.h>
#include <intuition/gadgetclass.h>
#include <graphics/rpattr.h>
#include <graphics/text.h>
#include <libraries/gadtools.h>
#include <string.h>
#include <stdlib.h>

#include <clib/gadtools_protos.h>
#include <pragmas/gadtools_pragmas.h>

#include "Garshneblanker.h"

#include "includes.h"
#include "libraries.h"
#include "protos/protos.h"

struct Library *GadtoolsBase;

__saveds __asm LONG RenderHookFunc( register __a0 struct Hook *hook,
    register __a2 struct Node *Node, register __a1 struct LVDrawMsg *Msg )
{
    struct RastPort *Rast = Msg->lvdm_RastPort;

    switch( Msg->lvdm_MethodID )
    {
    case LV_DRAW:
    {
        BlankerEntry *Entry = ( BlankerEntry * )Node;

        if( Msg->lvdm_State == LVR_SELECTED )
        {
            LONG OldPen = GetAPen( Rast );
            SetAPen( Rast, ( 1L << Rast->BitMap->Depth ) - 1 );
            RectFill( Rast, Msg->lvdm_Bounds.MinX,
                Msg->lvdm_Bounds.MinY, Msg->lvdm_Bounds.MaxX,
                Msg->lvdm_Bounds.MaxY );
            SetAPen( Rast, OldPen );
        }
        else
            EraseRect( Rast, Msg->lvdm_Bounds.MinX,
                Msg->lvdm_Bounds.MinY, Msg->lvdm_Bounds.MaxX,
                Msg->lvdm_Bounds.MaxY );
        Move( Rast, Msg->lvdm_Bounds.MinX + 2,
            Msg->lvdm_Bounds.MinY + Rast->TxBaseline );
        if( Entry->be_Disabled )
            SetSoftStyle( Rast, FS_NORMAL, AskSoftStyle( Rast ));
        else
            SetSoftStyle( Rast, FSF_BOLD, AskSoftStyle( Rast ));
        Text( Rast, Entry->be_Name, strlen( Entry->be_Name ));
        SetSoftStyle( Rast, FS_NORMAL, AskSoftStyle( Rast ));
        return LVCB_OK;
    }
    default:
        return LVCB_UNKNOWN;
    }
}
struct Hook RenderHook = {{ 0L, 0L }, ( APTR )RenderHookFunc, 0L, 0L };

STRPTR NameSansParens( STRPTR Name )
{
    BYTE Buf[64];

    if( Name[0] == '{' )
    {
        strcpy( Buf, Name + 1 );
        Buf[strlen( Buf ) - 1] = '\0';
    }
    else
        return Name;

    return Buf;
}

STRPTR NameForEntry( struct List *Entries, LONG Entry )
{
    struct Node *Tmp;

    for( Tmp = Entries->lh_Head; Entry && Tmp; Tmp = Tmp->ln_Succ, --Entry );

    return Tmp ? Tmp->ln_Name : 0L;
}

int BT_PREFSClicked( VOID )
{
    if( Stricmp( Prefs->bp_Blanker, "Random" ))
        ExecSubProc( "PrefInterp", "" );

    return OK;
}

int BT_INFOClicked( VOID )
{
    ExecSubProc( "ShowInfo", ".txt" );

    return OK;
}

int BT_HIDEClicked( VOID )
{
    return CLOSEWIN;
}

int BT_QUITClicked( VOID )
{
    return QUIT;
}

int LV_BLANKERSClicked( VOID )
{
    LONG Rand;
    
    MessageModule( "GarshneClient", BM_DOQUIT );
    MessageModule( "GarshnePrefs", BM_DOQUIT );
    strcpy( Prefs->bp_Blanker,
           NameForEntry( BlankerEntries, BlankerMsg.Code ));
    Rand = !Stricmp( Prefs->bp_Blanker, "Random" );
    GT_SetGadgetAttrs( BlankerGadgets[GD_BT_PREFS], BlankerWnd, 0L,
                      GA_Disabled, Rand, TAG_DONE );
    GT_SetGadgetAttrs( BlankerGadgets[GD_BT_TOGGLE], BlankerWnd, 0L,
                      GA_Disabled, Rand, TAG_DONE );
    if( !Rand )
        LoadModule( Prefs->bp_Dir, Prefs->bp_Blanker );
    
    BlankerToEnv( Prefs );

    return OK;
}

int BT_TOGGLEClicked( VOID )
{
    BlankerEntry *Entry;
    BYTE Path[108];

    Entry = ( BlankerEntry * )FindName( BlankerEntries, Prefs->bp_Blanker );
    strcpy( Path, Prefs->bp_Dir );
    AddPart( Path, Prefs->bp_Blanker, 108 );
    strcat( Path, ".disabled" );
    ToggleModuleDisabled( Prefs );
    Entry->be_Disabled = !Entry->be_Disabled;

    GT_SetGadgetAttrs( BlankerGadgets[GD_LV_BLANKERS], BlankerWnd, 0L,
        GTLV_Labels, ~0, TAG_DONE );
    GT_SetGadgetAttrs( BlankerGadgets[GD_LV_BLANKERS], BlankerWnd, 0L,
        GTLV_Labels, BlankerEntries, TAG_DONE );

    return OK;
}

int BlankerCloseWindow( VOID )
{
    return CLOSEWIN;
}

int BlankerVanillaKey( VOID )
{
    switch( BlankerMsg.Code )
    {
    case 'p':
        return BT_PREFSClicked();
    case 'i':
        return BT_INFOClicked();
    case 'h':
        return CLOSEWIN;
    case 't':
        return BT_TOGGLEClicked();
    case 'q':
        return QUIT;
    default:
        return OK;
    }
}

VOID CloseInterface( VOID )
{
    if( BlankerWnd )
    {
        WinBox.Left = BlankerWnd->LeftEdge;
        WinBox.Top = BlankerWnd->TopEdge;
        CloseBlankerWindow();
        CloseDownScreen();
    }

    if( GadtoolsBase )
    {
        CloseLibrary( GadtoolsBase );
        GadtoolsBase = 0L;
    }
}

LONG OpenInterface( VOID )
{
    static BYTE Title[80];
    struct Node *TmpNode;
    LONG i;

    if( GadtoolsBase )
        return OK;

    if(!( GadtoolsBase = OpenLibrary( "gadtools.library", 37 )))
        return QUIT;

    if( !BlankerWnd )
    {
        if( SetupScreen())
            return 1L;
        strcpy( Title, "Garshneblanker ( PopKey=" );
        strcat( Title, Prefs->bp_PopKey );
        strcat( Title, ", BlankKey=" );
        strcat( Title, Prefs->bp_BlankKey );
        strcat( Title, " )" );
        BlankerLeft = WinBox.Left;
        BlankerTop = WinBox.Top;
        OpenBlankerWindow();
        SetWindowTitles( BlankerWnd, FilePart( ProgName ), Title );
    }
    else
    {
        ActivateWindow( BlankerWnd );
        WindowToFront( BlankerWnd );
    }
    
    if( BlankerWnd )
        ScreenToFront( BlankerWnd->WScreen );
    else
        return QUIT;
    
    GT_SetGadgetAttrs( BlankerGadgets[GD_LV_BLANKERS], BlankerWnd, 0L,
                      GTLV_Labels, BlankerEntries, TAG_DONE );

    for( i = 0, TmpNode = BlankerEntries->lh_Head;
        TmpNode && Stricmp( TmpNode->ln_Name, Prefs->bp_Blanker );
        TmpNode = TmpNode->ln_Succ, i++ );
    if( !TmpNode )
        i = 0;
    GT_SetGadgetAttrs( BlankerGadgets[GD_LV_BLANKERS], BlankerWnd, 0L,
                      GTLV_Selected, i, GTLV_MakeVisible, i, GTLV_Top, i,
                      TAG_DONE );
    
    i = !Stricmp( Prefs->bp_Blanker, "Random" );
    GT_SetGadgetAttrs( BlankerGadgets[GD_BT_PREFS], BlankerWnd, 0L,
                      GA_Disabled, i, TAG_DONE );
    GT_SetGadgetAttrs( BlankerGadgets[GD_BT_TOGGLE], BlankerWnd, 0L,
                      GA_Disabled, i, TAG_DONE );

    return OK;
}

LONG HandleInterface( VOID )
{
    if( !GadtoolsBase )
        return OK;
    
    return HandleBlankerIDCMP();
}

ULONG ISigs( VOID )
{
    return BlankerWnd ?( ULONG )( 1L << BlankerWnd->UserPort->mp_SigBit ):
        ( ULONG )0L;
}
