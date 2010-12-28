#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <string.h>

extern struct Library *SysBase;

LONG main( LONG argc, STRPTR argv[] )
{
    struct Library *DOSBase, *IntuitionBase;

    if( argc != 2 )
        return RETURN_WARN;

    DOSBase = OpenLibrary( "dos.library", 37L );
    IntuitionBase = OpenLibrary( "intuition.library", 37L );

    if( DOSBase && IntuitionBase )
    {
        struct EasyStruct InfoReq =
            { sizeof( struct EasyStruct ), 0, 0L, 0L, "Ok" };
        BYTE ModuleName[64], Title[108];
        STRPTR Ptr;

        strcpy( ModuleName, FilePart( argv[1] ));
        if( Ptr = strstr( ModuleName, ".txt" ))
            *Ptr = '\0';
        strcpy( Title, ModuleName );
        strcat( Title, " Information" );

        if( InfoReq.es_TextFormat = AllocVec( 1024, MEMF_CLEAR ))
        {
            BPTR InfoHandle;

            if( InfoHandle = Open( argv[1], MODE_OLDFILE ))
            {
                LONG BytesRead;

                BytesRead = Read( InfoHandle, InfoReq.es_TextFormat, 1024 );
                if( InfoReq.es_TextFormat[BytesRead-1] == '\n' )
                    InfoReq.es_TextFormat[BytesRead-1] = '\0';
                else
                    InfoReq.es_TextFormat[BytesRead] = '\0';
                Close( InfoHandle );
            }
            else
            {
                strcpy( InfoReq.es_TextFormat,
                    "No information available for module: " );
                strcat( InfoReq.es_TextFormat, ModuleName );
                strcat( InfoReq.es_TextFormat, "." );
            }

            InfoReq.es_Title = Title;
            EasyRequestArgs( 0L, &InfoReq, 0L, 0L );
            FreeVec( InfoReq.es_TextFormat );
        }
    }

    if( IntuitionBase )
        CloseLibrary( IntuitionBase );

    if( DOSBase )
        CloseLibrary( DOSBase );

    return RETURN_OK;
}
