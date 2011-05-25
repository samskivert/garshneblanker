/*                 Copyright (c) 1995 Michael D. Bayne.                 */
/*                         All rights reserved.                         */
/* See accompanying documentation for distribution and disclaimer info. */

#include <exec/memory.h>
#include <proto/exec.h>
#include <clib/alib_protos.h>
#include <stdlib.h>

#include "resource.h"

struct List Resources;

void resourceClassDestruct( int value )
{
	PvtResource *aRes;

	while( aRes = ( PvtResource * )RemHead( &Resources ))
	{
		aRes->rs_Destructor( aRes->rs_Resource );
		FreeMem( aRes, sizeof( PvtResource ));
	}
	
	exit( value );
}

void resourceClassInit( void )
{
	NewList( &Resources );
}

int resourceDestroy( void *theResource )
{
	PvtResource *aRes;

	if( IsListEmpty( &Resources ))
		return FALSE;

	for( aRes = resourceHead( &Resources );; aRes = resourceSucc( aRes ))
	{
		if( aRes->rs_Resource == theResource )
		{
			Remove(( struct Node * )aRes );
			aRes->rs_Destructor( aRes->rs_Resource );
			FreeMem( aRes, sizeof( PvtResource ));
			return TRUE;
		}
		if( aRes == resourceTail( &Resources ))
			break;
	}

	return FALSE;
}

void resourceAdd( void *Resource, void (*Destructor)( void * ))
{
	PvtResource *theRes;
	
	if( theRes = AllocMem( sizeof( PvtResource ), MEMF_CLEAR|MEMF_PUBLIC ))
	{
		theRes->rs_Resource = Resource;
		theRes->rs_Destructor = Destructor;
		AddHead( &Resources, ( struct Node * )theRes );
	}
	else
		resourceClassDestruct( 2 );
}
