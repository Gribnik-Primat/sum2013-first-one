/* FILE NAME: T03DBLBF.C
 * PROGRAMMER: VG4
 * DATE: 03.06.2013
 * PURPOSE: WinAPI double buffered output.
 */
#include "VEC.h"

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
    10, 10, 750, 750, NULL, NULL, hInstance, NULL);


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

VEC *Vertexes, *VertexesProj;
INT NumOfVertexes;

/* Facet array */
INT (*Facets)[3];
INT NumOfFacets;


VOID LoadCow( VOID )
{
  FILE *F;
  INT fn = 0, vn = 0;
  static CHAR Buf[1000];

  if ((F = fopen("cow_new1.object", "rt")) == NULL)
    return;

  while (fgets(Buf, sizeof(Buf), F) != NULL)
    if (Buf[0] == 'v' && Buf[1] == ' ')
      vn++;
    else if (Buf[0] == 'f' && Buf[1] == ' ')
      fn++;

  if ((Vertexes = malloc(2 * sizeof(VEC) * vn)) == NULL)
  {
    fclose(F);
    return;
  }
  if ((Facets = malloc(sizeof(INT [3]) * fn)) == NULL)
  {
    free(Vertexes);
    fclose(F);
    return;
  }
  NumOfVertexes = vn;
  NumOfFacets = fn;
  VertexesProj = Vertexes + NumOfVertexes;

  vn = 0;
  fn = 0;
  rewind(F);
  while (fgets(Buf, sizeof(Buf), F) != NULL)
    if (Buf[0] == 'v' && Buf[1] == ' ')
    {
      DBL x, y, z;

      sscanf(Buf + 2, "%lf%lf%lf", &x, &y, &z);
      Vertexes[vn].X = x;
      Vertexes[vn].Y = y;
      Vertexes[vn].Z = z;
      vn++;
    }
    else if (Buf[0] == 'f' && Buf[1] == ' ')
    {
      INT n1, n2, n3;

      sscanf(Buf + 2, "%d%d%d", &n1, &n2, &n3);
      Facets[fn][0] = n1 - 1;
      Facets[fn][1] = n2 - 1;
      Facets[fn][2] = n3 - 1;
      fn++;
    }

  fclose(F);
}


LRESULT CALLBACK MyWindowFunc( HWND hWnd, UINT Msg,
                               WPARAM wParam, LPARAM lParam )
{
  
  INT x,y,z,i,j,w=0,h=0;
  DOUBLE scale = 1,tim;
  double theta,phi,phaseshift=0;
  HDC hDC;
  HPEN hPen, hOldPen;
  HBRUSH hBr, hOldBr;
  CREATESTRUCT *cs;
  PAINTSTRUCT ps;
  POINT p[3];
  BITMAP Bm;
  BYTE r,g,b;
  static INT WinW, WinH;
  static HDC hMemDCFrame, hMemDC;
  static HBITMAP hBmFrame, hBmBack, hBmAnd, hBmXor;
  SYSTEMTIME st;
  COLORREF c;

  LoadCow();

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


    GetLocalTime(&st);

    
    SelectObject(hMemDCFrame, GetStockObject(DC_BRUSH));
    SelectObject(hMemDCFrame, GetStockObject(NULL_PEN));

    SetDCBrushColor(hMemDCFrame, RGB(0, 0, 0));
    Rectangle(hMemDCFrame, 0, 0, WinW + 1, WinH + 1);
    SelectObject(hMemDCFrame, GetStockObject(DC_PEN));
    SetDCPenColor(hMemDCFrame, RGB(255, 250, 250));

    
    
    tim = clock();
    SelectObject(hMemDCFrame, GetStockObject(NULL_BRUSH));
    SelectObject(hMemDCFrame, GetStockObject(DC_PEN));
    SetDCPenColor(hMemDCFrame, RGB(255, 255, 255));
    SetDCBrushColor(hMemDCFrame, RGB(0, 111, 0));
    scale = 59;
    /* calculate projection */
    for (i = 0; i < NumOfVertexes; i++)
    {
      DOUBLE a = tim * 0.00030;
      VertexesProj[i].X = WinW / 2 + scale * (Vertexes[i].X * cos(a) - Vertexes[i].Z * sin(a));
      VertexesProj[i].Y = WinH / 2 - scale * (Vertexes[i].Y);
      VertexesProj[i].Z = scale * (Vertexes[i].X * sin(a) + Vertexes[i].Z * cos(a));
    }

    for (i = 0; i < NumOfFacets; i++)
    {
      POINT p[3];

      for (j = 0; j < 3; j++)
      {
        p[j].x = VertexesProj[Facets[i][j]].X;
        p[j].y = VertexesProj[Facets[i][j]].Y;
      }

      Polygon(hMemDCFrame, p, 3);
    }

    for (i = 0; i < NumOfVertexes; i++)
    {
      
        x = VertexesProj[i].X;
        y = VertexesProj[i].Y;
      SetPixelV(hMemDCFrame, x, y, RGB(255, 255, 255));
    }

    
    SelectObject(hMemDCFrame, GetStockObject(NULL_PEN));
    
    SetDCBrushColor(hMemDC, RGB(0, 0, 0));
    Rectangle(hMemDC, 0, 0, WinW + 1, WinH + 1);
    SelectObject(hMemDC, GetStockObject(DC_PEN));
    SetDCPenColor(hMemDC, RGB(0, 0, 0));


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
