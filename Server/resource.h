#ifndef RESOURCES_H
#define RESOURCES_H

#define RS_ZEROARG  0
#define RS_ONEARG 1

#ifndef EXEC_LISTS_H
#include <exec/lists.h>
#endif

typedef struct _PvtResource
{
	struct Node rs_Node;
	VOID *rs_Resource;
	VOID (*rs_Destructor)( VOID * );
}
PvtResource;

#define resourceHead( l )\
	( IsListEmpty( l ) ? 0L : ( PvtResource * )(l)->lh_Head )
#define resourceTail( l )\
	( IsListEmpty( l ) ? 0L : ( PvtResource * )(l)->lh_TailPred )
#define resourceSucc( n ) (( PvtResource * )(n)->rs_Node.ln_Succ )
#define resourcePred( n ) (( PvtResource * )(n)->rs_Node.ln_Pred )

#include "protos/resource_protos.h"

#endif /* RESOURCES_H */
