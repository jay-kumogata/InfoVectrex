#include <stdio.h>
#include "M6809.h"
#include "M6522.h"
#include "VG.h"
#include "System.h"

/*-------------------------------------------------------------------*/
/*  VG Resources                                                     */
/*-------------------------------------------------------------------*/

/* VG Internal Registers */
BYTE VG_X;       /* X sample and hold */
BYTE VG_Y;       /* Y sample and hold */
BYTE VG_Z;       /* Z sample and hold */
BYTE VG_R;       /* zero Ref sample and hold */

int VG_DX;       /* Delta X */
int VG_DY;       /* Delta Y */
int VG_CX;       /* Current axis X */
int VG_CY;       /* Current axis Y */

/* Joystick */
BYTE VG_JCH0;    /* Joystick direction channel 0 */
BYTE VG_JCH1;    /* Joystick direction channel 1 */
BYTE VG_JCH2;    /* Joystick direction channel 2 */
BYTE VG_JCH3;    /* Joystick direction channel 3 */
BYTE VG_J;       /* Joystick sample and hold */ 
BYTE VG_COMPARE; /* Compare VG_J and VG_X */

/*===================================================================*/
/*                                                                   */
/*                VG_Reset() : Reset Vector Generator                */
/*                                                                   */
/*===================================================================*/
void VG_Reset( void )
{
  /* Reset Registers */
  VG_JCH0 = VG_JCH1 = VG_JCH2 = VG_JCH3 = VG_J = 0x80;
  VG_COMPARE = 0x00;

  VG_CX = VG_MAX_X / 2;
  VG_CY = VG_MAX_Y / 2;
  VG_DX = VG_DY = 0;
}

/*-------------------------------------------------------------------*/
/*  Refresh Timing 50Hz                                              */
/*-------------------------------------------------------------------*/
void VG_Refresh( void )
{
  /* Update Screen */
  InfoVectrex_UpdateScreen();

  /* 50Hz / 1.5MHz = 30000 cycles */
  //  M6809_Step( 30000 );
  M6809_Step( 120000 );
}

/*===================================================================*/
/*                                                                   */
/* VG_Step() : Only the specified number of the clocks execute Op.   */
/*                                                                   */
/*===================================================================*/
void VG_Step( WORD wClocks )
{
  int DX, DY;

  for ( int i = 0; i < wClocks; i++ )
  {
    DX = DY = 0;

    if ( ( V_PCR & V__ZERO ) == V__ZERO_L ) {
      /* CA2 - ~ZERO */
#ifdef DEBUG
      //      if ( nFrame >= nFrameDebug )
      if ( bDebug )
	 printf("zero'd\n");
#endif
      DX = VG_MAX_X / 2 - VG_CX;
      DY = VG_MAX_Y / 2 - VG_CY;
    } else {
      /* PB7 - ~RAMP */
      if ( V_ACR & V_T1_PB7_OUT ) {
	 if ( !( V_IFR & V_T1_INT_F ) ) {
	   DX = VG_DX;
	   DY = VG_DY;
	 }
      } else { 
	 if ( !( V_ORB & V__RAMP ) ) {
	   DX = VG_DX;
	   DY = VG_DY;
	 }
      }
    }

    /* Draw Line */
    //    if ( DX > 0 || DY > 0 ) {
      if ( ( V_PCR & V__BLANK ) == V__BLANK_H ) {
	InfoVectrex_DrawLine( VG_CX, VG_CY, VG_CX + DX, VG_CY + DY, VG_Z );
      }
      //    }

    /* Position moves */
    VG_CX += DX;
    VG_CY += DY;

#ifdef DEBUG
    //      if ( nFrame >= nFrameDebug )
    if ( bDebug )
      printf("%d, %d, %d, %x\n",VG_CX,VG_CY,VG_Z,V_PCR & V__BLANK);
#else
      //      if ( ( V_PCR & V__BLANK ) == V__BLANK_H )
      //	 printf("%d, %d, %d\n", VG_CX,VG_MAX_Y - VG_CY,VG_Z );
#endif

    /*---------------------------------------------------------------*/
    /*  6522 Step                                                    */
    /*---------------------------------------------------------------*/
    M6522_Step( 1 );
  }
}

/*---------------------------------------------------------------*/
/*  Update beam position                                         */
/*---------------------------------------------------------------*/
void VG_Update( void )
{ 
  /*---------------------------------------------------------------*/
  /*  MUX                                                          */
  /*---------------------------------------------------------------*/
  switch ( V_ORB & V_SEL )
  {
  case 0x00:
    /* MUX sel: 00-Y integrator */
    if ( !( V_ORB & V_SWITCH ) ) {
      /* Enable MUX */
      VG_Y = VG_X;
    }
    /* Joystick channel #0 */
    VG_J = VG_JCH0;
    break;
    
  case 0x02:
    /* MUX sel: 01-X,Y Axis integrator */
    if ( !( V_ORB & V_SWITCH ) ) {
      /* Enable MUX */
      VG_R = VG_X;
    }
    /* Joystick channel #1 */
    VG_J = VG_JCH1;
    break;

  case 0x04:
    /* MUX sel: 02-Z Axis (Vector Brightness) level */
    if ( !( V_ORB & V_SWITCH ) ) {
      /* Enable MUX */
      if ( VG_X > 0x80 ) {
	 VG_Z = VG_X - 0x80;
      } else {
	 VG_Z = 0;
      }
    }
    /* Joystick channel #2 */
    VG_J = VG_JCH2;
    break;

  case 0x06:
    /* Joystick channel #3 */
    VG_J = VG_JCH3;
    break;
  }

  /* compare the current joystick direction with a reference */
  if ( VG_J > VG_X ) {
    //    V_SET( V_ORB, V_COMPARE );
    V_SET( VG_COMPARE, V_COMPARE );
  } else {
    //    V_RST( V_ORB, V_COMPARE );
    V_RST( VG_COMPARE, V_COMPARE );
  }

  /* compute new deltas */
  VG_DX = (int)VG_X - (int)VG_R; 
  VG_DY = (int)VG_R - (int)VG_Y; 

#ifdef DEBUG
  //  if ( nFrame >= nFrameDebug )
  if ( bDebug )
    printf("VG DX=%d DY=%d X=%d,Y=%d,R=%d,J=%d\n",VG_DX,VG_DY,VG_X,VG_Y,VG_R,VG_J);
#endif
}
