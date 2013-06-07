/* FILE NAME: ANIM.C
 * PROGRAMMER: VG4
 * DATE: 06.06.2013
 * PURPOSE: Main animation implementation module.
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include "anim.h"
#include "vec.h"

/* Глобальные переменные анимации */
static BOOL VG4_IsInit;
static vg4ANIM Anim;

/* Массив активных объектов анимации */
#define VG4_MAX_UNITS 1000
static INT VG4_NumOfUnits;
static vg4UNIT *VG4_Units[VG4_MAX_UNITS];

/* Функция инициализации анимации.
 * АРГУМЕНТЫ:
 *   - рабочее окно анимации:
 *       HWND hWnd;
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ:
 *   (BOOL) TRUE при успехе, FALSE при невозможности инициализации.
 */
BOOL AnimInit( HWND hWnd )
{
  INT i;
  HDC hDC;

  LARGE_INTEGER li;
  
  QueryPerformanceFrequency (&li); 
  TimeFreq  = li.QuadPart;
  QueryPerformanceCounter(&li); 
  TimeStart = TimeOld = TimeFPS = li.QuadPart;
  TimePause = TimeShift = 0;
  FrameCounter = 0;
  
  Anim.hWnd = hWnd;
  /* определяем контект окна */
  hDC = GetDC(hWnd);
  /* создаем контекст в памяти, совместимый с контекстом окна */
  Anim.hDC  = CreateCompatibleDC(hDC);
  /* убираем контекст окна */
  ReleaseDC(hWnd, hDC);

  VG4_IsInit = TRUE;

  /* Инициализация всех зарегистрированных объектов анимации */
  for (i = 0; i < VG4_NumOfUnits; i++)
    VG4_Units[i]->Init(VG4_Units[i], &Anim);

  return TRUE;
} /* End of 'AnimInit' function */

/* Функция деинициализации анимации.
 * АРГУМЕНТЫ: Нет.
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ: Нет.
 */
VOID AnimClose( VOID )
{
  INT i;

  if (Anim.hDC != NULL)
    DeleteDC(Anim.hDC), Anim.hDC = NULL;
  if (Anim.hFrame != NULL)
    DeleteObject(Anim.hFrame), Anim.hFrame = NULL;

  /* Уничтожение всех зарегистрированных объектов анимации */
  for (i = 0; i < VG4_NumOfUnits; i++)
    VG4_Units[i]->Close(VG4_Units[i]);

  VG4_IsInit = FALSE;
} /* End of 'AnimClose' function */

/* Функция обработки изменение размера кадра.
 * АРГУМЕНТЫ:
 *   - новые размеры кадра:
 *       INT W, H;
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ: Нет.
 */
VOID AnimResize( INT W, INT H )
{
  BITMAPINFOHEADER bih;

  if (!VG4_IsInit)
    return;

  if (Anim.hFrame != NULL)
    DeleteObject(Anim.hFrame);

  /* создаем изображение заданной структуры */
  bih.biSize = sizeof(BITMAPINFOHEADER);
  bih.biWidth = Anim.W = W;
  bih.biHeight = -(Anim.H = H);
  bih.biPlanes = 1;    /* всегда */
  bih.biBitCount = 32; /* xRGB */
  bih.biCompression = BI_RGB;
  bih.biSizeImage = W * H * 4;
  bih.biClrImportant = 0;
  bih.biClrUsed = 0;
  bih.biXPelsPerMeter = bih.biYPelsPerMeter = 0;

  Anim.hFrame = CreateDIBSection(NULL, (BITMAPINFO *)&bih,
    DIB_RGB_COLORS, (VOID *)&Anim.Bits, NULL, 0);

  SelectObject(Anim.hDC, Anim.hFrame);

  /* Первичное построение кадра в новом размере */
  AnimRender();
} /* End of 'AnimResize' function */

/* Функция построения кадра.
 * АРГУМЕНТЫ: Нет.
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ: Нет.
 */
VOID AnimRender( VOID )
{
  INT i;
  static CHAR Buf[100];

  /* Опрос таймера */

  /* Опрос устройств ввода */
  LARGE_INTEGER li;
  
  QueryPerformanceCounter(&li);

  /* глобальное время */
  Anim.GlobalTime = (DBL)(li.QuadPart - TimeStart) / TimeFreq;
  Anim.GlobalDeltaTime = (DBL)(li.QuadPart - TimeOld) / TimeFreq;

  /* локальное время */
  if (Anim.IsPause)
    Anim.DeltaTime = 0, TimePause += li.QuadPart - TimeOld;
  else
    Anim.DeltaTime = Anim.GlobalDeltaTime;

  Anim.Time = (DBL)(li.QuadPart - TimeStart - TimePause - TimeShift) /
    TimeFreq;

  /* вычисляем производительность */
  if (TimeFPS > TimeFreq)
  {
    Anim.FPS = FrameCounter / ((DBL)(li.QuadPart - TimeFPS) / TimeFreq);
    TimeFPS = li.QuadPart;
    FrameCounter = 0;
  }
  /* обновляем время "старого" кадра */
  TimeOld = li.QuadPart;


  /* Посылка всем объектам анимации сигнала обработки
   * межкадрового взаимодействия */
  for (i = 0; i < VG4_NumOfUnits; i++)
    VG4_Units[i]->Response(VG4_Units[i], &Anim);

  /* Инициализация кадра */
   
  if (!VG4_IsInit)
    return;

  
  SelectObject(Anim.hDC, GetStockObject(DC_BRUSH));
  SelectObject(Anim.hDC, GetStockObject(NULL_PEN));
  SetDCBrushColor(Anim.hDC, RGB(50, 150, 200));
  Rectangle(Anim.hDC, 0, 0, Anim.W, Anim.H);

  TextOut(Anim.hDC,10, 10,Buf,sprintf(Buf,"%lf   %lf  (%lf)",Anim.FPS,Anim.GlobalTime,Anim.Time));

  /* Посылка всем объектам анимации сигнала перерисовки */

  
  for (i = 0; i < VG4_NumOfUnits; i++)
  {
    SelectObject(Anim.hDC, GetStockObject(DC_BRUSH));
    SelectObject(Anim.hDC, GetStockObject(DC_PEN));
    SetDCBrushColor(Anim.hDC, RGB(0, 0, 0));
    SetDCPenColor(Anim.hDC, RGB(255, 255, 255));

    VG4_Units[i]->Render(VG4_Units[i], &Anim);
  }

  FrameCounter += 1;


} /* End of 'AnimRender' function */

/* Функция копирования рабочего кадра в окно.
 * АРГУМЕНТЫ: Нет.
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ: Нет.
 */
VOID AnimCopyFrame( VOID )
{
  HDC hDC;

  if (!VG4_IsInit)
    return;
  /* Выводм кадр в окно анимации */
  hDC = GetDC(Anim.hWnd);
  BitBlt(hDC, 0, 0, Anim.W, Anim.H, Anim.hDC, 0, 0, SRCCOPY);
  ReleaseDC(Anim.hWnd, hDC);
} /* End of 'AnimCopyFrame' function */

/***
 * Функции объектов обработки по-умолчанию
 ***/

/* Функция инициализации объекта.
 * АРГУМЕНТЫ:
 *   - указатель на "себя":
 *       vg4UNIT *Unit;
 *   - указатель на систему анимации:
 *       vg4ANIM *Ani;
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ: Нет.
 */
static VOID VG4_DefUnitInit( vg4UNIT *Unit, vg4ANIM *Ani )
{
} /* End of 'VG4_DefUnitInit' function */

/* Функция деинициализации объекта.
 * АРГУМЕНТЫ:
 *   - указатель на "себя":
 *       vg4UNIT *Unit;
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ: Нет.
 */
static VOID VG4_DefUnitClose( vg4UNIT *Unit )
{
} /* End of 'VG4_DefUnitClose' function */

/* Функция реакции на межкадровое взаимодействие объекта.
 * АРГУМЕНТЫ:
 *   - указатель на "себя":
 *       vg4UNIT *Unit;
 *   - указатель на систему анимации:
 *       vg4ANIM *Ani;
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ: Нет.
 */
static VOID VG4_DefUnitResponse( vg4UNIT *Unit, vg4ANIM *Ani )
{
} /* End of 'VG4_DefUnitResponse' function */

/* Функция отрисовки объекта.
 * АРГУМЕНТЫ:
 *   - указатель на "себя":
 *       vg4UNIT *Unit;
 *   - указатель на систему анимации:
 *       vg4ANIM *Ani;
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ: Нет.
 */
static VOID VG4_DefUnitRender( vg4UNIT *Unit, vg4ANIM *Ani )
{
} /* End of 'VG4_DefUnitRender' function */

/* Функция сроздания нового объекта-единицы анимации.
 * АРГУМЕНТЫ:
 *   - размер нового объекта в байтах:
 *       INT Size;
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ:
 *   (vg4UNIT *) указатель на созданный объект;
 */
vg4UNIT * VG4_UnitCreate( INT Size )
{
  vg4UNIT *Unit;

  if (Size < sizeof(vg4UNIT) || (Unit = malloc(Size)) == NULL)
    return NULL;

  memset(Unit, 0, Size);
  Unit->Init = VG4_DefUnitInit;
  Unit->Close = VG4_DefUnitClose;
  Unit->Response = VG4_DefUnitResponse;
  Unit->Render = VG4_DefUnitRender;
  return Unit;
} /* End of 'VG4_UnitCreate' function */

/* Функция добавления объекта анимации к системе.
 * АРГУМЕНТЫ:
 *   - указатель на объект:
 *       vg4UNIT *Unit;
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ: Нет.
 */
VOID AnimAdd( vg4UNIT *Unit )
{
  if (VG4_NumOfUnits >= VG4_MAX_UNITS)
    return;
  VG4_Units[VG4_NumOfUnits++] = Unit;
} /* End of 'AnimAdd' function */

/* END OF 'ANIM.C' FILE */
