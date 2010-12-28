#include <exec/memory.h>
#include <dos/dostags.h>
#include <libraries/bgui.h>
#include <libraries/bgui_macros.h>
#include <string.h>

#include <clib/bgui_protos.h>
#include <pragmas/bgui_pragmas.h>

#include <clib/wb_protos.h>
#include <pragmas/wb_pragmas.h>

#include "interface.h"
#include "/includes.h"
#include "/libraries.h"
#include "/protos/protos.h"

Object *BlankersLvw, *PrefsBtn, *InfoBtn, *ToggleBtn, *HideBtn;
Object *SettingsBtn, *QuitBtn, *BlankWnd;
struct Window *BlankerWnd;
struct Library *BGUIBase = 0L;
ULONG BGUI_Sigs = 0L;
BYTE Title[128];

struct bguiRequest InfoReq = {
	BREQF_CENTERWINDOW|BREQF_LOCKWINDOW|BREQF_AUTO_ASPECT,
	"Garshneblanker", "_OK",
	"Current Tooltype Settings\n\n   BLANKKEY=%s\n   POPKEY=%s\n   TIMEOUT=%ld\n   REPLACE=%s\n   RANDTIMEOUT=%ld\n   BLANKCORNER=%s\n   DONTCORNER=%s",
	0, 0L, '_', 0, 0L, 0L
	};

__saveds __asm APTR ResourceHookFunc( register __a0 struct Hook *hook,
                                     register __a2 Object *lv_obj,
                                     register __a1 struct lvResource *lvr )
{
    switch( lvr->lvr_Command )
    {
    case LVRC_MAKE:
        return lvr->lvr_Entry;
    case LVRC_KILL:
        return 0L;
    }
}
struct Hook ResourceHook = {{ 0L, 0L }, ( APTR )ResourceHookFunc, 0L, 0L };

__saveds __asm STRPTR DisplayHookFunc( register __a0 struct Hook *hook,
                        register __a2 Object *lv_obj,
                        register __a1 struct lvRender *lvr )
{
    BlankerEntry *Entry = ( BlankerEntry * )lvr->lvr_Entry;
    static BYTE Buffer[80];
    
    if( !Entry->be_Disabled )
        return Entry->be_Name;

    strcpy( Buffer, "(" );
    strcat( Buffer, Entry->be_Name );
    strcat( Buffer, ")" );

    return Buffer;
}
struct Hook DisplayHook = {{ 0L, 0L }, ( APTR )DisplayHookFunc, 0L, 0L };

ULONG ISigs( VOID )
{
    return BGUI_Sigs;
}

LONG OpenInterface( VOID )
{
    LONG Rand;
    
    if( BGUIBase )
        return OK;

    if(!( BGUIBase = OpenLibrary( BGUINAME, BGUIVERSION )))
        return QUIT;

    Rand = !Stricmp( Prefs->bp_Blanker, "Random" );

    strcpy( Title, "Garshneblanker ( PopKey=" );
    strcat( Title, Prefs->bp_PopKey );
    strcat( Title, ", BlankKey=" );
    strcat( Title, Prefs->bp_BlankKey );
    strcat( Title, " )" );

	BlankWnd = WindowObject,
    WINDOW_Bounds, &WinBox,
    WINDOW_Title, FilePart( ProgName ),
    WINDOW_ScreenTitle, Title,
    WINDOW_SmartRefresh, TRUE,
    WINDOW_MasterGroup,
    
    VGroupObject,
    HOffset( WHITESPACE ), VOffset( WHITESPACE ), Spacing( 2 ),
    
    StartMember,
        BlankersLvw = ListviewObject,
            GA_ID, ID_BLANKERS,
            LISTV_DisplayHook, &DisplayHook,
            LISTV_ResourceHook, &ResourceHook,
        EndObject,
        Weight( 80 ),
    EndMember,
    
    StartMember,
        HGroupObject,
            Spacing( WHITESPACE ),
            StartMember, PrefsBtn = KeyButton( "_Prefs", ID_PREFS ), EndMember,
            StartMember, InfoBtn = KeyButton( "_Info", ID_INFO ), EndMember,
            StartMember, ToggleBtn = KeyButton( "_Toggle", ID_TOGGLE ), EndMember,
        EndObject,
        Weight( 10 ),
    EndMember,
    
    StartMember,
        HGroupObject,
        Spacing( WHITESPACE ),
            StartMember, HideBtn = KeyButton( "_Hide", ID_HIDE ),
            Weight( 30 ), EndMember,
            StartMember, SettingsBtn = KeyButton( "_Settings", ID_SET ),
            Weight( 40 ), EndMember,
            StartMember, QuitBtn = KeyButton( "_Quit", ID_QUIT ),
            Weight( 30 ), EndMember,
        EndObject,
        Weight( 10 ),
    EndMember,
    
    EndObject,
    
    EndObject;

    if( BlankWnd )
    {
        BlankerEntry *Tmp;
        LONG i, Selected;
        
        SetAttrs( PrefsBtn, GA_Disabled, Rand, TAG_END );
        SetAttrs( ToggleBtn, GA_Disabled, Rand, TAG_END );
        GadgetKey( BlankWnd, PrefsBtn, "p" );
        GadgetKey( BlankWnd, InfoBtn, "i" );
        GadgetKey( BlankWnd, ToggleBtn, "t" );
        GadgetKey( BlankWnd, HideBtn, "h" );
        GadgetKey( BlankWnd, SettingsBtn, "s" );
        GadgetKey( BlankWnd, QuitBtn, "q" );

        for( i = 0, Tmp = ( BlankerEntry * )BlankerEntries->lh_Head;
            Tmp->be_Node.ln_Succ;
            i++, Tmp = ( BlankerEntry * )Tmp->be_Node.ln_Succ )
        {
            if( !Stricmp( Tmp->be_Name, Prefs->bp_Blanker ))
                Selected = i;
            BGUI_DoGadgetMethod( BlankersLvw, BlankerWnd, 0L,
                                LVM_ADDSINGLE, 0L, Tmp, 0L, 0L );
        }

        DoMethod( BlankersLvw, BASE_ADDCONDITIONAL, PrefsBtn,
                 LISTV_EntryNumber, NumBlankEntries, GA_Disabled, TRUE,
                 GA_Disabled, FALSE );
        DoMethod( BlankersLvw, BASE_ADDCONDITIONAL, ToggleBtn,
                 LISTV_EntryNumber, NumBlankEntries, GA_Disabled, TRUE,
                 GA_Disabled, FALSE );

        SetAttrs( BlankersLvw, LISTV_Select, Selected, TAG_DONE );

        if( BlankerWnd = WindowOpen( BlankWnd ))
        {
            GetAttr( WINDOW_SigMask, BlankWnd, &BGUI_Sigs );
            ActivateWindow( BlankerWnd );
            SetAttrs( BlankersLvw, LISTV_Select, Selected, TAG_DONE );
            DoMethod( BlankersLvw, LVM_REFRESH, 0L );
            WindowToFront( BlankerWnd );
            ScreenToFront( BlankerWnd->WScreen );

            return OK;
        }
    }

    return QUIT;
}

VOID CloseInterface( VOID )
{
    GetAttr( WINDOW_Bounds, BlankWnd, ( ULONG * )&WinBox );
    WinBoxToEnv( &WinBox );
    WindowClose( BlankWnd );
    BlankerWnd = 0L;
    BGUI_Sigs = 0L;
    DisposeObject( BlankWnd );
    BlankWnd = 0L;
    CloseLibrary( BGUIBase );
    BGUIBase = 0L;
}

LONG HandleInterface( VOID )
{
    LONG RetVal = OK, rc;

    if( !BGUIBase )
        return OK;

    while (( rc = HandleEvent( BlankWnd )) != WMHI_NOMORE )
    {
        switch ( rc )
        {
        case ID_QUIT:
            RetVal = QUIT;
            break;
        case WMHI_CLOSEWINDOW:
        case ID_HIDE:
            RetVal = CLOSEWIN;
            break;
        case ID_SET:
            if( WBenchMsg &&( SysBase->lib_Version > 38 ))
            {
                struct Library *WorkbenchBase;
                
                if( WorkbenchBase = OpenLibrary( "workbench.library", 39L ))
                {
                    struct Screen *PubScr;
                    
                    if( PubScr = LockPubScreen( 0L ))
                    {
                        WBInfo( WBenchMsg->sm_ArgList->wa_Lock,
                               WBenchMsg->sm_ArgList->wa_Name, PubScr );
                        UnlockPubScreen( 0L, PubScr );
                        RetVal = RESTART;
                    }
                    CloseLibrary( WorkbenchBase );
                }
            }
			else
			{
				BGUI_Request( BlankerWnd, &InfoReq, Prefs->bp_BlankKey,
							 Prefs->bp_PopKey, Prefs->bp_Timeout / 10,
							 Prefs->bp_Flags & BF_REPLACE ? "YES" : "NO",
							 Prefs->bp_RandTimeout / 10,
							 CornerStrs[Prefs->bp_BlankCorner],
							 CornerStrs[Prefs->bp_DontCorner] );
			}
            break;
        case ID_TOGGLE:
        {
            BlankerEntry *Entry;

            Entry = ( BlankerEntry * )DoMethod( BlankersLvw, LVM_FIRSTENTRY,
                                               0L, LVGEF_SELECTED );
                        ToggleModuleDisabled( Prefs );
            Entry->be_Disabled = !Entry->be_Disabled;
            DoMethod( BlankersLvw, LVM_REFRESH, 0L );
            break;
        }
        case ID_PREFS:
            if( Stricmp( Prefs->bp_Blanker, "Random" ))
                ExecSubProc( "PrefInterp", "" );
            break;
        case ID_INFO:
            ExecSubProc( "ShowInfo", ".txt" );
            break;
        case ID_BLANKERS:
        {
            BlankerEntry *Entry;
    
            MessageModule( "GarshneClient", BM_DOQUIT );
            MessageModule( "GarshnePrefs", BM_DOQUIT );
            Entry = ( BlankerEntry * )DoMethod( BlankersLvw, LVM_FIRSTENTRY,
                                               0L, LVGEF_SELECTED );
            strcpy( Prefs->bp_Blanker, Entry->be_Name );
            if( Stricmp( Prefs->bp_Blanker, "Random" ))
                LoadModule( Prefs->bp_Dir, Prefs->bp_Blanker );
            BlankerToEnv( Prefs );
            break;
        }
        default:
            break;
        }
    }

    return RetVal;
}
