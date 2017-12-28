#include <stdio.h>
#include "M6522.h"
#include "System.h"

/*-------------------------------------------------------------------*/
/*  VIA Resources                                                    */
/*-------------------------------------------------------------------*/

/* VIA Internal Registers */
BYTE V_R[ 0x16 ];

/*===================================================================*/
/*                                                                   */
/*                M6522_Reset() : Reset 6522A VIA                    */
/*                                                                   */
/*===================================================================*/
void M6522_Reset( void )
{
/*
 *  Reset 6522A VIA
 *
 */

  /* Reset Internal Registers */
  for ( int i = 0; i < sizeof V_R; i++ ) {
    V_R[ i ] = 0x00;
  }

  /* Set Initial Values */
  V_SET( V_IFR, V_T1_INT_F );
  V_SET( V_IFR, V_T2_INT_F );
  //  V_SET( V_ORB, V__RAMP );
}

/*===================================================================*/
/*                                                                   */
/* M6522_Step() : Only the specified number of the clocks execute Op.*/
/*                                                                   */
/*===================================================================*/
void M6522_Step( WORD wClocks )
{
  for ( int i = 0; i < wClocks; i++ )
  {
    /*---------------------------------------------------------------*/
    /*  Timer #1                                                     */
    /*---------------------------------------------------------------*/
    if ( !( V_ACR & V_T1_CNTL ) )
    {
      /*-------------------------------------------------------------*/
      /*  Timer 1 One-Shot Mode                                      */
      /*-------------------------------------------------------------*/
      if ( !( V_IFR & V_T1_INT_F ) )
      {
	/* Completed? */
	if ( V_T1C_H == 0 && V_T1C_L == 0 )
	{
	  V_SET( V_IFR, V_T1_INT_F );

	  /* PB7 Output */
	  //	  if ( V_ACR & V_T1_PB7_OUT ) { V_SET( V_ORB, V__RAMP ); }
	}
	else
	{
	  /* Decrement T1 by system clock (O2) */
	  if ( V_T1C_L-- == 0 ) { V_T1C_H--; V_T1C_L = 0xFF; }
	}
      }
    } 
    else 
    {
      /*-------------------------------------------------------------*/
      /*  Timer 1 Free-Run Mode                                      */
      /*-------------------------------------------------------------*/

      /* Completed? */
      if ( V_T1C_H == 0 && V_T1C_L == 0 )
      {
	V_SET( V_IFR, V_T1_INT_F );
	V_T1C_H = V_T1L_H;
	V_T1C_L = V_T1L_L;
	
	/* PB7 Output */
	//	if ( V_ACR & V_T1_PB7_OUT ) { V_INV( V_ORB, V__RAMP ); }
      }
      else 
      {
	/* Decrement T1 by system clock (O2) */
	if ( V_T1C_L-- == 0 ) { V_T1C_H--; V_T1C_L = 0xFF; }
      }
    }   

    /*---------------------------------------------------------------*/
    /*  Timer #2                                                     */
    /*---------------------------------------------------------------*/
    if ( !( V_ACR & V_T2_CNTL ) )
    {
      /*-------------------------------------------------------------*/
      /*  Timer 2 One-Shot Mode                                      */
      /*-------------------------------------------------------------*/
      if ( V_T2C_H == 0 && V_T2C_L == 0 )
      {
	/* Completed? */
	V_SET( V_IFR, V_T2_INT_F );
      }
      else
      {
	/* Decrement by system clock (O2) */
	if ( !( V_IFR & V_T2_INT_F ) )
	{
	  if ( V_T2C_L-- == 0 ) { V_T2C_H--; V_T2C_L = 0xFF; }
	}
      }
    }
    else
    {
      /*-------------------------------------------------------------*/
      /*  Timer 2 Pulse Counting Mode                                */
      /*-------------------------------------------------------------*/
      printf("Error: Timer 2 Pulse Counting Mode not implemented.\n");
    }

#ifdef DEBUG
    //    if ( !( V_IFR & ( V_T1_INT_F | V_T2_INT_F ) ) )
    //    if ( nFrame >= nFrameDebug )
    if ( bDebug )
      printf("T1=%x T2=%x _RAMP=%d T1_INT_F=%d T2_INT_F=%d ACR=%x\n", 
	     V_T1C_H * 256 + V_T1C_L, 
	     V_T2C_H * 256 + V_T2C_L, 
	     V_ORB & V__RAMP ? 1 : 0,
	     V_IFR & V_T1_INT_F ? 1 : 0,
	     V_IFR & V_T2_INT_F ? 1 : 0,
	     V_ACR
	     );
#endif

    /*---------------------------------------------------------------*/
    /*  Shift Register                                               */
    /*---------------------------------------------------------------*/
    switch ( V_ACR & V_SR_CNTL ) 
    {
    case 0x00:
      /* Mode 0: Disabled */
      V_RST( V_IFR, V_SR_INT_F );
      V_SR_CNT = 0x00;
      break;

    case 0x04:
      /* Mode 2: Shift in Under O2 Control */
      if ( V_SR_CNT > 0 )
      {
        /* CB1 shift clock */
        V_INV( V_PCR, V_CB1 );
	
        /* SR is shifted out when CB1 is negative-going edge */
        if ( !( V_PCR & V_CB1 ) )
	 {
	   /* Output into CB2 ( ~BLANK ) */
	   V_RST( V_PCR, V__BLANK ); 
	   if ( V_SR & 0x01 ) { V_SET( V_PCR, V__BLANK_H ); }
	   else               { V_SET( V_PCR, V__BLANK_L ); }

	   /* Shifted in */
	   V_SR = ( V_SR & 0x01 ? 0x80 : 0x00 ) | ( V_SR >> 1 );

	   /* After 8 shifts, SR int. flag will be set */
	   if ( --V_SR_CNT == 0 ) { 
	     V_SET( V_IFR, V_SR_INT_F );
	   }
	 }
      }
      break;

    case 0x08:
      /**/
      printf("Error: not implemented.\n");
      break;

    case 0x0C:
      /**/
      printf("Error: not implemented.\n");
      break;

    case 0x10:
      /**/
      printf("Error: not implemented.\n");
      break;

    case 0x14:
      /**/
      printf("Error: not implemented.\n");
      break;

    case 0x18:
      /* Mode 6: Shift out Under O2 Control */
      if ( V_SR_CNT > 0 )
      {
        /* CB1 shift clock */
        V_INV( V_PCR, V_CB1 );
	
        /* SR is shifted out when CB1 is negative-going edge */
        if ( !( V_PCR & V_CB1 ) )
	 {
	   /* Output into CB2 ( ~BLANK ) */
	   V_RST( V_PCR, V__BLANK ); 
	   if ( V_SR & 0x80 ) { V_SET( V_PCR, V__BLANK_H ); }
	   else               { V_SET( V_PCR, V__BLANK_L ); }

	   /* Shifted out */
	   V_SR = ( V_SR << 1 ) | ( V_SR & 0x80 ? 0x01 : 0x00 );

	   /* After 8 shifts, SR int. flag will be set */
	   if ( --V_SR_CNT == 0 ) { 
	     V_SET( V_IFR, V_SR_INT_F );
	   }
	 }
      }
      break;

    case 0x1C:
      /**/
      printf("Error: not implemented.\n");
      break;
    }

#ifdef DEBUG
    //    if ( nFrame >= nFrameDebug )
    if ( bDebug )
      printf("SR=%x CNT=%d _BLANK=%x CB1=%x SRF=%d\n", 
	     V_SR,
	     V_SR_CNT,
	     V_PCR & V__BLANK,
	     V_PCR & V_CB1,
	     V_IFR & V_SR_INT_F ? 1 : 0
	     );
#endif


  }
}
