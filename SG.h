#ifndef SG_H_INCLUDED
#define SG_H_INCLUDED

#include "Types.h"

/*-------------------------------------------------------------------*/
/*  Function Prototype                                               */
/*-------------------------------------------------------------------*/
void SG_Reset( void );
void SG_Update( void );

/*-------------------------------------------------------------------*/
/*  SG Resources                                                     */
/*-------------------------------------------------------------------*/

/* SG Internal Registers */
extern BYTE SG_R[ 0x10 ];
extern BYTE SG_SEL;

#endif /* !SG_H_INCLUDED */
