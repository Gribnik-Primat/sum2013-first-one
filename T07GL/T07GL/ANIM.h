/* FILE NAME: ANIM.H
 * PROGRAMMER: VG4
 * DATE: 06.06.2013
 * PURPOSE: Main animation declaration module.
 */

#ifndef __ANIM_H_
#define __ANIM_H_

#include "def.h"
#include "vec.h"

/* ��� ������������� ��������� �������� */

INT64
  TimeFreq,  /* ������ ��������� ������� � ������� */
  TimeStart, /* ����� ������ �������� */
  TimeOld,   /* ����� ����������� ����� */
  TimePause, /* ����� ������� � ����� */
  TimeShift, /* ����� ������ */
  TimeFPS;   /* ����� ��� ������ FPS - frame-per-second */
LONG FrameCounter;



typedef struct CameraState
{
  /* Camera position */
  VEC Pos;
  /* Camera up */
  VEC Up;
  /* Camera looks at */
  VEC At;
  /* Perspctive view */
  DBL PD;
  /* Projection plane W and H */
  DBL Wh;
  DBL Hh;

  MATR Mview;
  MATR Mproj;
  MATR Mworld;

  MATR MVP;
} CAMERASTATE;



typedef struct tagvg4ANIM
{
  HWND hWnd;      /* ������� ���� �������� */
  HDC hDC;        /* �������� ����� � ������ */
  HGLRC hGLRC;
  HBITMAP hFrame; /* ������ - ����� ����� */
  INT W, H,JPov,JCKID1;/* ������� ����� �������� */
  DOUBLE Jx,Jz,Jy,Jr,Wp,Hp,PD;
  BYTE JButOld[32],JBut[32];
  DWORD *Bits; 
  DBL
  Time,            /* ����� � �������� �� ������ ��������� */
  DeltaTime,       /* ����� � �������� � �������� ����� */
  GlobalTime,      /* ����� � �������� �� ������ ��������� ��� ����� */
  GlobalDeltaTime, /* ����� � �������� � �������� ����� ��� ����� */
  FPS;  /* ���������� ������ � ������� */
  BOOL
    IsPause;  /* ����������� ����� �� ������ (DWORD �� �����) */

  CAMERASTATE camera;

} vg4ANIM;




/*** ���� ���������� �������-������� ��������-������ �������� ***/
typedef struct tagvg4UNIT vg4UNIT;

/* ������� ������������� ������� */
typedef VOID (*vg4UNIT_INIT)( vg4UNIT *Unit, vg4ANIM *Ani );
/* ������� ��������������� ������� */
typedef VOID (*vg4UNIT_CLOSE)( vg4UNIT *Unit );
/* ������� ������� �� ����������� �������������� ������� */
typedef VOID (*vg4UNIT_RESPONSE)( vg4UNIT *Unit, vg4ANIM *Ani );
/* ������� ��������� ������� */
typedef VOID (*vg4UNIT_RENDER)( vg4UNIT *Unit, vg4ANIM *Ani );

void setCamera(VEC pos, VEC lookAt, VEC up);

VEC World2Screen(VEC point);

/* ������� �������� */
#define VG4_UNIT_BASE_FUNCS \
  vg4UNIT_INIT Init;         \
  vg4UNIT_CLOSE Close;       \
  vg4UNIT_RESPONSE Response; \
  vg4UNIT_RENDER Render


/* ��� ������������� ��������� ������� �������� */
struct tagvg4UNIT
{
  VG4_UNIT_BASE_FUNCS;
};



/* ������� ������������� �������� */
BOOL AnimInit( HWND hWnd );

/* ������� ��������������� �������� */
VOID AnimClose( VOID );

/* ������� ��������� ��������� ������� ����� */
VOID AnimResize( INT W, INT H );

/* ������� ���������� ����� */
VOID AnimRender( VOID );

/* ������� ����������� �������� ����� � ���� */
VOID AnimCopyFrame( VOID );

/* ������� ��������� ������ �������-������� �������� */
vg4UNIT * VG4_UnitCreate( INT Size );

/* ������� ���������� ������� �������� � ������� */
VOID AnimAdd( vg4UNIT *Unit );


#endif /* __ANIM_H_ */

/* END OF 'ANIM.H' FILE */
