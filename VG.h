#ifndef VG_H_INCLUDED
#define VG_H_INCLUDED

#include "Types.h"

/*-------------------------------------------------------------------*/
/*  Function Prototype                                               */
/*-------------------------------------------------------------------*/
void VG_Reset( void );
void VG_Refresh( void );
void VG_Step( WORD wClocks );
void VG_Update( void );

/*-------------------------------------------------------------------*/
/*  VG Resources                                                     */
/*-------------------------------------------------------------------*/

/* VG Internal Registers */
extern BYTE VG_X;
extern BYTE VG_Y;

/* Joystick */
extern BYTE VG_JCH0;    /* Joystick direction channel 0 */
extern BYTE VG_JCH1;    /* Joystick direction channel 1 */
extern BYTE VG_JCH2;    /* Joystick direction channel 2 */
extern BYTE VG_JCH3;    /* Joystick direction channel 3 */
extern BYTE VG_J;       /* Joystick sample and hold */ 
extern BYTE VG_COMPARE; /* Compare VG_J and VG_X */

/*-------------------------------------------------------------------*/
/*  VG Constants                                                     */
/*-------------------------------------------------------------------*/
#define VG_MAX_X     33000
#define VG_MAX_Y     41000

#endif /* !VG_H_INCLUDED */
