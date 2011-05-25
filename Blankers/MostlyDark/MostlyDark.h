#include <exec/types.h>
#include <exec/lists.h>


#define TXT(rp,c,x,y,s) { SetAPen(rp,c); Move(rp,x,y); Text(rp,s,strlen(s)); }


struct Blob {

    struct Node nod;

    LONG	x;
    LONG	y;

    WORD	c;
    UBYTE	inc_f;
};
typedef struct Blob Blob_t;

