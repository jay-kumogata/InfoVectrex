#include <stdio.h>
#include "M6809.h"
#include "M6522.h"
#include "VG.h"
#include "System.h"
#include "SG.h"

/*-------------------------------------------------------------------*/
/*  SG Resources                                                     */
/*-------------------------------------------------------------------*/

/* SG Internal Registers */
BYTE SG_R[ 0x10 ];
BYTE SG_SEL;

/*---------------------------------------------------------------*/
/*  Update sound generator                                       */
/*---------------------------------------------------------------*/
void SG_Reset( void )
{
  /* Reset Registers */
  SG_R[ 0xe ] = 0xff;
}

/*---------------------------------------------------------------*/
/*  Update sound generator                                       */
/*---------------------------------------------------------------*/
void SG_Update( void )
{ 
  switch ( V_ORB & ( V_BDIR | V_BC1 ) ) {
  case 0x00:
    /* sound chip is disabled */
    break;
  case V_BC1:
    /* sound chip is sending data */
    break;
  case V_BDIR:
    /* sound chip is recieving data */
    if ( SG_SEL != 0xe ) {
      SG_R[ SG_SEL ] = V_ORA;
    }
    break;
  case V_BC1 | V_BDIR:
    /* sound chip is latching an address */
    if ( !( V_ORA & 0xf0 ) ) {
      SG_SEL = V_ORA & 0x0f;
    }
    break;
  }
#ifdef DEBUG
  //  if ( nFrame >= nFrameDebug )
  if ( bDebug )
    printf("SG SEL=%d #14=%d n",SG_SEL,SG_R[14]);
#endif

}
