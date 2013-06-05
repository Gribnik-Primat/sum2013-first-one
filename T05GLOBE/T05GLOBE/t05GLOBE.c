/* FILE NAME: T03DBLBF.C
 * PROGRAMMER: VG4
 * DATE: 03.06.2013
 * PURPOSE: WinAPI double buffered output.
 */

#include <windows.h>

#include <time.h>

#include <math.h>

#include <stdio.h>

#define WND_CLASS_NAME "My window class"

/* ������ ������ */
LRESULT CALLBACK MyWindowFunc( HWND hWnd, UINT Msg,
                               WPARAM wParam, LPARAM lParam );

/* ������� ������� ���������.
 * ���������:
 *   - ���������� ���������� ����������:
 *       HINSTANCE hInstance;	
 *   - ���������� ����������� ���������� ����������
 *     (�� ������������ � ������ ���� NULL):
 *       HINSTANCE hPrevInstance;
 *   - ��������� ������:
 *       CHAR *CmdLine;
 *   - ���� ������ ����:
 *       INT ShowCmd;
 * ������������ ��������:
 *   (INT) ��� �������� � ������������ �������.
 */
INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    CHAR *CmdLine, INT ShowCmd )
{
  WNDCLASS wc;
  MSG msg;
  HWND hWnd;

  
  
  wc.style = CS_VREDRAW | CS_HREDRAW; /* ����� ����: ��������� ��������������
                                       * ��� ��������� ������������� ���
                                       * ��������������� �������� */
  wc.cbClsExtra = 0; /* �������������� ���������� ���� ��� ������ */
  wc.cbWndExtra = 0; /* �������������� ���������� ���� ��� ���� */
  wc.hbrBackground = (HBRUSH)COLOR_WINDOW; /* ������� ���� - ��������� � ������� */
  wc.hCursor = LoadCursor(NULL, IDC_ARROW); /* �������� ������� (����������) */
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION); /* �������� ����������� (���������) */
  wc.lpszMenuName = NULL; /* ��� ������� ���� */
  wc.hInstance = hInstance; /* ���������� ����������, ��������������� ����� */
  wc.lpfnWndProc = MyWindowFunc; /* ��������� �� ������� ��������� */
  wc.lpszClassName = WND_CLASS_NAME; /* ��� ������ */

  /* ����������� ������ � ������� */
  if (!RegisterClass(&wc))
  {
    MessageBox(NULL, "Error register window class", "ERROR", MB_OK);
    return 0;
  }

  hWnd = CreateWindowA(WND_CLASS_NAME, "> 30 <",
    WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN,
    10, 10, 1024, 1024, NULL, NULL, hInstance, NULL);

  while (GetMessage(&msg, NULL, 0, 0))
    DispatchMessage(&msg);

  return msg.wParam;
} /* End of 'WinMain' function */

/* ������� ��������� ��������� ����.
 * ���������:
 *   - ���������� ����:
 *       HWND hWnd;
 *   - ����� ���������:
 *       UINT Msg;
 *   - �������� ��������� ('word parameter'):
 *       WPARAM wParam;
 *   - �������� ��������� ('long parameter'):
 *       LPARAM lParam;
 * ������������ ��������:
 *   (LRESULT) - � ����������� �� ���������.
 */




LRESULT CALLBACK MyWindowFunc( HWND hWnd, UINT Msg,
                               WPARAM wParam, LPARAM lParam )
{
  
  INT x, y,i,j,w=0,h=0;
  double theta,phi,phaseshift=0;
  HDC hDC;
  HPEN hPen, hOldPen;
  HBRUSH hBr, hOldBr;
  CREATESTRUCT *cs;
  PAINTSTRUCT ps;
  POINT pt;
  BITMAP Bm;
  FILE *F;
  BYTE *mem,r,g,b;
  static INT WinW, WinH;
  static HDC hMemDCFrame, hMemDC;
  static HBITMAP hBmFrame, hBmBack, hBmAnd, hBmXor;

  /*F=fopen("GLOBE.G24","rb");
  fread(&w,2,1,F);
  fread(&h,2,1,F);
  mem = malloc(w*h*3);
  fread(mem,3,w*h,F);
  fclose(F);  */
                     
  
  SYSTEMTIME st;

  switch (Msg)
  {
  case WM_CREATE:
    cs = (CREATESTRUCT *)lParam;
    SetTimer(hWnd, 30, 5, NULL);
                                                                      
                          

    /*** ������� �������� � ������ ***/
    /* ���������� ������� ���� */
    hDC = GetDC(hWnd);
    /* ������� �������� � ������, ����������� � ���������� ���� */
    hMemDCFrame = CreateCompatibleDC(hDC);
    hMemDC = CreateCompatibleDC(hDC);
    SetPixelV(hMemDC, 100, 100, RGB(0, 0, 0));
    /* ������� �������� ���� */
    ReleaseDC(hWnd, hDC);

    return 0;
  case WM_SIZE:
    WinW = LOWORD(lParam);
    WinH = HIWORD(lParam);

    /*** �������� �������� (��������� ������) ***/
    if (hBmFrame != NULL)
      DeleteObject(hBmFrame);
    /* ������� � ������ �����������, ����������� � ��������� ���� */
    hDC = GetDC(hWnd);
    hBmFrame = CreateCompatibleBitmap(hDC, WinW, WinH);
    ReleaseDC(hWnd, hDC);
    /* �������� �� � �������� ������ */
    SelectObject(hMemDCFrame, hBmFrame);

    /* ��������� ��������� � ����� �������� */
    SendMessage(hWnd, WM_TIMER, 0, 0);
    return 0;

  case WM_TIMER:
    /* ���������� ����� ����� */
    GetCursorPos(&pt);
    ScreenToClient(hWnd, &pt);

    GetLocalTime(&st);

    SelectObject(hMemDCFrame, GetStockObject(DC_BRUSH));
    SelectObject(hMemDCFrame, GetStockObject(NULL_PEN));

    SetDCBrushColor(hMemDCFrame, RGB(0, 0, 0));
    Rectangle(hMemDCFrame, 0, 0, WinW + 1, WinH + 1);
    SelectObject(hMemDCFrame, GetStockObject(DC_PEN));
    SetDCPenColor(hMemDCFrame, RGB(255, 250, 250));

    for(i = 0; i <= WinW;i++)
      for(j = 0;j < WinH;j++)
      {    
        theta = i*3.14/WinW;
        phi = j*2*3.14/WinH; 
      
        x = (INT) 500*sin(theta)*cos(phi);
        y = (INT) 500*cos(theta);

        if(x>0) 
        {         
          SetPixel(hMemDCFrame,x+512,y+512,RGB(255,255,255));
        }
      }

  
   
    /* ����� ����������� */
    InvalidateRect(hWnd, NULL, FALSE);
    return 0;
  case WM_ERASEBKGND:
    return 0;
  case WM_PAINT:

    /* ��������� */
    hDC = BeginPaint(hWnd, &ps);
    BitBlt(hDC, 0, 0, WinW, WinH, hMemDCFrame, 0, 0, SRCCOPY);
    EndPaint(hWnd, &ps);
    return 0;
  case WM_DESTROY:
    /* ���������� �������� � ������ � �������� */
    if (hBmFrame != NULL)
      DeleteObject(hBmFrame);
    if (hMemDCFrame != NULL)
      DeleteDC(hMemDCFrame);

    /* ������� ��������� 'WM_QUIT' � ���������� 'wParam' 0 - ��� ��������,
     * ������ �����: PostMessage(hWnd, WM_QUIT, ���_��������, 0); */
    PostQuitMessage(30);
    KillTimer(hWnd, 30);
    return 0;
  }
  return DefWindowProc(hWnd, Msg, wParam, lParam);
} /* End of 'MyWindowFunc' function */


/* END OF 'T03DBLBF.C' FILE */
