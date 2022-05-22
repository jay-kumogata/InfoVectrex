/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/
#include <windows.h>
#include <wingdi.h>
#include <stdio.h>
#include "../System.h"
#include "../Memory.h"
#include "../M6809.h"
#include "../M6522.h"
#include "../VG.h"
#include "../SG.h"

int bDebug = 0;
int nFrame;
int nFrameDebug = 30000;
#define FRAME_END              2300

/*-------------------------------------------------------------------*/
/*  Variables for Windows                                            */
/*-------------------------------------------------------------------*/
#define APP_NAME     "InfoVectrex v0.31J"

#define MENU_WIDTH   8
#define MENU_HEIGHT  34

HWND hWnd;
byte *pScreenMem;
HBITMAP hScreenBmp;
LOGPALETTE *plpal;
BITMAPINFO *bmi;

//WNDCLASS wc;
//HACCEL hAccel;
HDC hDC;
HPEN hPen, hPen0, hPen1, hPen2;
HBRUSH hBrush;

/* Variables for Emulation Thread */
HANDLE m_hThread;
DWORD m_ThreadID = 0;

/* Variables for Drawing Screen */
_Line l[ MAX_LINE ];
int nL;

/*-------------------------------------------------------------------*/
/*  Function prototypes ( Windows specific )                         */
/*-------------------------------------------------------------------*/
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
int Init( void );
int Fin( void );
int CreateScreen( void );
void DestroyScreen( void );

/*-------------------------------------------------------------------*/
/*  Main Routine                                                     */
/*-------------------------------------------------------------------*/
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, \
		    LPSTR lpCmdLine, int nCmdShow )
{	
  /*-------------------------------------------------------------------*/
  /*  Create a window                                                  */
  /*-------------------------------------------------------------------*/

  WNDCLASSEX wcex; 
 
  wcex.cbSize = sizeof(wcex);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
  wcex.lpszMenuName = NULL;
  wcex.lpszClassName = "INFOVECTREX";
  wcex.hIconSm = NULL;
  
  if (!RegisterClassEx(&wcex))
    return FALSE; 

  hWnd = CreateWindow("INFOVECTREX", APP_NAME, WS_OVERLAPPEDWINDOW,
			  CW_USEDEFAULT, CW_USEDEFAULT,
			  DISP_WIDTH + MENU_WIDTH,
			  DISP_HEIGHT + MENU_HEIGHT,
			  NULL, NULL, hInstance, NULL);

  if (!hWnd)
    return FALSE;

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  /*-------------------------------------------------------------------*/
  /*  Init Resources                                                   */
  /*-------------------------------------------------------------------*/
  Init();

  /*-------------------------------------------------------------------*/
  /*  Main Loop                                                        */
  /*-------------------------------------------------------------------*/

  /* Create Emulation Thread */
  m_hThread = CreateThread( (LPSECURITY_ATTRIBUTES)NULL, (DWORD)0,
			    (LPTHREAD_START_ROUTINE)InfoVectrex_Main, 
			    (LPVOID)NULL, (DWORD)0, &m_ThreadID);

  /*-------------------------------------------------------------------*/
  /*  The Message Pump                                                 */
  /*-------------------------------------------------------------------*/
  /* Message Pump */
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return msg.wParam;
}

/*-------------------------------------------------------------------*/
/*  Callback Function                                                */
/*-------------------------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT ps;
	
  switch (message) {
  case WM_DESTROY:
    Fin();
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

/*-------------------------------------------------------------------*/
/*  Initialize Resources                                             */
/*-------------------------------------------------------------------*/
int Init()
{
  /* Create Screen */
  CreateScreen();

  /* Create Device Context */
  hDC = GetDC( hWnd );

  /* Create handle of pen */
  hPen0 = CreatePen(PS_SOLID, 0, RGB( 255, 255, 255 ) );
  hPen1 = CreatePen(PS_SOLID, 1, RGB( 255, 255, 255 ) );
  hPen2 = CreatePen(PS_SOLID, 2, RGB( 255, 255, 255 ) );
  hPen  = hPen0;

  /* Create handle of brush */ 
  hBrush = CreateSolidBrush( RGB( 0, 0, 0 ) );

  /* Release Device Context */
  //  ReleaseDC( hWnd, hDC );

  /* Initialize counter of line */
  nL = 0;

  return 0;
}

/*-------------------------------------------------------------------*/
/*  Emulation Main Routine                                           */
/*-------------------------------------------------------------------*/
int InfoVectrex_Main(void)
{
  /* Initialize */
  M6809_Init();

  /* Load rom image */
  if ( InfoVectrex_LoadRom( "./rom.dat" ) < 0 )
      return -1;

  /* Reset */
  M6809_Reset();
  M6522_Reset();
  VG_Reset();
  SG_Reset();

  /* Execute */
#ifdef DEBUG
  //  for ( nFrame = 0; nFrame < FRAME_END; nFrame++ ) {
  for ( ;; ) {
    VG_Refresh();
  }
#else
  for ( ;; ) {
    VG_Refresh();
  }
#endif

  /* Finalize */
    return 0;
}

/*-------------------------------------------------------------------*/
/*  Finalize Resources                                               */
/*-------------------------------------------------------------------*/
int Fin()
{
  /* Terminate Emulation Thread */

  /* Release Device Context */
  ReleaseDC( hWnd, hDC );

  /* Destroy Screen */
  DestroyScreen();

  return 0;
}

/*-------------------------------------------------------------------*/
/*  Create a InfoChip8 screen                                        */
/*-------------------------------------------------------------------*/
int CreateScreen( void )
{
  HDC hDC = GetDC( hWnd );
  
  BITMAPINFOHEADER bi;
  
  bi.biSize = sizeof( BITMAPINFOHEADER );
  bi.biWidth = DISP_WIDTH;
  bi.biHeight = DISP_HEIGHT * -1;
  bi.biPlanes = 1;
  bi.biBitCount = 16;
  bi.biCompression = BI_RGB;
  bi.biSizeImage = DISP_WIDTH * DISP_HEIGHT * 2;
  bi.biXPelsPerMeter = 0;
  bi.biYPelsPerMeter = 0;
  bi.biClrUsed = 0;
  bi.biClrImportant = 0;
  
  hScreenBmp = CreateDIBSection( hDC, 
                                 (BITMAPINFO *)&bi,
                                 DIB_RGB_COLORS, 
                                 (void **)&pScreenMem, 
                                 0,
                                 0 ); 
  ReleaseDC( hWnd, hDC );

  if ( !hScreenBmp ) { return -1; } 
  else {  return 0; }
}

/*-------------------------------------------------------------------*/
/*  Destroy InfoChip8 screen                                         */
/*-------------------------------------------------------------------*/
void DestroyScreen()
{
  if ( !hScreenBmp ) { DeleteObject( hScreenBmp ); }
}

/*-------------------------------------------------------------------*/
/*  Load ROM image                                                   */
/*-------------------------------------------------------------------*/
int InfoVectrex_LoadRom( const char *pszFileName )
{
  int len;
  FILE *fp;

  /* Open ROM file */
  fp = fopen( pszFileName, "rb" );
  if ( fp == NULL )
    return -1;

  /* Load ROM Image to Memory */
  len = fread (ROM, 1, sizeof (ROM), fp );

  /* File close */
  fclose( fp );

  if ( len < sizeof ( ROM ) ) {
    printf ( "read %d bytes from '%s'. need %d bytes.",
	     len, pszFileName, sizeof (ROM));
    return -1;
  }

  /* Successful */
  return 0;
}

#if 0
/*-------------------------------------------------------------------*/
/*  Draw Line                                                        */
/*-------------------------------------------------------------------*/
int InfoVectrex_DrawLine( int x1, int y1, int x2, int y2, int z )
{
  //  printf("%d %d %d %d %d\n",x1,y1,x2,y2,z);

  MoveToEx( hDC, x1 / 100, y1 / 100, NULL );
  LineTo( hDC, x2 / 100, y2 / 100 );

  return 0;
}
#endif

/*-------------------------------------------------------------------*/
/*  Update Screen                                                    */
/*-------------------------------------------------------------------*/
int InfoVectrex_UpdateScreen( void )
{
  /* Drawing Screen */
  HDC hMemDC = CreateCompatibleDC( hDC );
  HBITMAP hOldBmp = (HBITMAP)SelectObject( hMemDC, hScreenBmp );

  SelectObject( hMemDC, hPen );
  SelectObject( hMemDC, hBrush );

  Rectangle( hMemDC, 0, 0, DISP_WIDTH, DISP_HEIGHT );

  for ( int n = 0; n < nL; n++ ) {
    MoveToEx( hMemDC, l[n].x1 / 100, l[n].y1 / 100, NULL );
    LineTo( hMemDC, l[n].x2 / 100, l[n].y2 / 100 );
  }
  nL = 0;

  StretchBlt( hDC, 0, 0, DISP_WIDTH, DISP_HEIGHT, hMemDC, 
              0, 0, DISP_WIDTH, DISP_HEIGHT, SRCCOPY );

  SelectObject( hMemDC, hOldBmp );

  DeleteDC( hMemDC );

  /* Joystick #0 : Button */
#if 1
 SG_R[ 0xe ] = ( SG_R[ 0xe ] & 0xf0 ) |
    ( ( ( GetAsyncKeyState( 'A' ) < 0 ) ? 0 : 1 ) << 0 ) |
    ( ( ( GetAsyncKeyState( 'S' ) < 0 ) ? 0 : 1 ) << 1 ) |
    ( ( ( GetAsyncKeyState( 'D' ) < 0 ) ) ? 0 : 1 << 2 ) |
    ( ( ( GetAsyncKeyState( 'F' ) < 0 ) ) ? 0 : 1 << 3 );
#else 
 if ( GetAsyncKeyState( 'A' ) < 0 ) { V_RST( SG_R[ 0xe ], 0x01 ); }
 else                               { V_SET( SG_R[ 0xe ], 0x01 ); }
 if ( GetAsyncKeyState( 'S' ) < 0 ) { V_RST( SG_R[ 0xe ], 0x02 ); }
 else                               { V_SET( SG_R[ 0xe ], 0x02 ); }
 if ( GetAsyncKeyState( 'D' ) < 0 ) { V_RST( SG_R[ 0xe ], 0x04 ); }
 else                               { V_SET( SG_R[ 0xe ], 0x04 ); }
 if ( GetAsyncKeyState( 'F' ) < 0 ) { V_RST( SG_R[ 0xe ], 0x08 ); }
 else                               { V_SET( SG_R[ 0xe ], 0x08 ); }
#endif

  /* Joystick #0 : Stick */
  if      ( GetAsyncKeyState( VK_LEFT ) < 0 )  { VG_JCH0 = 0x00; }
  else if ( GetAsyncKeyState( VK_RIGHT ) < 0 ) { VG_JCH0 = 0xff; }
  else                                         { VG_JCH0 = 0x80; }
  if      ( GetAsyncKeyState( VK_UP ) < 0 )    { VG_JCH1 = 0x00; }
  else if ( GetAsyncKeyState( VK_DOWN ) < 0 )  { VG_JCH1 = 0xff; }
  else                                         { VG_JCH1 = 0x80; }

  /* Pen select */
  if ( GetAsyncKeyState( '0' ) < 0 )  { hPen = hPen0; }
  if ( GetAsyncKeyState( '1' ) < 0 )  { hPen = hPen1; }
  if ( GetAsyncKeyState( '2' ) < 0 )  { hPen = hPen2; }
}
