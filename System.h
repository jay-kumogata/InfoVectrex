#ifndef SYSTEM_H_INCLUDED
#define SYSTEM_H_INCLUDED

#include "Types.h"

#define DISP_WIDTH   ( VG_MAX_X / 100 )
#define DISP_HEIGHT  ( VG_MAX_Y / 100 )

extern int bDebug;
extern int nFrame;
extern int nFrameDebug;

typedef struct {
  int x1;
  int y1;
  int x2;
  int y2;
  int z;
} _Line;

#define MAX_LINE    40000
extern int nL;

/* Variables for Drawing Screen */
extern _Line l[ MAX_LINE ];

/*-------------------------------------------------------------------*/
/*  Function Prototype                                               */
/*-------------------------------------------------------------------*/
int InfoVectrex_Main( void );
int InfoVectrex_LoadRom( const char *pszFileName );
#if 0
int InfoVectrex_DrawLine( int x1, int y1, int x2, int y2, int z );
#endif
int InfoVectrex_UpdateScreen( void );

/*-------------------------------------------------------------------*/
/*  Macros                                                           */
/*-------------------------------------------------------------------*/
#define InfoVectrex_DrawLine(_x1,_y1,_x2,_y2,_z) \
{ \
  l[ nL ].x1 = (_x1); \
  l[ nL ].x2 = (_x2); \
  l[ nL ].y1 = (_y1); \
  l[ nL ].y2 = (_y2); \
  l[ nL ].z  = (_z); \
  nL++; \
}

#endif /* !SYSTEM_H_INCLUDED */
