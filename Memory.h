#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#include "Types.h"

/*-------------------------------------------------------------------*/
/*  Memory Resource                                                  */
/*-------------------------------------------------------------------*/

extern BYTE ROM[ 0x2000 ];

/*-------------------------------------------------------------------*/
/*  Function Prototype                                               */
/*-------------------------------------------------------------------*/

BYTE M6809_Read( WORD wAddr );
WORD M6809_ReadW( WORD wAddr );
void M6809_Write( WORD wAddr, BYTE byData );
void M6809_WriteW( WORD wAddr, WORD wData );

#endif /* MEMORY_H_INCLUDED */
