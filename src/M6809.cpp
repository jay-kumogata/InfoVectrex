#include <stdio.h>
#include "M6809.h"
#include "Memory.h"
#include "VG.h"
#include "System.h"

/*-------------------------------------------------------------------*/
/*  CPU Resources                                                    */
/*-------------------------------------------------------------------*/

/* Registers */
WORD X, Y, U, S, PC;
BYTE A, B, DP, F, PB;

/* Post Byte Register Array */
WORD *PB_R[ 4 ] = { &X, &Y, &U, &S };

/* The state of the NMI pin */
BYTE NMI_State;

/* Wiring of the NMI pin */
BYTE NMI_Wiring;

/* The state of the FIRQ pin */
BYTE FIRQ_State;

/* Wiring of the FIRQ pin */
BYTE FIRQ_Wiring;

/* The state of the IRQ pin */
BYTE IRQ_State;

/* Wiring of the IRQ pin */
BYTE IRQ_Wiring;

/* The state of the Interrupt */
BYTE INT_State;

/* The number of the clocks that it passed */
WORD g_wPassedClocks;

/*-------------------------------------------------------------------*/
/*  Constant Tables                                                  */
/*-------------------------------------------------------------------*/

/* A table for the test */
BYTE g_byTestTable[ 256 ];

/* Value and Flag Data */
struct value_table_tag
{
  BYTE byValue;
  BYTE byFlag;
};

/* A table for ASL */
struct value_table_tag g_ASLTable[ 256 ];

/* A table for ASR */
struct value_table_tag g_ASRTable[ 256 ];

/* A table for LSR */
struct value_table_tag g_LSRTable[ 256 ];

/* A table for ROL */
struct value_table_tag g_ROLTable[ 2 ][ 256 ];

/* A table for ROR */
struct value_table_tag g_RORTable[ 2 ][ 256 ];

/*===================================================================*/
/*                                                                   */
/*                M6809_Init() : Initialize M6809                    */
/*                                                                   */
/*===================================================================*/
void M6809_Init( void )
{
  BYTE idx;
  BYTE idx2;

  /* The establishment of the IRQ pin */
  NMI_Wiring = NMI_State = 1;
  FIRQ_Wiring = FIRQ_State = 1;
  IRQ_Wiring = IRQ_State = 1;

  /* Make a table for TEST() */
  idx = 0;
  do {
    if ( idx == 0 )
      g_byTestTable[ idx ] = FLAG_Z;
    else
    if ( idx > 127 )
      g_byTestTable[ idx ] = FLAG_N;
    else
      g_byTestTable[ idx ] = 0;

    ++idx;
  } while ( idx != 0 );

  /* Make a table ASL */
  idx = 0;
  do
  {
    g_ASLTable[ idx ].byValue = idx << 1;
    g_ASLTable[ idx ].byFlag = 0;

    if ( idx > 127 )
      g_ASLTable[ idx ].byFlag = FLAG_C | FLAG_V;

    if ( g_ASLTable[ idx ].byValue == 0 )
      g_ASLTable[ idx ].byFlag |= FLAG_Z;
    else
    if ( g_ASLTable[ idx ].byValue & 0x80 )
      g_ASLTable[ idx ].byFlag |= FLAG_N;

    ++idx;
  } while ( idx != 0 );

  /* Make a table ASR */
  idx = 0;
  do
  {
    g_ASRTable[ idx ].byValue = ( idx >> 1 ) | ( idx & 0x80 );
    g_ASRTable[ idx ].byFlag = 0;

    if ( idx & 0x01 )
      g_ASRTable[ idx ].byFlag = FLAG_C;

    if ( g_ASRTable[ idx ].byValue == 0 )
      g_ASRTable[ idx ].byFlag |= FLAG_Z;
    else
    if ( g_ASRTable[ idx ].byValue & 0x80 )
      g_ASRTable[ idx ].byFlag |= FLAG_N;

    ++idx;
  } while ( idx != 0 );

  /* Make a table LSR */
  idx = 0;
  do
  {
    g_LSRTable[ idx ].byValue = ( idx >> 1 );
    g_LSRTable[ idx ].byFlag = 0;

    if ( idx & 0x01 )
      g_LSRTable[ idx ].byFlag = FLAG_C;

    if ( g_LSRTable[ idx ].byValue == 0 )
      g_LSRTable[ idx ].byFlag |= FLAG_Z;

    ++idx;
  } while ( idx != 0 );

  /* Make a table ROL */
  for ( idx2 = 0; idx2 < 2; ++idx2 )
  {
    idx = 0;
    do
    {
      g_ROLTable[ idx2 ][ idx ].byValue = ( idx << 1 ) | idx2;
      g_ROLTable[ idx2 ][ idx ].byFlag = 0;

      if ( idx > 127 )
        g_ROLTable[ idx2 ][ idx ].byFlag = FLAG_C;

      if ( g_ROLTable[ idx2 ][ idx ].byValue == 0 )
        g_ROLTable[ idx2 ][ idx ].byFlag |= FLAG_Z;
      else
      if ( g_ROLTable[ idx2 ][ idx ].byValue & 0x80 )
        g_ROLTable[ idx2 ][ idx ].byFlag |= FLAG_N;

      ++idx;
    } while ( idx != 0 );
  }

  /* Make a table ROR */
  for ( idx2 = 0; idx2 < 2; ++idx2 )
  {
    idx = 0;
    do
    {
      g_RORTable[ idx2 ][ idx ].byValue = ( idx >> 1 ) | ( idx2 << 7 );
      g_RORTable[ idx2 ][ idx ].byFlag = 0;

      if ( idx & 1 )
        g_RORTable[ idx2 ][ idx ].byFlag = FLAG_C;

      if ( g_RORTable[ idx2 ][ idx ].byValue == 0 )
        g_RORTable[ idx2 ][ idx ].byFlag |= FLAG_Z;
      else
      if ( g_RORTable[ idx2 ][ idx ].byValue & 0x80 )
        g_RORTable[ idx2 ][ idx ].byFlag |= FLAG_N;

      ++idx;
    } while ( idx != 0 );
  }
}

/*===================================================================*/
/*                                                                   */
/*                  M6809_Reset() : Reset CPU                        */
/*                                                                   */
/*===================================================================*/
void M6809_Reset( void )
{
/*
 *  Reset CPU
 *
 */

  /* Reset Registers */
  PC = VECTOR_RESET;
  DP = 0x00;
  F = FLAG_F | FLAG_I;

  /* Set up the state of the Interrupt pin. */
  NMI_State = NMI_Wiring;
  FIRQ_State = FIRQ_Wiring;
  IRQ_State = IRQ_Wiring;
  INT_State = INT_NORMAL;

  /* Reset Passed Clocks */
  g_wPassedClocks = 0;
}

/*===================================================================*/
/*                                                                   */
/* M6809_Step() : Only the specified number of the clocks execute Op.*/
/*                                                                   */
/*===================================================================*/
void M6809_Step( WORD wClocks )
{
  BYTE byCode;

  BYTE byD0, byD1;
  WORD wA0, wD0, wD1;
  DWORD dwD0;

  /* It has a loop until a constant clock passes */
  while ( g_wPassedClocks < wClocks )
  {
    /* Dispose of it if there is an interrupt requirement */
    if ( NMI_State != NMI_Wiring ) 
    {
      /* NMI Interrupt */
      NMI_State = NMI_Wiring;
      CLK( 18 );

      SETF( FLAG_E );
      PSHSW( PC ); PSHSW( U ); PSHSW( Y ); PSHSW( X );
      PSHS( DP ); PSHS( B ); PSHS( A ); PSHS( F );

      SETF( FLAG_F | FLAG_I );
      PC = VECTOR_NMI;
    }
    else if ( FIRQ_State != FIRQ_Wiring )
    {
      /* FIRQ Interrupt */
      /* Execute FIRQ if an F flag isn't being set */
      if ( !( F & FLAG_F ) )
      {
	 FIRQ_State = FIRQ_Wiring;
	 CLK( 9 );

	 if ( INT_State != INT_CWAI ) {
	   RSTF( FLAG_E );
	   PSHSW( PC ); PSHS( F );
	 }

      SETF( FLAG_F | FLAG_I );
      PC = VECTOR_FIRQ;
      INT_State = INT_NORMAL;
      } 
      else
      {
	 if ( INT_State == INT_SYNC ) {
	   INT_State = INT_NORMAL;
	 }
      }
    }
    else if ( IRQ_State != IRQ_Wiring )
    {
      /* IRQ Interrupt */
      /* Execute IRQ if an I flag isn't being set */
      if ( !( F & FLAG_I ) )
      {
	 IRQ_State = IRQ_Wiring;
	 CLK( 18 );

	 if ( INT_State != INT_SYNC ) {
	   SETF( FLAG_E );
	   PSHSW( PC ); PSHSW( U ); PSHSW( Y ); PSHSW( X );
	   PSHS( DP ); PSHS( B ); PSHS( A ); PSHS( F );
	 }

	 SETF( FLAG_I );
	 PC = VECTOR_IRQ;
	 INT_State = INT_NORMAL;
      } else {
	 if ( INT_State == INT_SYNC ) {
	   INT_State = INT_NORMAL;
	 }
      }
    }

    /* Continue, if Interrupt State isn't INT_NORMAL */
    if ( INT_State != INT_NORMAL ) { CLK( 1 ); }
    else { break; }
  }

  /* It has a loop until a constant clock passes */
  while ( g_wPassedClocks < wClocks )
  {
    byCode = M6809_Read( PC++ );

    switch ( byCode )
    {
    case 0x00:
      /* NEG Direct */
      NEG( AA_DIR ); CLK( 6 );
      break;

    case 0x03:
      /* COM Direct */
      COM( AA_DIR ); CLK( 6 );
      break;

    case 0x04:
      /* LSR Direct */
      LSR( AA_DIR ); CLK( 6 );
      break;

    case 0x06:
      /* ROR Direct */
      ROR( AA_DIR ); CLK( 6 );
      break;

    case 0x07:
      /* ASR Direct */
      ASR( AA_DIR ); CLK( 6 );
      break;

    case 0x08:
      /* ASL Direct & LSL Direct */
      ASL( AA_DIR ); CLK( 6 );
      break;

    case 0x09:
      /* ROL Direct */
      ROL( AA_DIR ); CLK( 6 );
      break;

    case 0x0A:
      /* DEC Direct */
      DEC( AA_DIR ); CLK( 6 );
      break;

    case 0x0C:
      /* INC Direct */
      INC( AA_DIR ); CLK( 6 );
      break;

    case 0x0D:
      /* TST Direct */
      TST( AA_DIR ); CLK( 6 );
      break;

    case 0x0E:
      /* JMP Direct */
      JMP( AA_DIR ); CLK( 3 );
      break;

    case 0x0F:
      /* CLR Direct */
      CLR( AA_DIR ); CLK( 6 );
      break;

    case 0x10:
      /* 2 Byte Instructions */
      byCode = M6809_Read( PC++ );

      switch ( byCode )
      {
      case 0x20:
	 /* LBRA */
	 LBRA( 1 );
	 break;

      case 0x21:
	 /* LBRN */
	 LBRA( 0 );
	 break;

      case 0x22:
 	 /* LBHI */
	 LBRA( !( A_C | A_Z ) );
	 break;

      case 0x23:
	 /* LBLS */
	 LBRA( A_C | A_Z );
	 break;

      case 0x24:
	 /* LBCC & BHS */
	 LBRA( !A_C );
	 break;
	
      case 0x25:
	 /* LBCS & BLO*/
	 LBRA( A_C );
	 break;
	
      case 0x26:
	 /* LBNE */
	 LBRA( !A_Z );
	 break;
	
      case 0x27:
	 /* LBEQ */
	 LBRA( A_Z );
	 break;
	
      case 0x28:
	 /* LBVC */
	 LBRA( !A_V );
	 break;
	
      case 0x29:
	 /* LBVS */
	 LBRA( A_V );
	 break;
	
      case 0x2A:
	 /* LBPL */
	 LBRA( !A_N );
	 break;
	
      case 0x2B:
	 /* LBMI */
	 LBRA( A_N );
	 break;
	
      case 0x2C:
	 /* LBGE */
	 LBRA( !( A_N ^ A_V ) );
	 break;
	
      case 0x2D:
	 /* LBLT */
	 LBRA( A_N ^ A_V );
	 break;
	
      case 0x2E:
	 /* LBGT */
	 LBRA( !( A_Z | ( A_N ^ A_V ) ) );
	 break;

      case 0x2F:
	 /* LBLE */
	 LBRA( A_Z | ( A_N ^ A_V ) );
	 break;

      case 0x3F:
	 /* SWI2 */
	 SWI2; CLK( 20 );
	 break;

      case 0x83:
	 /* CMPD Immdiate */
	 CMPD( AW_IMM ); CLK( 5 );
	 break;

      case 0x8C:
	 /* CMPY Immdiate */
	 CMPY( AW_IMM ); CLK( 5 );
	 break;

      case 0x8E:
	 /* LDY Immdiate */
	 LDY( AW_IMM ); CLK( 4 );
	 break;

      case 0x93:
	 /* CMPD Direct */
	 CMPD( AW_DIR ); CLK( 7 );
	 break;

      case 0x9C:
	 /* CMPY Direct */
	 CMPY( AW_DIR ); CLK( 7 );
	 break;

      case 0x9E:
	 /* LDY Direct */
	 LDY( AW_DIR ); CLK( 6 );
	 break;

      case 0x9F:
	/* STY Direct */
	STY( AA_DIR ); CLK( 6 );
	break;

      case 0xA3:
	 /* CMPD Index */
	 CMPD( AW_IND ); CLK( 7 );
	 break;

      case 0xAC:
	 /* CMPY Index */
	 CMPY( AW_IND ); CLK( 7 );
	 break;

      case 0xAE:
	 /* LDY Index */
	 LDY( AW_IND ); CLK( 6 );
	 break;

      case 0xAF:
	/* STY Index */
	STY( AA_IND ); CLK( 6 );
	break;

      case 0xB3:
        /* CMPD Extended */
	 CMPD( AW_EXT ); CLK( 8 );
	 break;

      case 0xBC:
        /* CMPY Extended */
	 CMPY( AW_EXT ); CLK( 8 );
	 break;

      case 0xBE:
        /* LDY Extended */
	 LDY( AW_EXT ); CLK( 7 );
	 break;

      case 0xBF:
	/* STY Extended */
	STY( AA_EXT ); CLK( 7 );
	break;

      case 0xCE:
	 /* LDS Immediate */
	 LDS( AW_IMM ); CLK( 4 );
	 break;

      case 0xDE:
        /* LDS Direct */
	 LDS( AW_DIR ); CLK( 6 );
	 break;

      case 0xDF:
	/* STS Direct */
	STS( AA_DIR ); CLK( 6 );
	break;
      
      case 0xEE:
        /* LDS Index */
        LDS( AW_IND ); CLK( 6 );
	 break;

      case 0xEF:
	/* STS Index */
	STS( AA_IND ); CLK( 6 );
	break;
      
      case 0xFE:
        /* LDS Extended */
        LDS( AW_EXT ); CLK( 7 );
        break;

      case 0xFF:
        /* STS Extended */
	 STS( AA_EXT ); CLK( 7 );
	 break;
      }
      break;

    case 0x11:
      /* 2 Byte Instructions */
      byCode = M6809_Read( PC++ );

      switch ( byCode )
      {
      case 0x83:
	 /* CMPU Immdiate */
	 CMPU( AW_IMM ); CLK( 5 );
	 break;

      case 0x8C:
	 /* CMPS Immdiate */
	 CMPS( AW_IMM ); CLK( 5 );
	 break;

      case 0x93:
	 /* CMPU Direct */
	 CMPU( AW_DIR ); CLK( 7 );
	 break;

      case 0x9C:
	 /* CMPS Direct */
	 CMPS( AW_DIR ); CLK( 7 );
	 break;

      case 0xA3:
	 /* CMPU Index */
	 CMPU( AW_IND ); CLK( 7 );
	 break;

      case 0xAC:
	 /* CMPS Index */
	 CMPS( AW_IND ); CLK( 7 );
	 break;

      case 0xB3:
        /* CMPU Extended */
	 CMPU( AW_EXT ); CLK( 8 );
	 break;

      case 0xBC:
        /* CMPS Extended */
	 CMPS( AW_EXT ); CLK( 8 );
	 break;

      case 0x3F:
	 /* SWI3 */
	 SWI3; CLK( 20 );
	 break;
      }
      break;

    case 0x12:
      /* NOP */
      CLK( 2 );
      break;

    case 0x13:
      /* SYNC */
      SYNC; CLK( 2 );
      break;

    case 0x17:
      /* LBSR */
      LBSR; CLK( 9 ); 
      break;

    case 0x19:
      /* DAA */
      DAA; CLK( 2 );
      break;

    case 0x1A:
      /* ORCC Immediate */
      ORF( A_IMM ); CLK( 3 );
      break;

    case 0x1C:
      /* ANDCC Immediate */
      ANDF( A_IMM ); CLK( 3 );
      break;

    case 0x1D:
      /* SEX */
      SEX; CLK( 2 );
      break;

    case 0x1E:
      /* EXG */
      EXG; CLK( 8 );
      break;

    case 0x1F:
      /* TFR */
      TFR; CLK( 7 );
      break;

    case 0x20:
      /* BRA */
      BRA( 1 ); CLK( 3 );
      break;

    case 0x21:
      /* BRN */
      BRA( 0 ); CLK( 3 );
      break;

    case 0x22:
      /* BHI */
      BRA( !( A_C | A_Z ) ); CLK( 3 );
      break;

    case 0x23:
      /* BLS */
      BRA( A_C | A_Z ); CLK( 3 );
      break;

    case 0x24:
      /* BCC & BHS */
      BRA( !A_C ); CLK( 3 );
      break;

    case 0x25:
      /* BCS & BLO*/
      BRA( A_C ); CLK( 3 );
      break;

    case 0x26:
      /* BNE */
      BRA( !A_Z ); CLK( 3 );
      break;

    case 0x27:
      /* BEQ */
      BRA( A_Z ); CLK( 3 );
      break;

    case 0x28:
      /* BVC */
      BRA( !A_V ); CLK( 3 );
      break;

    case 0x29:
      /* BVS */
      BRA( A_V ); CLK( 3 );
      break;

    case 0x2A:
      /* BPL */
      BRA( !A_N ); CLK( 3 );
      break;

    case 0x2B:
      /* BMI */
      BRA( A_N ); CLK( 3 );
      break;

    case 0x2C:
      /* BGE */
      BRA( !( A_N ^ A_V ) ); CLK( 3 );
      break;

    case 0x2D:
      /* BLT */
      BRA( A_N ^ A_V ); CLK( 3 );
      break;

    case 0x2E:
      /* BGT */
      BRA( !( A_Z | ( A_N ^ A_V ) ) ); CLK( 3 );
      break;

    case 0x2F:
      /* BLE */
      BRA( A_Z | ( A_N ^ A_V ) ); CLK( 3 );
      break;

    case 0x30:
      /* LEAX */
      LEAX( AA_IND ); CLK( 4 );
      break;

    case 0x31:
      /* LEAY */
      LEAY( AA_IND ); CLK( 4 );
      break;

    case 0x32:
      /* LEAS */
      LEAS( AA_IND ); CLK( 4 );
      break;

    case 0x33:
      /* LEAU */
      LEAU( AA_IND ); CLK( 4 );
      break;

    case 0x34:
      /* PSHS */
      byCode = M6809_Read( PC++ ); CLK( 5 );

      if ( byCode & 0x80 ) { PSHSW( PC ); CLK( 2 ); }
      if ( byCode & 0x40 ) { PSHSW( U ); CLK( 2 ); }
      if ( byCode & 0x20 ) { PSHSW( Y ); CLK( 2 ); }
      if ( byCode & 0x10 ) { PSHSW( X ); CLK( 2 ); } 
      if ( byCode & 0x08 ) { PSHS( DP ); CLK( 1 ); }
      if ( byCode & 0x04 ) { PSHS( B ); CLK( 1 ); }
      if ( byCode & 0x02 ) { PSHS( A ); CLK( 1 ); }
      if ( byCode & 0x01 ) { PSHS( F ); CLK( 1 ); }  
      break;

    case 0x35:
      /* PULS */
      byCode = M6809_Read( PC++ ); CLK( 5 );

      if ( byCode & 0x01 ) { PULS( F ); CLK( 1 ); }    
      if ( byCode & 0x02 ) { PULS( A ); CLK( 1 ); }    
      if ( byCode & 0x04 ) { PULS( B ); CLK( 1 ); }  
      if ( byCode & 0x08 ) { PULS( DP ); CLK( 1 ); }  
      if ( byCode & 0x10 ) { PULSW( X ); CLK( 2 ); }  
      if ( byCode & 0x20 ) { PULSW( Y ); CLK( 2 ); }
      if ( byCode & 0x40 ) { PULSW( U ); CLK( 2 ); } 
      if ( byCode & 0x80 ) { PULSW( PC ); CLK( 2 ); }
      break;

    case 0x36:
      /* PSHU */
      byCode = M6809_Read( PC++ ); CLK( 5 );

      if ( byCode & 0x80 ) PSHUW( PC );
      if ( byCode & 0x40 ) PSHUW( U ); 
      if ( byCode & 0x20 ) PSHUW( Y ); 
      if ( byCode & 0x10 ) PSHUW( X ); 
      if ( byCode & 0x08 ) PSHU( DP );
      if ( byCode & 0x04 ) PSHU( B );
      if ( byCode & 0x02 ) PSHU( A );  
      if ( byCode & 0x01 ) PSHU( F );  
      break;

    case 0x37:
      /* PULU */
      byCode = M6809_Read( PC++ ); CLK( 5 );

      if ( byCode & 0x01 ) PULU( F );  
      if ( byCode & 0x02 ) PULU( A );  
      if ( byCode & 0x04 ) PULU( B );
      if ( byCode & 0x08 ) PULU( DP );
      if ( byCode & 0x10 ) PULUW( X ); 
      if ( byCode & 0x20 ) PULUW( Y ); 
      if ( byCode & 0x40 ) PULUW( U ); 
      if ( byCode & 0x80 ) PULUW( PC );
      break;

    case 0x39:
      /* RTS */
      RTS; CLK( 5 ); 
      break;

    case 0x3A:
      /* ABX */
      ABX; CLK( 3 );
      break;

    case 0x3B:
      /* RTI */
      RTI; CLK( 6 ); 
      break;

    case 0x3C:
      /* CWAI */
      CWAI( A_IMM ); CLK( 20 );
      break;

    case 0x3D:
      /* MUL */
      MUL; CLK( 11 );
      break;

    case 0x3F:
      /* SWI */
      SWI; CLK( 19 );
      break;

    case 0x40:
      /* NEGA */
      NEGA; CLK( 2 );
      break;

    case 0x43:
      /* COMA */
      COMA; CLK( 2 );
      break;

    case 0x44:
      /* LSRA */
      LSRA; CLK( 2 );
      break;

    case 0x46:
      /* RORA */
      RORA; CLK( 2 );
      break;

    case 0x47:
      /* ASRA */
      ASRA; CLK( 2 );
      break;

    case 0x48:
      /* ASLA & LSLA */
      ASLA; CLK( 2 );
      break;

    case 0x49:
      /* ROLA */
      ROLA; CLK( 2 );
      break;

    case 0x4A:
      /* DECA */
      DECA; CLK( 2 );
      break;

    case 0x4C:
      /* INCA */
      INCA; CLK( 2 );
      break;

    case 0x4D:
      /* TSTA */
      TSTA; CLK( 2 );
      break;

    case 0x4F:
      /* CLRA */
      CLRA; CLK( 2 );
      break;

    case 0x50:
      /* NEGB */
      NEGB; CLK( 2 );
      break;

    case 0x53:
      /* COMB */
      COMB; CLK( 2 );
      break;

    case 0x54:
      /* LSRB */
      LSRB; CLK( 2 );
      break;

    case 0x56:
      /* RORB */
      RORB; CLK( 2 );
      break;

    case 0x57:
      /* ASRB */
      ASRB; CLK( 2 );
      break;

    case 0x58:
      /* ASLB & LSLB*/
      ASLB; CLK( 2 );
      break;

    case 0x59:
      /* ROLB */
      ROLB; CLK( 2 );
      break;

    case 0x5A:
      /* DECB */
      DECB; CLK( 2 );
      break;

    case 0x5C:
      /* INCB */
      INCB; CLK( 2 );
      break;

    case 0x5D:
      /* TSTB */
      TSTB; CLK( 2 );
      break;

    case 0x5F:
      /* CLRB */
      CLRB; CLK( 2 );
      break;

    case 0x60:
      /* NEG Index */
      NEG( AA_IND ); CLK( 6 );
      break;

    case 0x63:
      /* COM Index */
      COM( AA_IND ); CLK( 6 );
      break;

    case 0x64:
      /* LSR Index */
      LSR( AA_IND ); CLK( 6 );
      break;

    case 0x66:
      /* ROR Index */
      ROR( AA_IND ); CLK( 6 );
      break;

    case 0x67:
      /* ASR Index */
      ASR( AA_IND ); CLK( 6 );
      break;

    case 0x68:
      /* ASL Index & LSL Index */
      ASL( AA_IND ); CLK( 6 );
      break;

    case 0x69:
      /* ROL Index */
      ROL( AA_IND ); CLK( 6 );
      break;

    case 0x6A:
      /* DEC Index */
      DEC( AA_IND ); CLK( 6 );
      break;

    case 0x6C:
      /* INC Index */
      INC( AA_IND ); CLK( 6 );
      break;

    case 0x6D:
      /* TST Index */
      TST( AA_IND ); CLK( 6 );
      break;

    case 0x6E:
      /* JMP Index */
      JMP( AA_IND ); CLK( 3 );
      break;

    case 0x6F:
      /* CLR Index */
      CLR( AA_IND ); CLK( 6 );
      break;

    case 0x70:
      /* NEG Extended */
      NEG( AA_EXT ); CLK( 7 );
      break;

    case 0x73:
      /* COM Extended */
      COM( AA_EXT ); CLK( 7 );
      break;

    case 0x74:
      /* LSR Extended */
      LSR( AA_EXT ); CLK( 7 );
      break;

    case 0x76:
      /* ROR Extended */
      ROR( AA_EXT ); CLK( 7 );
      break;

    case 0x77:
      /* ASR Extended */
      ASR( AA_EXT ); CLK( 7 );
      break;

    case 0x78:
      /* ASL Extended & LSL Extended */
      ASL( AA_EXT ); CLK( 7 );
      break;

    case 0x79:
      /* ROL Extended */
      ROL( AA_EXT ); CLK( 7 );
      break;

    case 0x7A:
      /* DEC Extended */
      DEC( AA_EXT ); CLK( 7 );
      break;

    case 0x7C:
      /* INC Extended */
      INC( AA_EXT ); CLK( 7 );
      break;

    case 0x7D:
      /* TST Extended */
      TST( AA_EXT ); CLK( 7 );
      break;

    case 0x7E:
      /* JMP Extended */
      JMP( AA_EXT ); CLK( 4 );
      break;

    case 0x7F:
      /* CLR Extended */
      CLR( AA_EXT ); CLK( 7 );
      break;

    case 0x80:
      /* SUBA Immediate */
      SUBA( A_IMM ); CLK( 2 );
      break;

    case 0x81:
      /* CMPA Immediate */
      CMPA( A_IMM ); CLK( 2 );
      break;

    case 0x82:
      /* SBCA Immediate */
      SBCA( A_IMM ); CLK( 2 );
      break;

    case 0x83:
      /* SUBD Immediate */
      SUBD( AW_IMM ); CLK( 4 );
      break;

    case 0x84:
      /* ANDA Immediate */
      ANDA( A_IMM ); CLK( 2 );
      break;

    case 0x85:
      /* BITA Immediate */
      BITA( A_IMM ); CLK( 2 );
      break;

    case 0x86:
      /* LDA Immediate */
      LDA( A_IMM ); CLK( 2 );
      break;

    case 0x88:
      /* EORA Immediate */
      EORA( A_IMM ); CLK( 2 );
      break;

    case 0x89:
      /* ADCA Immediate */
      ADCA( A_IMM ); CLK( 2 );
      break;

    case 0x8A:
      /* ORA Immediate */
      ORA( A_IMM ); CLK( 2 );
      break;

    case 0x8B:
      /* ADDA Immdiate */
      ADDA( A_IMM ); CLK( 2 );
      break;

    case 0x8C:
      /* CMPX Immdiate */
      CMPX( AW_IMM ); CLK( 4 );
      break;

    case 0x8D:
      /* BSR */
      BSR; CLK( 7 ); 
      break;

    case 0x8E:
      /* LDX Immdiate */
      LDX( AW_IMM ); CLK( 3 );
      break;

    case 0x90:
      /* SUBA Direct */
      SUBA( A_DIR ); CLK( 4 );
      break;

    case 0x91:
      /* CMPA Direct */
      CMPA( A_DIR ); CLK( 4 );
      break;

    case 0x92:
      /* SBCA Direct */
      SBCA( A_DIR ); CLK( 4 );
      break;

    case 0x93:
      /* SUBD Direct */
      SUBD( AW_DIR ); CLK( 6 );
      break;

    case 0x94:
      /* ANDA Direct */
      ANDA( A_DIR ); CLK( 4 );
      break;

    case 0x95:
      /* BITA Direct */
      BITA( A_DIR ); CLK( 4 );
      break;

    case 0x96:
      /* LDA Direct */
      LDA( A_DIR ); CLK( 4 );
      break;

    case 0x97:
      /* STA Direct */
      STA( AA_DIR ); CLK( 4 );
      break;
      
    case 0x98:
      /* EORA Direct */
      EORA( A_DIR ); CLK( 4 );
      break;

    case 0x99:
      /* ADCA Direct */
      ADCA( A_DIR ); CLK( 4 );
      break;

    case 0x9A:
      /* ORA Direct */
      ORA( A_DIR ); CLK( 4 );
      break;

    case 0x9B:
      /* ADDA Direct */
      ADDA( A_DIR ); CLK( 4 );
      break;

    case 0x9C:
      /* CMPX Direct */
      CMPX( AW_DIR ); CLK( 6 );
      break;

    case 0x9D:
      /* JSR Direct */
      JSR( AA_DIR ); CLK( 7 );
      break;

    case 0x9E:
      /* LDX Direct */
      LDX( AW_DIR ); CLK( 5 );
      break;

    case 0x9F:
      /* STX Direct */
      STX( AA_DIR ); CLK( 5 );
      break;

    case 0xA0:
      /* SUBA Index */
      SUBA( A_IND ); CLK( 4 );
      break;
      
    case 0xA1:
      /* CMPA Index */
      CMPA( A_IND ); CLK( 4 );
      break;
      
    case 0xA2:
      /* SBCA Index */
      SBCA( A_IND ); CLK( 4 );
      break;
      
    case 0xA3:
      /* SUBD Index */
      SUBD( AW_IND ); CLK( 6 );
      break;
      
    case 0xA4:
      /* ANDA Index */
      ANDA( A_IND ); CLK( 4 );
      break;
      
    case 0xA5:
      /* BITA Index */
      BITA( A_IND ); CLK( 4 );
      break;
      
    case 0xA6:
      /* LDA Index */
      LDA( A_IND ); CLK( 4 );
      break;
      
    case 0xA7:
      /* STA Index */
      STA( AA_IND ); CLK( 4 );
      break;
      
    case 0xA8:
      /* EORA Index */
      EORA( A_IND ); CLK( 4 );
      break;
      
    case 0xA9:
      /* ADCA Index */
      LDA( A_IND ); CLK( 4 );
      break;

    case 0xAA:
      /* ORA Index */
      ORA( A_IND ); CLK( 4 );
      break;

    case 0xAB:
      /* ADDA Index */
      ADDA( A_IND ); CLK( 4 );
      break;

    case 0xAC:
      /* CMPX Index */
      CMPX( AW_IND ); CLK( 6 );
      break;

    case 0xAD:
      /* JSR Index */
      JSR( AA_IND ); CLK( 7 );
      break;

    case 0xAE:
      /* LDX Index */
      LDX( AW_IND ); CLK( 5 );
      break;

    case 0xAF:
      /* STX Index */
      STX( AA_IND ); CLK( 5 );
      break;

    case 0xB0:
      /* SUBA Extended */
      SUBA( A_EXT ); CLK( 5 );
      break;

    case 0xB1:
      /* CMPA Extended */
      CMPA( A_EXT ); CLK( 5 );
      break;

    case 0xB2:
      /* SBCA Extended */
      SBCA( A_EXT ); CLK( 5 );
      break;

    case 0xB3:
      /* SUBD Extended */
      SUBD( AW_EXT ); CLK( 7 );
      break;

    case 0xB4:
      /* ANDA Extended */
      ANDA( A_EXT ); CLK( 5 );
      break;

    case 0xB5:
      /* BITA Extended */
      BITA( A_EXT ); CLK( 5 );
      break;

    case 0xB6:
      /* LDA Extended */
      LDA( A_EXT ); CLK( 5 );
      break;

    case 0xB7:
      /* STA Extended */
      STA( AA_EXT ); CLK( 5 );
      break;

    case 0xB8:
      /* EORA Extended */
      EORA( A_EXT ); CLK( 5 );
      break;

    case 0xB9:
      /* ADCA Extended */
      ADCA( A_EXT ); CLK( 5 );
      break;

    case 0xBA:
      /* ORA Extended */
      ORA( A_EXT ); CLK( 5 );
      break;

    case 0xBB:
      /* ADDA Extended */
      ADDA( A_EXT ); CLK( 5 );
      break;

    case 0xBC:
      /* CMPX Extended */
      CMPX( AW_EXT ); CLK( 7 );
      break;

    case 0xBD:
      /* JSR Extended */
      JSR( AA_EXT ); CLK( 8 );
      break;

    case 0xBE:
      /* LDX Extended */
      LDX( AW_EXT ); CLK( 6 );
      break;

    case 0xBF:
      /* STX Extended */
      STX( AA_EXT ); CLK( 6 );
      break;

    case 0xC0:
      /* SUBB Immediate */
      SUBB( A_IMM ); CLK( 2 );
      break;
      
    case 0xC1:
      /* CMPB Immediate */
      CMPB( A_IMM ); CLK( 2 );
      break;
      
    case 0xC2:
      /* SBCB Immediate */
      SBCB( A_IMM ); CLK( 2 );
      break;
      
    case 0xC3:
      /* ADDD Immediate */
      ADDD( AW_IMM ); CLK( 4 );
      break;
      
    case 0xC4:
      /* ANDB Immediate */
      ANDB( A_IMM ); CLK( 2 );
      break;
      
    case 0xC5:
      /* BITB Immediate */
      BITB( A_IMM ); CLK( 2 );
      break;
      
    case 0xC6:
      /* LDB Immediate */
      LDB( A_IMM ); CLK( 2 );
      break;
      
    case 0xC8:
      /* EORB Immediate */
      EORB( A_IMM ); CLK( 2 );
      break;
      
    case 0xC9:
      /* ADCB Immediate */
      ADCB( A_IMM ); CLK( 2 );
      break;

    case 0xCA:
      /* ORB Immediate */
      ORB( A_IMM ); CLK( 2 );
      break;

    case 0xCB:
      /* ADDB Immediate */
      ADDB( A_IMM ); CLK( 2 );
      break;

    case 0xCC:
      /* LDD Immediate */
      LDD( AW_IMM ); CLK( 3 );
      break;

    case 0xCE:
      /* LDU Immediate */
      LDU( AW_IMM ); CLK( 3 );
      break;

    case 0xD0:
      /* SUBB Direct */
      SUBB( A_DIR ); CLK( 4 );
      break;

    case 0xD1:
      /* CMPB Direct */
      CMPB( A_DIR ); CLK( 4 );
      break;

    case 0xD2:
      /* SBCB Direct */
      SBCB( A_DIR ); CLK( 4 );
      break;

    case 0xD3:
      /* ADDD Direct */
      ADDD( AW_DIR ); CLK( 6 );
      break;

    case 0xD4:
      /* ANDB Direct */
      ANDB( A_DIR ); CLK( 4 );
      break;

    case 0xD5:
      /* BITB Direct */
      BITB( A_DIR ); CLK( 4 );
      break;

    case 0xD6:
      /* LDB Direct */
      LDB( A_DIR ); CLK( 4 );
      break;

    case 0xD7:
      /* STB Direct */
      STB( AA_DIR ); CLK( 4 );
      break;

    case 0xD8:
      /* EORB Direct */
      EORB( A_DIR ); CLK( 4 );
      break;

    case 0xD9:
      /* ADCB Direct */
      ADCB( A_DIR ); CLK( 4 );
      break;
      
    case 0xDA:
      /* ORB Direct */
      ORB( A_DIR ); CLK( 4 );
      break;
      
    case 0xDB:
      /* ADDB Direct */
      ADDB( A_DIR ); CLK( 4 );
      break;
      
    case 0xDC:
      /* LDD Direct */
      LDD( AW_DIR ); CLK( 5 );
      break;
      
    case 0xDD:
      /* STD Direct */
      STD( AA_DIR ); CLK( 5 );
      break;
      
    case 0xDE:
      /* LDU Direct */
      LDU( AW_DIR ); CLK( 5 );
      break;
      
    case 0xDF:
      /* STU Direct */
      STU( AA_DIR ); CLK( 5 );
      break;
      
    case 0xE0:
      /* SUBB Index */
      SUBB( A_IND ); CLK( 4 );
      break;
      
    case 0xE1:
      /* CMPB Index */
      CMPB( A_IND ); CLK( 4 );
      break;
      
    case 0xE2:
      /* SBCB Index */
      SBCB( A_IND ); CLK( 4 );
      break;
      
    case 0xE3:
      /* ADDD Index */
      ADDD( AW_IND ); CLK( 6 );
      break;
      
    case 0xE4:
      /* ANDB Index */
      ANDB( A_IND ); CLK( 4 );
      break;
      
    case 0xE5:
      /* BITB Index */
      BITB( A_IND ); CLK( 4 );
      break;
      
    case 0xE6:
      /* LDB Index */
      LDB( A_IND ); CLK( 4 );
      break;
      
    case 0xE7:
      /* STB Index */
      STB( AA_IND ); CLK( 4 );
      break;

    case 0xE8:
      /* EORB Index */
      EORB( A_IND ); CLK( 4 );
      break;

    case 0xE9:
      /* ADCB Index */
      ADCB( A_IND ); CLK( 4 );
      break;
      
    case 0xEA:
      /* ORB Index */
      ORB( A_IND ); CLK( 4 );
      break;
      
    case 0xEB:
      /* ADDB Index */
      ADDB( A_IND ); CLK( 4 );
      break;
      
    case 0xEC:
      /* LDD Index */
      LDD( AW_IND ); CLK( 5 );
      break;
      
    case 0xED:
      /* STD Index */
      STD( AA_IND ); CLK( 5 );
      break;
      
    case 0xEE:
      /* LDU Index */
      LDU( AW_IND ); CLK( 5 );
      break;
      
    case 0xEF:
      /* STU Index */
      STU( AA_IND ); CLK( 5 );
      break;
      
    case 0xF0:
      /* SUBB Extended */
      SUBB( A_EXT ); CLK( 5 );
      break;

    case 0xF1:
      /* CMPB Extended */
      CMPB( A_EXT ); CLK( 5 );
      break;

    case 0xF2:
      /* SBCB Extended */
      SBCB( A_EXT ); CLK( 5 );
      break;

    case 0xF4:
      /* ANDB Extended */
      ANDB( A_EXT ); CLK( 5 );
      break;

    case 0xF5:
      /* BITB Extended */
      BITB( A_EXT ); CLK( 5 );
      break;

    case 0xF6:
      /* LDB Extended */
      LDB( A_EXT ); CLK( 5 );
      break;

    case 0xF7:
      /* STB Extended */
      STB( AA_EXT ); CLK( 5 );
      break;

    case 0xF8:
      /* EORB Extended */
      EORB( A_EXT ); CLK( 5 );
      break;

    case 0xF9:
      /* ADCB Extended */
      ADCB( A_EXT ); CLK( 5 );
      break;

    case 0xFA:
      /* ORB Extended */
      ORB( A_EXT ); CLK( 5 );
      break;

    case 0xFB:
      /* ADDB Extended */
      ADDB( A_EXT ); CLK( 5 );
      break;

    case 0xFC:
      /* LDD Extended */
      LDD( AW_EXT ); CLK( 6 );
      break;

    case 0xFD:
      /* STD Extended */
      STD( AA_EXT ); CLK( 6 );
      break;

    case 0xFE:
      /* LDU Extended */
      LDU( AW_EXT ); CLK( 6 );
      break;

    case 0xFF:
      /* STU Extended */
      STU( AA_EXT ); CLK( 6 );
      break;
    }  /* end of switch ( byCode ) */


    /*---------------------------------------------------------------*/
    /*  Patches for Vectrex                                          */
    /*---------------------------------------------------------------*/
    if ( PC == 0xE59E && A == 0x04 ) { A = 0x02; printf("patch\n"); bDebug = 1; }

#ifdef DEBUG
    //    if ( nFrame >= nFrameDebug ) {
    if ( bDebug ) {
      printf("CK=%d PC=%x X=%x Y=%x U=%x S=%x ", g_wPassedClocks,PC,X,Y,U,S);
      printf("Op=%x A=%x B=%x F=%x DP=%x\n", byCode,A,B,F,DP);
    }
#endif

  }   /* end of while ... */

  /* Correct the number of the clocks */
  g_wPassedClocks -= wClocks;
}

/*-------------------------------------------------------------------*/
/*  Addressing Mode                                                  */
/*-------------------------------------------------------------------*/

/* This function returns offset address */
WORD M6809_Index( void )
{
  WORD wInd;
  PB = M6809_Read( PC++ );   // Post Byte
  if ( PB_N5BO ) {           // Not 5 Bit Offset Mode
    switch ( PB_MODE ) {
    case 0x04:
    case 0x14:
      // Zero Offset Mode ( + Indirect )
      wInd = PB_REG;
      if ( PB_INDIR ) { wInd = M6809_ReadW( wInd ); CLK( 3 ); }
      break;

    case 0x08:
    case 0x18:
      // 8 Bit Offset Mode ( + Indirect )
      wInd = M6809_Read( PC++ );
      wInd = SIGNED( wInd ) + PB_REG;
      if ( PB_INDIR ) { wInd = M6809_ReadW( wInd ); CLK( 4 ); }
      else { CLK( 1 ); }
      break;

    case 0x09:
    case 0x19:
      // 16 Bit Offset Mode ( + Indirect )
      wInd = M6809_ReadW( PC ) + PB_REG; PC += 2;
      if ( PB_INDIR ) { wInd = M6809_ReadW( wInd ); CLK( 7 ); }
      else { CLK( 4 ); }
      break;

    case 0x06:
    case 0x16:
      // A Register Offset Mode ( + Indirect )
      wInd = SIGNED( A ) + PB_REG;
      if ( PB_INDIR ) { wInd = M6809_ReadW( wInd ); CLK( 4 ); }
      else { CLK( 1 ); }
      break;

    case 0x05:
    case 0x15:
      // B Register Offset Mode ( + Indirect )
      wInd = SIGNED( B ) + PB_REG;
      if ( PB_INDIR ) { wInd = M6809_ReadW( wInd ); CLK( 4 ); }
      else { CLK( 1 ); }
      break;

    case 0x0B:
    case 0x1B:
      // D Register Offset Mode ( + Indirect )
      wInd = D + PB_REG;
      if ( PB_INDIR ) { wInd = M6809_ReadW( wInd ); CLK( 7 ); }
      else { CLK( 4 ); }
      break;

    case 0x00:
      // Increment(+1) Mode
      wInd = PB_REG++;
      CLK( 2 );
      break;

    case 0x01:
    case 0x11:
      // Increment(+2) Mode ( + Indirect )
      wInd = PB_REG; PB_REG += 2;
      if ( PB_INDIR ) { wInd = M6809_ReadW( wInd ); CLK( 6 ); }
      else { CLK( 3 ); }
      break;

    case 0x02:
      // Decrement(-1) Mode
      wInd = --PB_REG;
      CLK( 2 );
      break;

    case 0x03:
    case 0x13:
      // Decrement(-2) Mode ( + Indirect )
      PB_REG -= 2; wInd = PB_REG; 
      if ( PB_INDIR ) { wInd = M6809_ReadW( wInd ); CLK( 6 ); }
      else { CLK( 3 ); }
      break;

    case 0x1F:
      // Extended Indirect Mode
      wInd = M6809_ReadW( M6809_ReadW( PC ) ); PC += 2;
      CLK( 5 );
      break;

    case 0x0C:
    case 0x1C:
      // PC relative (8 bit) Mode
      wInd = (WORD)M6809_Read( PC++ ); wInd =+ PC;
      if ( PB_INDIR ) { wInd = M6809_ReadW( wInd ); CLK( 4 ); }
      else { CLK( 1 ); }
      break;

    case 0x0D:
    case 0x1D:
      // PC relative (16 bit) Mode
      wInd = M6809_ReadW( PC ); PC += 2; wInd += PC;
      if ( PB_INDIR ) { wInd = M6809_ReadW( wInd ); CLK( 8 ); }
      else { CLK( 5 ); }
      break;

    default:
      break;
    }
  } else {
    // 5 Bit Offset Mode
    wInd = (WORD)PB_MODE;
    wInd = PB_REG + SIGNED5( wInd );
    CLK( 1 );
  }
  // return
  return wInd;
}

#if 0
int main( void )
{
  WORD wD0, wD1;

  X = 0x100; Y = 0x200; U = 0x300; S = 0x400; 
  A = 0x10; B = 0x20; F = 0x30; DP = 0x40;

  printf("D = %x; X = %x; Y = %x; U = %x; S = %x;\n", D,X,Y,U,S);
  printf("A = %x; B = %x; F = %x; DP = %x;\n", A,B,F,DP);
  TFR;
  printf("D = %x; X = %x; Y = %x; U = %x; S = %x;\n", D,X,Y,U,S);
  printf("A = %x; B = %x; F = %x; DP = %x;\n", A,B,F,DP);

#if 0
  printf("X = %d; Y = %d; U = %d; S = %d;\n", X,Y,U,S);
  printf("X = %d; Y = %d; U = %d; S = %d;\n", 
	 *( PB_R[ 0 ] ),
	 *( PB_R[ 1 ] ),
	 *( PB_R[ 2 ] ),
	 *( PB_R[ 3 ] ) );

  PB = 0x00;
  printf("X = %d; ", PB_REG);
  PB_REG++;
  printf("X = %d; ", PB_REG);


  PB = 0x20;
  printf("Y = %d; ", PB_REG);
  PB = 0x40;
  printf("U = %d; ", PB_REG);
  PB = 0x60;
  printf("S = %d; ", PB_REG);
#endif
}

#endif
