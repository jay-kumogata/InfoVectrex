#ifndef M6809_H_INCLUDED
#define M6809_H_INCLUDED

#include "Types.h"

/*-------------------------------------------------------------------*/
/*  Function Prototype                                               */
/*-------------------------------------------------------------------*/
void M6809_Init( void );
void M6809_Reset( void );
WORD M6809_Index( void );
void M6809_Step( WORD wClocks );

/*-------------------------------------------------------------------*/
/*  CPU Resources                                                    */
/*-------------------------------------------------------------------*/

/* Registers */
extern WORD X, Y, U, S, PC;
extern BYTE A, B, DP, F, PB;

/* The state of the NMI pin */
extern BYTE NMI_State;

/* Wiring of the NMI pin */
extern BYTE NMI_Wiring;

/* The state of the FIRQ pin */
extern BYTE FIRQ_State;

/* Wiring of the FIRQ pin */
extern BYTE FIRQ_Wiring;

/* The state of the IRQ pin */
extern BYTE IRQ_State;

/* Wiring of the IRQ pin */
extern BYTE IRQ_Wiring;

/* The state of the Interrupt */
extern BYTE INT_State;

/* The number of the clocks that it passed */
extern WORD g_wPassedClocks;

/*-------------------------------------------------------------------*/
/*  CPU Macros                                                       */
/*-------------------------------------------------------------------*/

/* Registers Macro */
#define D         ( ( (WORD)A << 8 ) | B )
#define S_D(a)    { wD0 = (a); A = wD0 >> 8; B = wD0 & 0xFF; }

/* Condition Code */
#define A_E       ( ( F & FLAG_E ) >> 7 )
#define A_F       ( ( F & FLAG_F ) >> 6 )
#define A_H       ( ( F & FLAG_H ) >> 5 )
#define A_I       ( ( F & FLAG_I ) >> 4 )
#define A_N       ( ( F & FLAG_N ) >> 3 )
#define A_Z       ( ( F & FLAG_Z ) >> 2 )
#define A_V       ( ( F & FLAG_V ) >> 1 )
#define A_C       ( ( F & FLAG_C ) >> 0 )

/* Condition Code Mask */
#define FLAG_E     0x80
#define FLAG_F     0x40
#define FLAG_H     0x20
#define FLAG_I     0x10
#define FLAG_N     0x08
#define FLAG_Z     0x04
#define FLAG_V     0x02
#define FLAG_C     0x01

/* Interrupt Vector */
#define VECTOR_RESET  M6809_ReadW( 0xFFFE )
#define VECTOR_NMI    M6809_ReadW( 0xFFFC )
#define VECTOR_SWI    M6809_ReadW( 0xFFFA )
#define VECTOR_IRQ    M6809_ReadW( 0xFFF8 )
#define VECTOR_FIRQ   M6809_ReadW( 0xFFF6 )
#define VECTOR_SWI2   M6809_ReadW( 0xFFF4 )
#define VECTOR_SWI3   M6809_ReadW( 0xFFF2 )

/* Interrupt State */
#define INT_NORMAL    0
#define INT_SYNC      1
#define INT_CWAI      2

/* Index Mode */ 
#define PB_N5BO    ( PB & 0x80 )
#define PB_REG     ( *( PB_R[ ( PB & 0x60 ) >> 5 ] ) )
#define PB_MODE    ( PB & 0x1f )
#define PB_INDIR   ( PB & 0x10 )

#define SIGNED(a)  ( ( (a) & 0x80 ? 0xff00 : 0x0000 ) | (a) )
#define SIGNED5(a) ( ( (a) & 0x10 ? 0xffe0 : 0x0000 ) | (a) )

/* TFR & EXG Op. */
#define PB_R1      ( ( PB & 0xf0 ) >> 4 )
#define PB_R2      ( PB & 0x0f )
#define PB_RRD(r,a) { switch( a ) { \
                      case 0x00: r = D; break; \
                      case 0x01: r = X; break; \
                      case 0x02: r = Y; break; \
                      case 0x03: r = U; break; \
                      case 0x04: r = S; break; \
                      case 0x05: r = PC; break; \
                      case 0x08: r = (WORD)A; break; \
                      case 0x09: r = (WORD)B; break; \
                      case 0x0A: r = (WORD)F; break; \
                      case 0x0B: r = (WORD)DP; break; } }

#define PB_RWT(a,d) { switch( a ) { \
                      case 0x00: S_D(d); break; \
                      case 0x01: X = (d); break; \
                      case 0x02: Y = (d); break; \
                      case 0x03: U = (d); break; \
                      case 0x04: S = (d); break; \
                      case 0x05: PC = (d); break; \
                      case 0x08: A = (BYTE)(d); break; \
                      case 0x09: B = (BYTE)(d); break; \
                      case 0x0A: F = (BYTE)(d); break; \
                      case 0x0B: DP = (BYTE)(d); break; } }

/*-------------------------------------------------------------------*/
/*  Operation Macros                                                 */
/*-------------------------------------------------------------------*/

/* Clock Op. */
#define CLK(a)  { g_wPassedClocks += (a); VG_Step(a); }

/* Addressing Mode Op. */
/* Address */
/* Extended */
#define AA_EXT  ( ( (WORD)M6809_Read( PC++ ) << 8 ) | M6809_Read( PC++ ) )
/* Direct */
#define AA_DIR  ( M6809_Read( PC++ ) | (WORD)DP << 8 )
/* Index */
#define AA_IND  ( M6809_Index() )

/* Data */
/* Immediate */
#define A_IMM   M6809_Read( PC++ )
/* Extended */
#define A_EXT   M6809_Read( AA_EXT )
/* Direct */
#define A_DIR   M6809_Read( AA_DIR )
/* Index  */
#define A_IND   M6809_Read( AA_IND )

/* Data 16 */
/* Immediate */
#define AW_IMM  AA_EXT
/* Extended */
#define AW_EXT  M6809_ReadW( AA_EXT )
/* Direct */
#define AW_DIR  M6809_ReadW( AA_DIR )
/* Index  */
#define AW_IND  M6809_ReadW( AA_IND )

/* Flag Op. */
#define SETF(a) F |= (a)
#define RSTF(a) F &= ~(a)
#define TEST(a) RSTF( FLAG_N | FLAG_Z ); SETF( g_byTestTable[ a ] )
#define TSTW(a) RSTF( FLAG_N | FLAG_Z ); \
                SETF( ( (a) & 0x8000 ? FLAG_N : 0 ) | ( (a) == 0 ? FLAG_Z : 0 ) )

/* Load & Store Op. */
#define LDA(a)  { A = (a); TEST( A ); RSTF( FLAG_V ); }
#define LDB(a)  { B = (a); TEST( B ); RSTF( FLAG_V ); }
#define LDD(a)  { S_D( a ); TSTW( D ); RSTF( FLAG_V ); }
#define LDS(a)  { S = (a); TSTW( S ); RSTF( FLAG_V ); }
#define LDU(a)  { U = (a); TSTW( U ); RSTF( FLAG_V ); }
#define LDX(a)  { X = (a); TSTW( X ); RSTF( FLAG_V ); }
#define LDY(a)  { Y = (a); TSTW( Y ); RSTF( FLAG_V ); }
#define STA(a)  { M6809_Write( (a), A ); TEST( A ); RSTF( FLAG_V ); }
#define STB(a)  { M6809_Write( (a), B ); TEST( B ); RSTF( FLAG_V ); }
#define STD(a)  { M6809_WriteW( (a), D ); TSTW( D ); RSTF( FLAG_V ); }
#define STS(a)  { M6809_WriteW( (a), S ); TSTW( S ); RSTF( FLAG_V ); }
#define STU(a)  { M6809_WriteW( (a), U ); TSTW( U ); RSTF( FLAG_V ); }
#define STX(a)  { M6809_WriteW( (a), X ); TSTW( X ); RSTF( FLAG_V ); }
#define STY(a)  { M6809_WriteW( (a), Y ); TSTW( Y ); RSTF( FLAG_V ); }
#define LEAS(a) { S = (a); }
#define LEAU(a) { U = (a); }
#define LEAX(a) { X = (a); RSTF( FLAG_Z ); SETF( X == 0 ? FLAG_Z : 0 ); }
#define LEAY(a) { Y = (a); RSTF( FLAG_Z ); SETF( Y == 0 ? FLAG_Z : 0 ); }

/* Logical Op. */
#define ORA(a)  { A |= (a); TEST( A ); RSTF( FLAG_V ); }
#define ORB(a)  { B |= (a); TEST( B ); RSTF( FLAG_V ); }
#define ORF(a)  { F |= (a); }
#define ANDA(a) { A &= (a); TEST( A ); RSTF( FLAG_V ); }
#define ANDB(a) { B &= (a); TEST( B ); RSTF( FLAG_V ); }
#define ANDF(a) { F &= (a); }
#define BITA(a) { byD0 = A & (a); TEST( byD0 ); RSTF( FLAG_V ); }
#define BITB(a) { byD0 = B & (a); TEST( byD0 ); RSTF( FLAG_V ); }
#define EORA(a) { A ^= (a); TEST( A ); RSTF( FLAG_V ); }
#define EORB(a) { B ^= (a); TEST( B ); RSTF( FLAG_V ); }

/* Arithmetic Op. */
/* ADD */
#define ADDA(a) { byD0 = (a); \
                  wD0 = A + byD0; \
                  byD1 = (BYTE)wD0; \
                  RSTF( FLAG_N | FLAG_Z | FLAG_V | FLAG_C | FLAG_H ); \
                  SETF( g_byTestTable[ byD1 ] | \
                        ( ( ~( A ^ byD0 ) & ( A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | \
                        ( ( wD0 > 0xff ) ? FLAG_C : 0 ) | \
                        ( ( ( A & 0x0f ) + ( byD0 & 0x0f ) > 0x0f ) ? FLAG_H : 0 ) ); \
                  A = byD1; }
#define ADDB(a) { byD0 = (a); \
                  wD0 = B + byD0; \
                  byD1 = (BYTE)wD0; \
                  RSTF( FLAG_N | FLAG_Z | FLAG_V | FLAG_C | FLAG_H ); \
                  SETF( g_byTestTable[ byD1 ] | \
                        ( ( ~( B ^ byD0 ) & ( B ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | \
                        ( ( wD0 > 0xff ) ? FLAG_C : 0 ) | \
                        ( ( ( B & 0x0f ) + ( byD0 & 0x0f ) > 0x0f ) ? FLAG_H : 0 ) ); \
                  B = byD1; }
#define ADDD(a) { wD0 = (a); \
                  dwD0 = D + wD0; \
                  wD1 = (WORD)dwD0; \
                  RSTF( FLAG_N | FLAG_Z | FLAG_V | FLAG_C ); \
                  SETF( ( wD1 & 0x8000 ? FLAG_N : 0 ) | \
                        ( wD1 == 0 ? FLAG_Z : 0 ) | \
                        ( ( ~( D ^ wD0 ) & ( D ^ wD1 ) & 0x8000 ) ? FLAG_V : 0 ) | \
                        ( dwD0 > 0xffff ? FLAG_C : 0 ) ); \
                  S_D( wD1 ); } 

/* ADC */
#define ADCA(a) { byD0 = (a); \
                  wD0 = A + byD0 + A_C; \
                  byD1 = (BYTE)wD0; \
                  RSTF( FLAG_N | FLAG_Z | FLAG_V | FLAG_C | FLAG_H ); \
                  SETF( g_byTestTable[ byD1 ] | \
                        ( ( ~( A ^ byD0 ) & ( A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | \
                        ( ( wD0 > 0xff ) ? FLAG_C : 0 ) | \
                        ( ( ( A & 0x0f ) + ( byD0 & 0x0f ) + A_C > 0x0f ) ? FLAG_H : 0 ) ); \
                  A = byD1; }
#define ADCB(a) { byD0 = (a); \
                  wD0 = B + byD0 + A_C; \
                  byD1 = (BYTE)wD0; \
                  RSTF( FLAG_N | FLAG_Z | FLAG_V | FLAG_C | FLAG_H ); \
                  SETF( g_byTestTable[ byD1 ] | \
                        ( ( ~( B ^ byD0 ) & ( B ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | \
                        ( ( wD0 > 0xff ) ? FLAG_C : 0 ) | \
                        ( ( ( B & 0x0f ) + ( byD0 & 0x0f ) + A_C > 0x0f ) ? FLAG_H : 0 ) ); \
                  B = byD1; }

/* SUB */
#define SUBA(a) { byD0 = (a); \
                  wD0 = A - byD0; \
                  byD1 = (BYTE)wD0; \
                  RSTF( FLAG_N | FLAG_Z | FLAG_V | FLAG_C ); \
                  SETF( g_byTestTable[ byD1 ] | \
                        ( ( ( A ^ byD0 ) & ( A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | \
                        ( ( A < byD0 ) ? FLAG_C : 0 ) ); \
		    A = byD1; }
#define SUBB(a) { byD0 = (a); \
                  wD0 = B - byD0; \
                  byD1 = (BYTE)wD0; \
                  RSTF( FLAG_N | FLAG_Z | FLAG_V | FLAG_C ); \
                  SETF( g_byTestTable[ byD1 ] | \
                        ( ( ( B ^ byD0 ) & ( B ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | \
                        ( ( B < byD0 ) ? FLAG_C : 0 ) ); \
		    B = byD1; }
#define SUBD(a) { wD0 = (a); \
                  dwD0 = D - wD0; \
                  wD1 = (WORD)dwD0; \
                  RSTF( FLAG_N | FLAG_Z | FLAG_V | FLAG_C ); \
                  SETF( ( wD1 & 0x8000 ? FLAG_N : 0 ) | \
                        ( wD1 == 0 ? FLAG_Z : 0 ) | \
                        ( ( ( D ^ wD0 ) & ( D ^ wD1 ) & 0x8000 ) ? FLAG_V : 0 ) | \
                        ( ( D < wD0 ) ? FLAG_C : 0 ) ); \
                  S_D( wD1 ); } 

/* SBC */
#define SBCA(a) { byD0 = (a); \
                  wD0 = A - byD0 - A_C; \
                  byD1 = (BYTE)wD0; \
                  RSTF( FLAG_N | FLAG_Z | FLAG_V | FLAG_C ); \
                  SETF( g_byTestTable[ byD1 ] | \
                        ( ( ( A ^ byD0 ) & ( A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | \
                        ( ( A < byD0 ) ? 1 : 0 ) ); \
		    A = byD1; }
#define SBCB(a) { byD0 = (a); \
                  wD0 = B - byD0 - A_C; \
                  byD1 = (BYTE)wD0; \
                  RSTF( FLAG_N | FLAG_Z | FLAG_V | FLAG_C ); \
                  SETF( g_byTestTable[ byD1 ] | \
                        ( ( ( B ^ byD0 ) & ( B ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | \
                        ( ( B < byD0 ) ? 1 : 0 ) ); \
		    B = byD1; }

/* INC */
#define INCA    { byD0 = A; ++A; RSTF( FLAG_N | FLAG_Z | FLAG_V ); \
                  SETF( g_byTestTable[ A ] | \
		          ( ( ~byD0 & A & 0x80 ) ? FLAG_V : 0 ) ); }  
#define INCB    { byD0 = B; ++B; RSTF( FLAG_N | FLAG_Z | FLAG_V ); \
                  SETF( g_byTestTable[ B ] | \
		          ( ( ~byD0 & B & 0x80 ) ? FLAG_V : 0 ) ); }  
#define INC(a)  { wA0 = (a); byD0 = byD1 = M6809_Read( wA0 ); ++byD1; \
                  M6809_Write( wA0, byD1 ); RSTF( FLAG_N | FLAG_Z | FLAG_V ); \
                  SETF( g_byTestTable[ byD1 ] | \
		          ( ( ~byD0 & byD1 & 0x80 ) ? FLAG_V : 0 ) ); }  

/* DEC */
#define DECA    { byD0 = A; --A; RSTF( FLAG_N | FLAG_Z | FLAG_V ); \
                  SETF( g_byTestTable[ A ] | \
		          ( ( byD0 & ~A & 0x80 ) ? FLAG_V : 0 ) ); }  
#define DECB    { byD0 = B; --B; RSTF( FLAG_N | FLAG_Z | FLAG_V ); \
                  SETF( g_byTestTable[ B ] | \
		          ( ( byD0 & ~B & 0x80 ) ? FLAG_V : 0 ) ); }  
#define DEC(a)  { wA0 = (a); byD0 = byD1 = M6809_Read( wA0 ); --byD1; \
                  M6809_Write( wA0, byD1 ); RSTF( FLAG_N | FLAG_Z | FLAG_V ); \
                  SETF( g_byTestTable[ byD1 ] | \
		          ( ( byD0 & ~byD1 & 0x80 ) ? FLAG_V : 0 ) ); }  

/* CLR */
#define CLRA    { A = 0; RSTF( FLAG_N | FLAG_V | FLAG_C ); SETF( FLAG_Z ); } 
#define CLRB    { B = 0; RSTF( FLAG_N | FLAG_V | FLAG_C ); SETF( FLAG_Z ); } 
#define CLR(a)  { M6809_Write( (a), 0 ); \
                  RSTF( FLAG_N | FLAG_V | FLAG_C ); SETF( FLAG_Z ); } 

/* COM */
#define COMA    { A = ~A; RSTF( FLAG_N | FLAG_Z | FLAG_V ); \
                  SETF( g_byTestTable[ A ] | FLAG_C ); }
#define COMB    { B = ~B; RSTF( FLAG_N | FLAG_Z | FLAG_V ); \
                  SETF( g_byTestTable[ B ] | FLAG_C ); }
#define COM(a)  { wA0 = (a); byD0 = M6809_Read( wA0 ); byD0 = ~byD0; \
                  M6809_Write( wA0, byD0 ); RSTF( FLAG_N | FLAG_Z | FLAG_V ); \
                  SETF( g_byTestTable[ byD0 ] | FLAG_C ); }

/* NEG */
#define NEGA    { byD0 = A; A = ~A + 1; \
                  RSTF( FLAG_N | FLAG_Z | FLAG_V | FLAG_C ); \
                  SETF( g_byTestTable[ A ] | \
                        ( ( byD0 & A & 0x80 ) ? FLAG_V : 0 ) | \
                        ( ( A & 0x80 ) ? FLAG_C : 0 ) ); }
#define NEGB    { byD0 = B; B = ~B + 1; \
                  RSTF( FLAG_N | FLAG_Z | FLAG_V | FLAG_C ); \
                  SETF( g_byTestTable[ B ] | \
                        ( ( byD0 & B & 0x80 ) ? FLAG_V : 0 ) | \
                        ( ( B & 0x80 ) ? FLAG_C : 0 ) ); }
#define NEG(a)  { wA0 = (a); byD0 = M6809_Read( wA0 ); byD1 = ~byD0 + 1; \
                  M6809_Write( wA0, byD1 ); \
                  RSTF( FLAG_N | FLAG_Z | FLAG_V | FLAG_C ); \
                  SETF( g_byTestTable[ byD1 ] | \
                        ( ( byD0 & byD1 & 0x80 ) ? FLAG_V : 0 ) | \
                        ( ( byD1 & 0x80 ) ? FLAG_C : 0 ) ); }

/* TST */
#define TSTA    { RSTF( FLAG_N | FLAG_Z | FLAG_V ); SETF(  g_byTestTable[ A ] ); }
#define TSTB    { RSTF( FLAG_N | FLAG_Z | FLAG_V ); SETF(  g_byTestTable[ B ] ); }
#define TST(a)  { byD0 = M6809_Read( a ); RSTF( FLAG_N | FLAG_Z | FLAG_V ); \
                  SETF( g_byTestTable[ byD0 ] ); }

/* CMP */
#define CMPA(a) { byD0 = (a); wD0 = (WORD)A - byD0; byD1 = wD0 & 0xff; \
                  RSTF( FLAG_N | FLAG_Z | FLAG_V | FLAG_C ); \
                  SETF( g_byTestTable[ byD1 ] | \
                        ( ( ( A ^ byD0 ) & ( A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | \
                        ( ( A < byD0 ) ? FLAG_C : 0 ) ); }
#define CMPB(a) { byD0 = (a); wD0 = (WORD)B - byD0; byD1 = wD0 & 0xff; \
                  RSTF( FLAG_N | FLAG_Z | FLAG_V | FLAG_C ); \
                  SETF( g_byTestTable[ byD1 ] | \
                        ( ( ( B ^ byD0 ) & ( B ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | \
                        ( ( B < byD0 ) ? FLAG_C : 0 ) ); }
#define CMPD(a) { wD0 = (a); \
                  dwD0 = D - wD0; \
                  wD1 = (WORD)dwD0; \
                  RSTF( FLAG_N | FLAG_Z | FLAG_V | FLAG_C ); \
                  SETF( ( wD1 & 0x8000 ? FLAG_N : 0 ) | \
                        ( wD1 == 0 ? FLAG_Z : 0 ) | \
                        ( ( ( D ^ wD0 ) & ( D ^ wD1 ) & 0x8000 ) ? FLAG_V : 0 ) | \
                        ( ( D < wD0 ) ? FLAG_C : 0 ) ); }
#define CMPS(a) { wD0 = (a); \
                  dwD0 = S - wD0; \
                  wD1 = (WORD)dwD0; \
                  RSTF( FLAG_N | FLAG_Z | FLAG_V | FLAG_C ); \
                  SETF( ( wD1 & 0x8000 ? FLAG_N : 0 ) | \
                        ( wD1 == 0 ? FLAG_Z : 0 ) | \
                        ( ( ( S ^ wD0 ) & ( S ^ wD1 ) & 0x8000 ) ? FLAG_V : 0 ) | \
                        ( ( S < wD0 ) ? FLAG_C : 0 ) ); }
#define CMPU(a) { wD0 = (a); \
                  dwD0 = U - wD0; \
                  wD1 = (WORD)dwD0; \
                  RSTF( FLAG_N | FLAG_Z | FLAG_V | FLAG_C ); \
                  SETF( ( wD1 & 0x8000 ? FLAG_N : 0 ) | \
                        ( wD1 == 0 ? FLAG_Z : 0 ) | \
                        ( ( ( U ^ wD0 ) & ( U ^ wD1 ) & 0x8000 ) ? FLAG_V : 0 ) | \
                        ( ( U < wD0 ) ? FLAG_C : 0 ) ); }
#define CMPX(a) { wD0 = (a); \
                  dwD0 = X - wD0; \
                  wD1 = (WORD)dwD0; \
                  RSTF( FLAG_N | FLAG_Z | FLAG_V | FLAG_C ); \
                  SETF( ( wD1 & 0x8000 ? FLAG_N : 0 ) | \
                        ( wD1 == 0 ? FLAG_Z : 0 ) | \
                        ( ( ( X ^ wD0 ) & ( X ^ wD1 ) & 0x8000 ) ? FLAG_V : 0 ) | \
                        ( ( X < wD0 ) ? FLAG_C : 0 ) ); }
#define CMPY(a) { wD0 = (a); \
                  dwD0 = Y - wD0; \
                  wD1 = (WORD)dwD0; \
                  RSTF( FLAG_N | FLAG_Z | FLAG_V | FLAG_C ); \
                  SETF( ( wD1 & 0x8000 ? FLAG_N : 0 ) | \
                        ( wD1 == 0 ? FLAG_Z : 0 ) | \
                        ( ( ( Y ^ wD0 ) & ( Y ^ wD1 ) & 0x8000 ) ? FLAG_V : 0 ) | \
                        ( ( Y < wD0 ) ? FLAG_C : 0 ) ); }

/* DAA */
#define DAA     { byD0 = 0x00; \
                  if ( ( A & 0x0f ) > 0x09 || A_H ) { byD0 |= 0x06; } \
                  if ( ( ( A + byD0 ) & 0xf0 ) > 0x90 || A_C ) { byD0 |= 0x60; } \
                  wD0 = A + byD0; byD1 = (BYTE)wD0; \
                  RSTF( FLAG_N | FLAG_Z ); \
                  SETF( g_byTestTable[ byD1 ] | ( ( wD0 > 0xff ) ? FLAG_C : 0 ) ); \
                  A = byD1; }

/* SEX : Sign EXtend */
#define SEX     { A = ( B & 0x80 ) ? 0xff : 0x00; TEST( A ); }

/* ABX */
#define ABX     { X += SIGNED( B ); }

/* MUL */ 
#define MUL     { wD0 = (WORD)A * (WORD)B; \
                  RSTF( FLAG_Z | FLAG_C ); \
                  SETF( ( wD0 == 0 ? FLAG_Z : 0 ) | ( wD0 & 0x80 ? FLAG_C : 0 ) ); \
                  S_D( wD0 ); }

/* Shift Op. */
#define ASLA    { RSTF( FLAG_N | FLAG_Z | FLAG_V | FLAG_C ); \
                  SETF( g_ASLTable[ A ].byFlag ); A = g_ASLTable[ A ].byValue; }
#define ASLB    { RSTF( FLAG_N | FLAG_Z | FLAG_V | FLAG_C ); \
                  SETF( g_ASLTable[ B ].byFlag ); B = g_ASLTable[ B ].byValue; }
#define ASL(a)  { wA0 = (a); byD0 = M6809_Read( wA0 ); \
                  RSTF( FLAG_N | FLAG_Z | FLAG_V | FLAG_C ); \
                  SETF( g_ASLTable[ byD0 ].byFlag ); \
                  M6809_Write( wA0, g_ASLTable[ byD0 ].byValue ); }

#define ASRA    { RSTF( FLAG_N | FLAG_Z | FLAG_C ); \
                  SETF( g_ASRTable[ A ].byFlag ); A = g_ASRTable[ A ].byValue; }
#define ASRB    { RSTF( FLAG_N | FLAG_Z | FLAG_C ); \
                  SETF( g_ASRTable[ B ].byFlag ); B = g_ASRTable[ B ].byValue; }
#define ASR(a)  { wA0 = (a); byD0 = M6809_Read( wA0 ); \
                  RSTF( FLAG_N | FLAG_Z | FLAG_C ); \
                  SETF( g_ASRTable[ byD0 ].byFlag ); \
                  M6809_Write( wA0, g_ASRTable[ byD0 ].byValue ); }

#define LSRA    { RSTF( FLAG_N | FLAG_Z | FLAG_C ); \
                  SETF( g_LSRTable[ A ].byFlag ); A = g_LSRTable[ A ].byValue; }
#define LSRB    { RSTF( FLAG_N | FLAG_Z | FLAG_C ); \
                  SETF( g_LSRTable[ B ].byFlag ); B = g_LSRTable[ B ].byValue; }
#define LSR(a)  { wA0 = (a); byD0 = M6809_Read( wA0 ); \
                  RSTF( FLAG_N | FLAG_Z | FLAG_C ); \
                  SETF( g_LSRTable[ byD0 ].byFlag ); \
                  M6809_Write( wA0, g_LSRTable[ byD0 ].byValue ); }

#define ROLA    { byD0 = A_C; RSTF( FLAG_N | FLAG_Z | FLAG_C ); \
                  SETF( g_ROLTable[ byD0 ][ A ].byFlag ); \
                  A = g_ROLTable[ byD0 ][ A ].byValue; }
#define ROLB    { byD0 = A_C; RSTF( FLAG_N | FLAG_Z | FLAG_C ); \
                  SETF( g_ROLTable[ byD0 ][ B ].byFlag ); \
                  B = g_ROLTable[ byD0 ][ B ].byValue; }
#define ROL(a)  { byD1 = A_C; RSTF( FLAG_N | FLAG_Z | FLAG_C ); \
                  wA0 = (a); byD0 = M6809_Read( wA0 ); \
                  SETF( g_ROLTable[ byD1 ][ byD0 ].byFlag ); \
                  M6809_Write( wA0, g_ROLTable[ byD1 ][ byD0 ].byValue ); }

#define RORA    { byD0 = A_C; RSTF( FLAG_N | FLAG_Z | FLAG_C ); \
                  SETF( g_RORTable[ byD0 ][ A ].byFlag ); \
                  A = g_RORTable[ byD0 ][ A ].byValue; }
#define RORB    { byD0 = A_C; RSTF( FLAG_N | FLAG_Z | FLAG_C ); \
                  SETF( g_RORTable[ byD0 ][ B ].byFlag ); \
                  B = g_RORTable[ byD0 ][ B ].byValue; }
#define ROR(a)  { byD1 = A_C; RSTF( FLAG_N | FLAG_Z | FLAG_C ); \
                  wA0 = (a); byD0 = M6809_Read( wA0 ); \
                  SETF( g_RORTable[ byD1 ][ byD0 ].byFlag ); \
                  M6809_Write( wA0, g_RORTable[ byD1 ][ byD0 ].byValue ); }

/* Push & Pull Op. */
#define PSHS(a) { M6809_Write( --S, (a) ); }
#define PSHU(a) { M6809_Write( --U, (a) ); }
#define PSHSW(a) { S -= 2; M6809_WriteW( S, (a) ); }
#define PSHUW(a) { U -= 2; M6809_WriteW( U, (a) ); }

#define PULS(a) { a = M6809_Read( S++ ); }
#define PULU(a) { a = M6809_Read( U++ ); }
#define PULSW(a) { a = M6809_ReadW( S ); S += 2; }
#define PULUW(a) { a = M6809_ReadW( U ); U += 2; }

/* Branch Op. */
#define BRA(a)  { if ( a ) { byD0 = M6809_Read( PC++ ); PC += SIGNED( byD0 ); } \
                  else { ++PC; } } 

#define LBRA(a) { if ( a ) { \
                    wD0 = M6809_ReadW( PC ); PC += 2; \
                    PC += wD0; CLK( 6 ); } else { PC += 2; CLK( 5 ); } }
#define JMP(a)  { PC = a; }
#define BSR     { byD0 = M6809_Read( PC++ ); PSHSW( PC ); \
                  PC += SIGNED( byD0 ); }

#define LBSR    { wD0 = M6809_ReadW( PC ); PC += 2; PSHSW( PC ); \
                  PC += wD0; }
#define JSR(a)  { wA0 = (a); PSHSW( PC ); PC = wA0; }
#define RTS     { PULSW( PC ); }
#define RTI     { PULS( F ); \
                  if ( F & FLAG_E ) { PULS( A ); PULS( B ); PULS( DP ); \
                                      PULSW( X ); PULSW( Y ); PULSW( U ); \
                                      CLK( 9 ); } \
                  PULSW( PC ); }

/* Exchange & Transfer Op. */
#define EXG     { PB = M6809_Read( PC++ ); \
                  PB_RRD( wD0, PB_R1 ); PB_RRD( wD1, PB_R2 ); \
                  PB_RWT( PB_R2, wD0 ); PB_RWT( PB_R1, wD1 ); }
#define TFR     { PB = M6809_Read( PC++ ); \
                  PB_RRD( wD0, PB_R1 ); PB_RWT( PB_R2, wD0 ); }

/* Interrupt Op. */
#define PSHSREG { PSHSW( PC ); PSHSW( U ); PSHSW( Y ); PSHSW( X ); \
                  PSHS( DP ); PSHS( B ); PSHS( A ); PSHS( F ); }
#define SYNC    { INT_State = INT_SYNC; }
#define CWAI(a) { F &= (a); SETF( FLAG_E ); PSHSREG; INT_State = INT_CWAI; }
#define SWI     { SETF( FLAG_E );  PSHSREG; SETF( FLAG_F | FLAG_I ); \
                  PC = VECTOR_SWI; }
#define SWI2    { SETF( FLAG_E ); PSHSREG; PC = VECTOR_SWI2; }
#define SWI3    { SETF( FLAG_E ); PSHSREG; PC = VECTOR_SWI3; }

/* dummy */

#if 0
#define LDX(a)  { X = (a); TESTW( X ); }
#define LDY(a)  { Y = (a); TESTW( Y ); }
#define LDU(a)  { U = (a); TESTW( U ); }
#define LDS(a)  { S = (a); TESTW( S ); }
#endif

#endif /* !M6809_H_INCLUDED */
