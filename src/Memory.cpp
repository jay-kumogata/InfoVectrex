#include <stdio.h>
#include "Memory.h"
#include "M6522.h"
#include "VG.h"
#include "System.h"
#include "SG.h"

/*-------------------------------------------------------------------*/
/*  Memory Resource                                                  */
/*-------------------------------------------------------------------*/

BYTE ROM[ 0x2000 ];
BYTE RAM[ 0x400 ];

/*===================================================================*/
/*                                                                   */
/*               M6809_Read() : Reading operation                    */
/*                                                                   */
/*===================================================================*/

BYTE M6809_Read( WORD wAddr )
{ 
  BYTE byData;

  switch ( wAddr & 0xE000 )
  {
  case 0xC000:
    switch ( wAddr & 0xF800 ) 
    {
    case 0xC800:   
      /* C800-CFFF: Vectrex RAM Space 1K, shadowed twice (r/w) */
      byData = RAM[ wAddr & 0x03FF ];
      break;
      
    case 0xD000:
      /* D000-D7FF: 6522VIA shadowed 128 times (r/w) */
      switch ( wAddr & 0x000F ) 
      { 
      case 0x0:
	/* VIA port B data I/O register */
	// byData = V_ORB & V_DDRB;
	byData = ( ( V_ORB & V_DDRB ) & ~V_COMPARE ) | VG_COMPARE;
	if ( V_ACR & V_PB_LATCH ) { byData |= V_PB & ~V_DDRB; }  /* Disabled */
	else                      { byData |= V_INB & ~V_DDRB; } /* Enabled */

	/* Update Input Register B */
	V_PB = byData;
	if ( !( V_PCR & V_CB1 ) ) { V_INB = V_PB & ~V_INB; }
	break;

      case 0x1:
      case 0xf:
	/* VIA port A data I/O register */
	if ( ( V_ORB & ( V_BC1 | V_BDIR ) ) == V_BC1 ) {
	  /* sound chip is driving port a */
	  byData = SG_R[ SG_SEL ];
	} else {
	  if ( V_ACR & V_PA_LATCH ) { byData = V_PA; }   /* Disabled */
	  else                      { byData = V_INA; }  /* Enabled */
	}

	/* Update Input Register A */
	V_PA = byData;
	if ( !( V_PCR & V_CA1 ) ) { V_INA = V_PA; }
	break;

      case 0x4:
	 /* VIA timer 1 count register lo (scale factor) */
	 byData = V_T1C_L;
	 V_RST( V_IFR, V_T1_INT_F );
	 break;

      case 0x5:
	 /* VIA timer 1 count register hi */
	 byData = V_T1C_H;
	 break;

      case 0x6:
	 /* VIA timer 1 latch register lo */
	 byData = V_T1L_L;
	 break;

      case 0x7:
        /* VIA timer 1 latch register hi */
	 byData = V_T1L_H;
	 break;

      case 0x8:
	 /* VIA timer 2 count/latch register lo (refresh) */
	 byData = V_T2C_L;
	 V_RST( V_IFR, V_T2_INT_F );
	 break;

      case 0x9:
	 /* VIA timer 2 count/latch register hi */	 
	 byData = V_T2C_H;
	 break;
	 
      case 0xA:
	 /* Shift register */
	 byData = V_SR;
	 V_SR_CNT = 0x08; V_SET( V_PCR, V_CB1 );
	 V_RST( V_IFR, V_SR_INT_F );
	 break;

      case 0xD:
	 /* Interrupt flag register */
	 V_IFR = ( V_IFR & V_IER & ~V_IRQ_ST_F ? V_IRQ_ST_F : 0x00 ) |
	         ( V_IFR & ~V_IRQ_ST_F );
	 byData = V_IFR;
	 break;

      case 0xE:
	 /* Interrupt enable register */
	 byData = V_IER_CNTL_EN | ( V_IER & ~V_IER_CNTL_EN );
	 break;

      default:
	 byData = V_R[ wAddr & 0x000F ];
	 break;
      }
      break;
    }
    break;

  case 0xE000:
    /* System ROM Space 8K (r) */
    byData = ROM[ wAddr & 0x1FFF ];
    break;
  }

#ifdef DEBUG
  //  if ( nFrame >= nFrameDebug )
  if ( bDebug )
    printf( "Read [%x] = %x\n", wAddr, byData );
#endif
  return byData;
}

/*===================================================================*/
/*                                                                   */
/*              M6809_Write() : Writing operation                    */
/*                                                                   */
/*===================================================================*/

void M6809_Write( WORD wAddr, BYTE byData ) 
{
#ifdef DEBUG
  //  if ( nFrame >= nFrameDebug )
  if ( bDebug )
    printf( "Write %x <- %x\n", wAddr, byData );
#endif 

  switch ( wAddr & 0xE000 )
  {
  case 0xC000:
    switch ( wAddr & 0xF800 ) 
    {
    case 0xC800:
      /* C800-CFFF: Vectrex RAM Space 1K, shadowed twice (r/w) */
      RAM[ wAddr & 0x03FF ] = byData;
      break;

    case 0xD000:
      /* D000-D7FF: 6522VIA shadowed 128 times (r/w) */
      switch ( wAddr & 0x000F )
      {
      case 0x0:
	/* VIA port B data I/O register */
       V_PB = byData;
	V_ORB = ( V_ORB & ~V_DDRB) | ( V_PB & V_DDRB );
	if ( !( V_PCR & V_CB1 ) ) { V_INB = V_PB & ~V_DDRB; }

	/* Update VG's and SG's Registers */
	SG_Update();
	VG_Update();
	break;

      case 0x1:
      case 0xf:
	/* VIA port A data I/O register */
	V_PA = byData;
	V_ORA = ( V_ORA & ~V_DDRA ) | ( V_PA & V_DDRA );
	if ( !( V_PCR & V_CA1 ) ) { V_INA = V_PA; }

	/* Update SG's Registers */
	SG_Update();

	/* VIA port A output feeds directly into DAC */
	VG_X = V_ORA ^ 0x80;
	VG_Update();
	break;

      case 0x2:
	/* Data Direction Register B */
	//	if ( !( V_ACR & V_T1_PB7_OUT ) ) { V_DDRB = byData & ~V__RAMP; } 
	//	else                             { V_DDRB = byData; }
	V_DDRB = byData;
	V_ORB = ( V_ORB & ~V_DDRB ) | ( V_PB & V_DDRB );
	break;

      case 0x3:
	/* Data Direction Register A */
	V_DDRA = byData;
	V_ORA = ( V_ORA & ~V_DDRA ) | ( V_PA & V_DDRA );
	break;

      case 0x4: 
	/* VIA timer 1 count register lo (scale factor) */
	V_T1L_L = byData;
	break;

      case 0x5:
	 /* VIA timer 1 count register hi */
	 V_T1L_H = V_T1C_H = byData;
	 V_T1C_L = V_T1L_L;
	 V_RST( V_IFR, V_T1_INT_F );

	 /* PB7 output */
	 if ( V_ACR & V_T1_PB7_OUT ) { V_RST( V_ORB, V__RAMP ); }
	 break;

      case 0x6:
	 /* VIA timer 1 latch register lo */
	 V_T1L_L = byData;
	 break;

      case 0x7:
        /* VIA timer 1 latch register hi */
	V_T1L_H = byData;
	break;

      case 0x8:
	 /* VIA timer 2 count/latch register lo (refresh) */
	 V_T2L_L = byData;
	 break;

      case 0x9:
	 /* VIA timer 2 count/latch register hi */	 
	 V_T2C_H = byData;
	 V_T2C_L = V_T2L_L;
	 V_RST( V_IFR, V_T2_INT_F );	 
	 break;

      case 0xA:
	 /* Shift register */
	 V_SR = byData;
	 V_SR_CNT = 0x08; V_SET( V_PCR, V_CB1 );
	 V_RST( V_IFR, V_SR_INT_F );
	 break;

      case 0xD:
	 /* Interrupt flag register */
	 V_IFR = byData & ~V_IRQ_ST_F;
	 break;


      case 0xE:
	 /* Interrupt enable register */
	 if ( byData & V_IER_CNTL_EN )
	 {
	   /* enable */
	   V_IER |= ( byData & ~V_IER_CNTL_EN );
	 } else {
	   /* disable */
	   V_IER &= ~( byData & ~V_IER_CNTL_EN );
	 }
	 break;
	
      default:
	 V_R[ wAddr & 0x000F ] = byData;
	 break;
      }
      break;
    }
    break;
  }
}

/* Reading/Writing operation (WORD version) */
WORD M6809_ReadW( WORD wAddr )
{ return (WORD)M6809_Read( wAddr ) << 8 | M6809_Read( wAddr + 1 ); }
void M6809_WriteW( WORD wAddr, WORD wData ) 
{ M6809_Write( wAddr, wData >> 8 ); M6809_Write( wAddr + 1, wData & 0xff ); }

/*

Memory Map

Memory map (hardware):
(taken from Keith Wilkins 'internal.txt')

 

0000-7fff

Cartridge ROM Space, the CART line is also gated with ~E to produce a signal that can be fed directly to the output enable of a ROM. This address range is not gated with the read-write line but this line is fed to the cartridge connector. (r/w) 


8000-C7FF 

Unmapped space.


C800-CFFF 

Vectrex RAM Space 1Kx8, shadowed twice. (r/w)


D000-D7FF 

6522VIA shadowed 128 times (r/w)


D800-DFFF 

Don't use this area. Both the 6522 and RAM are selected in any reads/writes to this area.


E000-FFFF 

System ROM Space 8Kx8 (r/w)


C800-C880 

is RAM used by the vectrex BIOS for housekeeping. (see appendix B) CBEA-CBFE is RAM used by the vectrex BIOS for housekeeping. (see appendix B)


C880-CBEA 

is RAM that can be used by the programmer!


E000-EFFF 

is ROM, the built in game MINE STORM.

*/ 

