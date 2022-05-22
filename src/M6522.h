#ifndef M6522_H_INCLUDED
#define M6522_H_INCLUDED

#include "Types.h"

/*-------------------------------------------------------------------*/
/*  Function Prototype                                               */
/*-------------------------------------------------------------------*/
void M6522_Reset( void );
void M6522_Step( WORD wClocks );

/*-------------------------------------------------------------------*/
/*  VIA Resources                                                    */
/*-------------------------------------------------------------------*/

/* VIA Internal Registers */
extern BYTE V_R[ 0x16 ];

/*-------------------------------------------------------------------*/
/*  Macros                                                           */
/*-------------------------------------------------------------------*/

#define V_SET(a,b)      a |= (b)
#define V_RST(a,b)      a &= ~(b)
#define V_INV(a,b)      a ^= (b)

/* VIA Internal Registers */
#define V_PB            V_R[ 0x00 ]
#define V_PA            V_R[ 0x01 ]
#define V_DDRB          V_R[ 0x02 ]
#define V_DDRA          V_R[ 0x03 ]
#define V_T1C_L         V_R[ 0x04 ]
#define V_T1C_H         V_R[ 0x05 ]
#define V_T1L_L         V_R[ 0x06 ]
#define V_T1L_H         V_R[ 0x07 ]
#define V_T2C_L         V_R[ 0x08 ]
#define V_T2C_H         V_R[ 0x09 ]
#define V_SR            V_R[ 0x0A ]
#define V_ACR           V_R[ 0x0B ]
#define V_PCR           V_R[ 0x0C ]
#define V_IFR           V_R[ 0x0D ]
#define V_IER           V_R[ 0x0E ]
#define V_PA_NH         V_R[ 0x0F ]
#define V_T2L_L         V_R[ 0x10 ]
#define V_ORB           V_R[ 0x11 ]
#define V_INB           V_R[ 0x12 ]
#define V_ORA           V_R[ 0x13 ]
#define V_INA           V_R[ 0x14 ]
#define V_SR_CNT        V_R[ 0x15 ]

/* PB: VIA port B */
#define V_SWITCH        0x01
#define V_SEL           0x06
#define V_BC1           0x08
#define V_BDIR          0x10
#define V_COMPARE       0x20
#define V_HIBANK        0x40
#define V__RAMP         0x80

/* ACR: VIA auxiliary control register */
#define V_PA_LATCH      0x01
#define V_PB_LATCH      0x02
#define V_SR_CNTL       0x1C
#define V_T2_CNTL       0x20
#define V_T1_CNTL       0x40
#define V_T1_PB7_OUT    0x80

/* PCR: VIA control register */
#define V_IO7           0x01
#define V__ZERO         0x0E
#define V__ZERO_L       0x0C
#define V__ZERO_H       0x0E
#define V_NC            0x10
#define V__BLANK        0xE0
#define V__BLANK_L      0xC0
#define V__BLANK_H      0xE0

#define V_CA1           0x01
#define V_CB1           0x10

/* IFR: VIA interrupt flags register */
#define V_CA2_INT_F     0x01
#define V_CA1_INT_F     0x02
#define V_SR_INT_F      0x04
#define V_CB2_INT_F     0x08
#define V_CB1_INT_F     0x10
#define V_T2_INT_F      0x20
#define V_T1_INT_F      0x40
#define V_IRQ_ST_F      0x80

/* IER: VIA interrupt enable register */
#define V_CA2_INT_EN    0x01
#define V_CA1_INT_EN    0x02
#define V_SR_INT_EN     0x04
#define V_CB2_INT_EN    0x08
#define V_CB1_INT_EN    0x10
#define V_T2_INT_EN     0x20
#define V_T1_INT_EN     0x40
#define V_IER_CNTL_EN   0x80

#endif /* !M6522_H_INCLUDED */
