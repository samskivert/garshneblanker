#include <exec/memory.h>
#include <libraries/bgui_macros.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/bgui_protos.h>
#include <clib/asl_protos.h>
#include "/Libraries/Garshnelib/Garshnelib_protos.h"
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/bgui_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include "/Libraries/Garshnelib/Garshnelib_pragmas.h"

#include <ctype.h>
#include <string.h>

#include "PrefInterp.h"
#include "/protos/parse.h"
#include "/protos/var.h"
#include "/defs.h"

extern struct Library *DOSBase;

ULONG Map[] = { SLIDER_Level, INDIC_Level, TAG_END };
struct IBox WinBox, OldBox;
struct Library *IntuitionBase, *BGUIBase, *GarshnelibBase;
Object *ModuleObj, **Objects;
struct Window *ModuleWnd;
struct MsgPort *ReplyPort = 0L;
ULONG ModuleSigs = 0L;
LONG NumGadgets;
VOID *Memory = 0L;
PrefObject *Prefs;

STRPTR MakeBoxVarName( STRPTR ProgName, STRPTR IfcName )
{
	static BYTE FileName[108], *Ptr;

	strcpy( FileName, ProgName );
	strcat( FileName, "/" );
	strcat( FileName, IfcName );
	if( Ptr = strstr( FileName, ".ifc" ))
		*Ptr = '\0';
	strcat( FileName, ".win" );
	
	return FileName;
}

VOID FontRequest( struct TextAttr *Attr )
{
    struct Library *AslBase = OpenLibrary( "asl.library", 37L );
    struct FontRequester *fReq;

    if( !AslBase )
        return;
    
    fReq = AllocAslRequestTags( ASL_FontRequest,
                               ASL_FontName, Attr->ta_Name,
                               ASL_FontHeight, Attr->ta_YSize,
                               ASL_MaxHeight, 100, TAG_DONE );
    if( fReq )
    {
        if( AslRequestTags( fReq,
                           ASLFO_Window, ModuleWnd,
                           ASLFO_SleepWindow, TRUE,
                           ASLFO_TitleText, ( LONG )"Please choose a font...",
                           TAG_DONE ))
        {
            CopyMem( fReq->fo_Attr.ta_Name, Attr->ta_Name, 31 );
            Attr->ta_YSize = fReq->fo_Attr.ta_YSize;
        }
        FreeAslRequest( fReq );
    }

    CloseLibrary( AslBase );
}   

VOID SendMessageToPort( LONG Type, STRPTR PortName )
{
    struct MsgPort *ForeignPort;
    BlankMsg *ClientMsg;
    
    if( ForeignPort = FindPort( PortName ))
    {
        if( ClientMsg = AllocPooled( Memory, sizeof( BlankMsg )))
        {
            ClientMsg->bm_Mess.mn_ReplyPort = ReplyPort;
            ClientMsg->bm_Mess.mn_Length = sizeof( BlankMsg );
            ClientMsg->bm_Type = Type;
            PutMsg( ForeignPort, ( struct Message * )ClientMsg );
        }
    }
}

LONG main( LONG argc, STRPTR argv[] )
{
    Object *VertGroup, *SaveBtn, *TestBtn, *CancelBtn, *DisplayBtn, *CtrlGrp;
    Object *NameInf, *SizeInf;
    LONG i, j, LastNonGrp = 0, DispID = -1, Min, Max, *Types, rc, ID, Sigs;
    BYTE DescripName[108], PrefsName[108], BogusBuf[128], ValidPrefs = FALSE;
    STRPTR *Labels, *KeyStrs, BoxVarName;
    ULONG Args[] = { 0L, 0L };
    Object **Indics;
    BPTR Descrip, Tmp;

    if( argc != 2 )
        return RETURN_WARN;
    
    if( FindPort( "GarshnePrefs" ))
        return RETURN_WARN;

    IntuitionBase = OpenLibrary( "intuition.library", 37 );
    BGUIBase = OpenLibrary( BGUINAME, BGUIVERSION );
    GarshnelibBase = OpenLibrary( "Garshnelib.library", 37 );

    if( !IntuitionBase || !BGUIBase || !GarshnelibBase )
        goto JAIL;
    
    if(!( Memory = CreatePool( MEMF_CLEAR, 1024, 512 )))
        goto JAIL;

    if(!( ReplyPort = CreatePort( "GarshnePrefs", 0L )))
        goto JAIL;

    strcpy( DescripName, argv[1] );
    strcat( DescripName, ".ifc" );

    strcpy( PrefsName, argv[1] );
    strcat( PrefsName, ".prefs" );
    
    if(!( Descrip = Open( DescripName, MODE_OLDFILE )))
        goto JAIL;

    NumGadgets = ScanDigit( Descrip );

    Objects = AllocPooled( Memory, sizeof( Object * ) * ( NumGadgets + 1 ));
    Indics = AllocPooled( Memory, sizeof( Object * ) * ( NumGadgets + 1 ));
    Prefs = AllocPooled( Memory, sizeof( PrefObject ) * ( NumGadgets + 1 ));
    Types = AllocPooled( Memory, sizeof( LONG ) * ( NumGadgets + 1 ));
    KeyStrs = AllocPooled( Memory, sizeof( STRPTR ) * ( NumGadgets + 1 ));
    
    if( !Objects || !Indics || !Prefs || !Types || !KeyStrs )
        goto PREJAIL;

    VertGroup = BGUI_NewObject( BGUI_GROUP_GADGET,
                               GROUP_Style, GRSTYLE_VERTICAL,
                               HOffset( 3 ), VOffset( 3 ), Spacing( 3 ),
                               TAG_END );

	BoxVarName = MakeBoxVarName( FilePart( argv[0] ), FilePart( argv[1] ));

	if( GetVar37( BoxVarName, ( STRPTR )&WinBox, sizeof( struct IBox ),
				 GVF_BINARY_VAR|GVF_DONT_NULL_TERM ) != -1 )
	{
		OldBox = WinBox;
		ModuleObj = BGUI_NewObject( BGUI_WINDOW_OBJECT,
								   WINDOW_MasterGroup, VertGroup,
								   WINDOW_Title, FilePart( argv[1] ),
								   WINDOW_Bounds, &WinBox,
								   TAG_END );
	}
	else
	{
		ModuleObj = BGUI_NewObject( BGUI_WINDOW_OBJECT,
								   WINDOW_MasterGroup, VertGroup,
								   WINDOW_Title, FilePart( argv[1] ),
								   TAG_END );
	}
		
    if( Tmp = Open( PrefsName, MODE_OLDFILE ))
    {
        Read( Tmp, Prefs, sizeof( LONG )); /* Ignore entry count */
        if( Read( Tmp, Prefs, sizeof( PrefObject ) * NumGadgets ) ==
           sizeof( PrefObject ) * NumGadgets )
            ValidPrefs = TRUE;
        Close( Tmp );
    }

    for( i = 0; i < NumGadgets; i++ )
    {
        STRPTR IDStr, LabelStr;
        
        IDStr = ScanToken( Descrip );
        switch( tolower( IDStr[0] ))
        {
        case 'c':
            Types[i] = GAD_CYCLE;
            break;
        case 'd':
            Types[i] = ( tolower( IDStr[1] ) == 'i' )? GAD_DISPLAY : GAD_DELIM;
            break;
        case 'f':
            Types[i] = GAD_FONT;
            break;
        case 's':
            Types[i] = ( tolower( IDStr[1] ) == 'l' )? GAD_SLIDER : GAD_STRING;
            break;
        }
        
        switch( Prefs[i].po_Type = Types[i] )
        {
        case GAD_CYCLE:
            LabelStr = ScanToken( Descrip );
            KeyStrs[i] = ScanToken( Descrip );
            Labels = ScanTokenArray( Descrip );
            if( ValidPrefs )
                ScanDigit( Descrip );
            else
                Prefs[i].po_Active = ScanDigit( Descrip );
            Objects[i] =
                BGUI_NewObject( BGUI_CYCLE_GADGET,
                               GA_ID, i + 10,
                               LAB_Label, LabelStr,
                               LAB_Underscore, '_',
                               FRM_Type, FRTYPE_BUTTON,
                               CYC_Labels, Labels,
                               CYC_Active, Prefs[i].po_Active,
                               TAG_DONE );
            break;
        case GAD_SLIDER:
            LabelStr = ScanToken( Descrip );
            KeyStrs[i] = ScanToken( Descrip );
            Min = ScanDigit( Descrip );
            Max = ScanDigit( Descrip );
            if( ValidPrefs )
                ScanDigit( Descrip );
            else
                Prefs[i].po_Level = ScanDigit( Descrip );
            Objects[i] =
                BGUI_NewObject( BGUI_SLIDER_GADGET,
                               GA_ID, i + 10,
                               LAB_Label, LabelStr,
                               LAB_Underscore, '_',
                               SLIDER_Min, Min, 
                               SLIDER_Max, Max,
                               SLIDER_Level, Prefs[i].po_Level,
                               TAG_DONE );
            Indics[i] =
                BGUI_NewObject( BGUI_INDICATOR_GADGET,
                               INDIC_Min, Min,
                               INDIC_Max, Max,
                               INDIC_Level, Prefs[i].po_Level,
                               INDIC_Justification, IDJ_CENTER,
                               TAG_DONE );
            break;
        case GAD_FONT:
            if( ValidPrefs )
            {
                ScanToken( Descrip );
                ScanDigit( Descrip );
            }
            else
            {
                strcpy( Prefs[i].po_Name, ScanToken( Descrip ));
                Prefs[i].po_Attr.ta_YSize = ScanDigit( Descrip );
            }
            Prefs[i].po_Attr.ta_Name = Prefs[i].po_Name;
            Objects[i] =
                BGUI_NewObject( BGUI_BUTTON_GADGET,
                               GA_ID, i + 10,
                               LAB_Label, "_Font",
                               LAB_Underscore, '_',
                               FRM_Type, FRTYPE_BUTTON,
                               TAG_DONE );
            Args[0] = Prefs[i].po_Attr.ta_YSize,
            Indics[i] =
                BGUI_NewObject( BGUI_GROUP_GADGET, Spacing( 3 ),
                               StartMember,
                               NameInf =
                               BGUI_NewObject( BGUI_INFO_GADGET,
                                              FRM_Type, FRTYPE_BUTTON,
                                              FRM_Flags, FRF_RECESSED,
                                              INFO_HorizOffset, 6,
                                              INFO_VertOffset, 3,
                                              INFO_TextFormat,
                                              Prefs[i].po_Name,
                                              TAG_DONE ),
                               Weight( 80 ),
                               EndMember,
                               StartMember,
                               SizeInf = 
                               BGUI_NewObject( BGUI_INFO_GADGET,
                                              FRM_Type, FRTYPE_BUTTON,
                                              FRM_Flags, FRF_RECESSED,
                                              INFO_HorizOffset, 6,
                                              INFO_VertOffset, 3,
                                              INFO_TextFormat, "%ld",
                                              INFO_Args, Args,
                                              TAG_DONE ),
                               Weight( 20 ),
                               EndMember,
                               TAG_DONE );
            break;
        case GAD_STRING:
            LabelStr = ScanToken( Descrip );
            KeyStrs[i] = ScanToken( Descrip );
            if( ValidPrefs )
                FGets( Descrip, BogusBuf, 128 );
            else
            {
                FGets( Descrip, Prefs[i].po_Value, 128 );
                Prefs[i].po_Value[strlen( Prefs[i].po_Value )-1] = '\0';
            }
            Objects[i] =
                BGUI_NewObject( BGUI_STRING_GADGET,
                               GA_ID, i + 10,
                               LAB_Label, LabelStr,
                               LAB_Underscore, '_',
                               FRM_Type, FRTYPE_RIDGE,
                               STRINGA_TextVal, Prefs[i].po_Value,
                               STRINGA_MaxChars, 127,
                               TAG_DONE );
            break;
        case GAD_DISPLAY:
            DispID = i + 10;
            if( ValidPrefs )
                ScanDigit( Descrip );
            else
            {
                Prefs[i].po_ModeID = getTopScreenMode();
                Prefs[i].po_Depth = ScanDigit( Descrip );
            }
            break;
        case GAD_DELIM:
            if( Types[LastNonGrp] == GAD_SLIDER )
                Objects[i] = BGUI_NewObject( BGUI_GROUP_GADGET, Spacing( 3 ),
                                            StartMember, Objects[LastNonGrp],
                                            Weight( 90 ), EndMember,
                                            StartMember, Indics[LastNonGrp],
                                            Weight( 10 ), EndMember,
                                            TAG_DONE );
            else if( Types[LastNonGrp] == GAD_FONT )
                Objects[i] = BGUI_NewObject( BGUI_GROUP_GADGET, Spacing( 3 ),
                                            StartMember, Objects[LastNonGrp],
                                            Weight( 20 ), EndMember,
                                            StartMember, Indics[LastNonGrp],
                                            Weight( 80 ), EndMember,
                                            TAG_DONE );
            else
                Objects[i] = BGUI_NewObject( BGUI_GROUP_GADGET, Spacing( 3 ),
                                            StartMember, Objects[LastNonGrp],
                                            EndMember, TAG_DONE );
            for( j = LastNonGrp + 1; Objects[i] &&( j < i ); j++ )
            {
                if( Types[j] == GAD_SLIDER )
                {
                    DoMethod( Objects[i], GRM_ADDMEMBER, Objects[j],
                             LGO_Weight, 90, TAG_END );
                    DoMethod( Objects[i], GRM_ADDMEMBER, Indics[j],
                             LGO_Weight, 10, TAG_END );
                }
                else if( Types[j] == GAD_FONT )
                {
                    DoMethod( Objects[i], GRM_ADDMEMBER, Objects[j],
                             LGO_Weight, 20, TAG_END );
                    DoMethod( Objects[i], GRM_ADDMEMBER, Indics[j],
                             LGO_Weight, 80, TAG_END );
                }
                else
                    for( j = LastNonGrp + 1; j < i; j++ )
                        DoMethod( Objects[i], GRM_ADDMEMBER, Objects[j],
                                 TAG_END );
            }               
            LastNonGrp = i+1;
            DoMethod( VertGroup, GRM_ADDMEMBER, Objects[i], TAG_END );
            break;
        default:
            break;
        }
    }
    
    if( Descrip )
    {
        Close( Descrip );
        Descrip = 0L;
    }

    if( !ModuleObj )
        goto PREJAIL;

    if( DispID != -1 )
    {
        CtrlGrp = HGroupObject, Spacing( 3 ),
        StartMember, SaveBtn = KeyButton( "_Save", ID_SAVE ), EndMember,
        StartMember, TestBtn = KeyButton( "_Test", ID_TEST ), EndMember,
        StartMember, DisplayBtn = KeyButton( "_Display", DispID ), EndMember,
        StartMember, CancelBtn = KeyButton( "_Cancel", ID_CANCEL ), EndMember,
        EndObject;
    }
    else
    {
        CtrlGrp = HGroupObject, Spacing( 3 ),
        StartMember, SaveBtn = KeyButton( "_Save", ID_SAVE ), EndMember,
        StartMember, TestBtn = KeyButton( "_Test", ID_TEST ), EndMember,
        StartMember, CancelBtn = KeyButton( "_Cancel", ID_CANCEL ), EndMember,
        EndObject;
    }
    DoMethod( VertGroup, GRM_ADDMEMBER, CtrlGrp, TAG_END );

    GadgetKey( ModuleObj, SaveBtn, "s" );
    GadgetKey( ModuleObj, TestBtn, "t" );
    if( DispID != -1 )
        GadgetKey( ModuleObj, DisplayBtn, "d" );
    GadgetKey( ModuleObj, CancelBtn, "c" );
    
    for( i = 0; i < NumGadgets; i++ )
    {
        switch( Types[i] )
        {
        case GAD_SLIDER:
            AddMap( Objects[i], Indics[i], Map );
        case GAD_CYCLE:
        case GAD_STRING:
            GadgetKey( ModuleObj, Objects[i], KeyStrs[i] );
            break;
        case GAD_FONT:
            GadgetKey( ModuleObj, Objects[i], "f" );
            break;
        default:
            break;
        }
    }
    ModuleWnd = WindowOpen( ModuleObj );
    GetAttr( WINDOW_SigMask, ModuleObj, &ModuleSigs );

    while( ModuleObj && ModuleSigs )
    {
        Sigs = Wait( ModuleSigs |( 1L << ReplyPort->mp_SigBit )|
            SIGBREAKF_CTRL_C );
        
        if( Sigs & ( 1L << ReplyPort->mp_SigBit ))
        {
            BlankMsg *FreeMe;

            while( FreeMe = ( BlankMsg * )GetMsg( ReplyPort ))
            {
                switch( FreeMe->bm_Type )
                {
                case BM_DOQUIT:
                    FreeMe->bm_Flags |= BF_REPLY;
                    ReplyMsg(( struct Message * )FreeMe );
                    ModuleSigs = 0L;
                    break;
                case BM_RELOADPREFS:
                case BM_SENDTEST:
                    FreePooled( Memory, FreeMe, sizeof( BlankMsg ));
                    break;
                }
            }
        }

        if( Sigs & SIGBREAKF_CTRL_C )
        {
            ModuleSigs = 0L;
            break;
        }
        
        do
        {
            switch( rc = HandleEvent( ModuleObj ))
            {
            case ID_SAVE:
                if( Tmp = Open( PrefsName, MODE_NEWFILE ))
                {
                    Write( Tmp, &NumGadgets, sizeof( LONG ));
                    Write( Tmp, Prefs, sizeof( PrefObject ) * NumGadgets );
                    Close( Tmp );
                }
                SendMessageToPort( BM_RELOADPREFS, "GarshneClient" );
				GetAttr( WINDOW_Bounds, ModuleObj, ( ULONG * )&WinBox );
				if( memcmp( &OldBox, &WinBox, sizeof( struct IBox )))
					SetVar37( BoxVarName, ( STRPTR )&WinBox,
							 sizeof( struct IBox ), GVF_BINARY_VAR|
							 GVF_DONT_NULL_TERM|GVF_SAVE_VAR|GVF_GLOBAL_ONLY );
            case ID_CANCEL:
            case WMHI_CLOSEWINDOW:
                ModuleSigs = 0L;
                break;
            case ID_TEST:
                if( Tmp = Open( "T:GBlankerTmpPrefs", MODE_NEWFILE ))
                {
                    Write( Tmp, &NumGadgets, sizeof( LONG ));
                    Write( Tmp, Prefs, sizeof( PrefObject ) * NumGadgets );
                    Close( Tmp );
                    SendMessageToPort( BM_SENDTEST, "GarshneServer" );
                }
                break;
            default:
                ID = rc - 10;
                if( rc > 9 && ID < NumGadgets )
                {
                    switch( Types[ID] )
                    {
                    case GAD_CYCLE:
                        GetAttr( CYC_Active, Objects[ID],
                                ( ULONG * )&Prefs[ID].po_Active );
                        break;
                    case GAD_SLIDER:
                        GetAttr( SLIDER_Level, Objects[ID],
                                ( ULONG * )&Prefs[ID].po_Level );
                        break;
                    case GAD_STRING:
                    {
                        STRPTR TmpPtr;
                        
                        GetAttr( STRINGA_TextVal, Objects[ID],
                                ( ULONG * )&TmpPtr );
                        strcpy( Prefs[ID].po_Value, TmpPtr );
                        break;
                    }
                    case GAD_FONT:
                        FontRequest( &Prefs[ID].po_Attr );
                        SetAttrs( NameInf, INFO_TextFormat, Prefs[ID].po_Name,
                                 TAG_DONE );
                        Args[0] = Prefs[ID].po_Attr.ta_YSize;
                        SetAttrs( SizeInf, INFO_Args, Args, TAG_DONE );
                        break;
                    case GAD_DISPLAY:
                        ScreenModeRequest( ModuleWnd, &Prefs[ID].po_ModeID,
                                          Prefs[ID].po_Depth ?
                                          &Prefs[ID].po_Depth : 0L );
                        break;
                    }
                }
                break;
            }
        }
        while( rc != WMHI_NOMORE );
    }

    DisposeObject( ModuleObj );
    
 PREJAIL:
    if( Descrip )
        Close( Descrip );

 JAIL:
    if( ReplyPort )
        DeletePort( ReplyPort );
    if( Memory )
        DeletePool( Memory );
    if( GarshnelibBase )
        CloseLibrary( GarshnelibBase );
    if( BGUIBase )
        CloseLibrary( BGUIBase );
    if( IntuitionBase )
        CloseLibrary( IntuitionBase );

    return RETURN_OK;
}
