/* FILE NAME: MEGACOW.C
 * PROGRAMMER: VG4
 * DATE: 06.06.2013
 * PURPOSE: Animation sample module.
 */


#include <windows.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <mmsystem.h>
#pragma comment(lib,"winmm")

#include "vec.h"
#include "anim.h"




/***
 * Функции обработки мегакуба
 ***/

/* Тип представления мегакуба */
typedef struct
{
  /* Включаем базовые поля объекта анимации */
  VG4_UNIT_BASE_FUNCS;
  INT X, Y, Z; /* Позиция куба  */
  INT RandShift; /* Случайный сдвиг */
  DBL RandScale; /* Случайный масштаб */
  INT Who;       /* Тип объекта */
  DBL r,g,b;
} COW;

/* Функция отрисовки куба.
 * АРГУМЕНТЫ:
 *   - указатель на "себя":
 *       COW *Unit;
 *   - указатель на систему анимации:
 *       vg4ANIM *Ani;
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ: Нет.
 */


static INT CowProg = -2;

VEC *Vertexes, *VertexesProj;
INT NumOfVertexes;

/* Facet array */
INT (*Facets)[3];
INT NumOfFacets;

static vg4ANIM Anim;

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



VEC WorldToScreen( VEC P )
{
  VEC P1, P2, P3, P4;
  VEC pt;
  DBL Xs, Ys;

  Anim.camera.Mworld = MatrMulMatr(MatrScale(40, 40, 40), MatrTranslate(0, 0, -100)); 
  Anim.camera.Mview = MatrLookAt(VecSet(0, 0, -3), VecSet(Anim.Jx, Anim.Jy, Anim.Jr), VecSet(0, 1, 0)); 
  Anim.camera.Mproj = MatrFrustrum(-1, 1, 1, -1, 1, 10);


  Anim.Wp = 1.0;
  Anim.Hp = 1.0;
  Anim.PD = 1;

  P1 = VecMulMatr(P, Anim.camera.Mworld);
  P2 = VecMulMatr(P1, Anim.camera.Mview);
  P3 = VecMulMatr(P2, Anim.camera.Mproj);

  P4.X = P2.X * Anim.PD/P2.Z;
  P4.Y = P2.Y * Anim.PD/P2.Z;                                            



  P4.X *= 2/Anim.Wp;
  P4.Y *= 2/Anim.Hp;

  Xs = (P4.X + 1) * (Anim.W - 1);
  Ys = (-P4.Y + 1) * (Anim.H - 1);
  pt.X = Xs;
  pt.Y = Ys;
  return pt;
}



static VOID CowRender( COW *Unit, vg4ANIM *Ani )
{
  
  /*if (CowProg < -1)
  {
    CowProg = ShadProgInit("Z:\\sum2013\\T06anim\\a.vert", "Z:\\sum2013\\T06anim\\a.frag");
  }
  if (Unit->model != NULL && CowProg >= 0)
  {
    HDC hDC = Ani->hDC;
    POLYGONAL_MODEL * model = Unit->model;
    VEC * vertexes = model->vertexes;
    FACET * facets = model->facets;
    MATR translateMatrix = MatrTranslate(Unit->X, Unit->Y, Unit->Z);
    MATR rotateMatrix = MatrRotateZ(Unit->RotZ);
    MATR transformMatrix = MatrMulMatr(MatrMulMatr(Unit->transformMatrix, rotateMatrix), translateMatrix);
    MATR fullMatr = MatrMulMatr(transformMatrix, Ani->camera.MVP);
    VEC * vertexesProj = Unit->model->vertProj;
    INT i, j;
    UINT locMatr, locTime;

    glColor3f(Unit->r, Unit->g, Unit->b);
        
    glUseProgram(CowProg);
    locMatr = glGetUniformLocation(CowProg, "Matr");
    locTime = glGetUniformLocation(CowProg, "Time");
    if (locMatr >= 0)
    {
      glUniformMatrix4fv(locMatr, 1, FALSE, &fullMatr.A[0][0]);
    }
    if (locTime >= 0)
    {
      glUniform1f(locTime, (float)(Ani->timer.activeTime));
    }

    // Draw model
    glBegin(GL_TRIANGLES);
    for (i = 0; i < model->facetsNum; i++)
    {
      for (j = 0; j < 3; j++)
      {
        VEC v = vertexes[facets[i].vertexes[j]];
        glVertex3f(v.X, v.Y, v.Z);
      }
    }
    glEnd();
    glUseProgram(0);
  }
} /* End of 'CubeRender' */
  
  BYTE Keys[256],KeysOld[256];
  DBL tim = (DBL)clock() / CLOCKS_PER_SEC,Wp = 0.7 * Anim.W/Anim.H,hp=0.7,PD=1.0;
  INT i,j;
  MATR WVP;
  MATR M1,M2;
  static UINT CowProg,locMatr,locTime;
  MATR fullMatr;
  
  if (CowProg < -1)
  {
    CowProg = ShadProgInit("Z:\\sum2013\\T07anim\\a.vert", "Z:\\sum2013\\T07anim\\a.frag");
  }

  setCamera(VecSet(-100, 600, -136), VecSet(Unit->X, Unit->Y, -136), VecSet(0, 0, 1));

  M1 = MatrScale(59,-59,59);
  M2 = MatrTranslate(Unit->X,Unit->Y,Unit->Z);
  Anim.camera.Mworld = MatrMulMatr(M1,M2);

  Anim.camera.Mproj = MatrFrustrum(-1,-1,1,1,-1,100);
  WVP = MatrMulMatr(Anim.camera.Mproj,MatrMulMatr(Anim.camera.Mworld,Anim.camera.Mview)); 
  fullMatr = MatrMulMatr(Anim.camera.Mworld, Ani->camera.MVP);


  LoadCow();

  memcpy(KeysOld,Keys,256);

  GetKeyboardState(Keys);

  for(i = 0;i < 256; i++)
    Keys[i]>>=7;
  

  
  /* calculate projection */
    
  if ((i = joyGetNumDevs()) > 1)
  {
    JOYCAPS jc;

    if (joyGetDevCaps(Anim.JCKID1, &jc, sizeof(jc)) == JOYERR_NOERROR)
    {
      JOYINFOEX ji;

      ji.dwSize = sizeof(ji);
      ji.dwFlags = JOY_RETURNALL;

      if (joyGetPosEx(Anim.JCKID1, &ji) == JOYERR_NOERROR)
      {
        memcpy(Anim.JButOld, Anim.JBut, 32);
        for (i = 0; i < 32; i++)
          Anim.JBut[i] = (ji.dwButtons >> i) & 1;

        Anim.Jx = 2.0 * (ji.dwXpos - jc.wXmin) / (jc.wXmax - jc.wXmin) - 1;
        Anim.Jy = 2.0 * (ji.dwYpos - jc.wYmin) / (jc.wYmax - jc.wYmin) - 1;
        Anim.Jz = 2.0 * (ji.dwZpos - jc.wZmin) / (jc.wZmax - jc.wZmin) - 1;
        Anim.Jr = 2.0 * (ji.dwRpos - jc.wRmin) / (jc.wRmax - jc.wRmin) - 1;

        if (ji.dwPOV == 0xFFFF)
          Anim.JPov = 0;
        else
          Anim.JPov = ji.dwPOV / 4500 + 1;
      }
    }
  }


  
  j = glGetUniformLocation(CowProg, "DrawColor");

  glUseProgram(CowProg);
  locMatr = glGetUniformLocation(CowProg, "Matr");
    locTime = glGetUniformLocation(CowProg, "Time");
    if (locMatr >= 0)
    {
      glUniformMatrix4fv(locMatr, 1, FALSE, &fullMatr.A[0][0]);
    }
    if (locTime >= 0)
    {
      glUniform1f(locTime, (float)(Ani->Time));
    }

   glBegin(GL_TRIANGLES);
    for (i = 0; i < NumOfFacets; i++)
    {
      for (j = 0; j < 3; j++)
      {
        VEC v = Vertexes[i][j];
        glVertex3f(v.X, v.Y, v.Z);
      }
    }
    glEnd();
    glUseProgram(0);
  


} /* End of 'CowRender' function */

/* Функция сроздания нового куба.
 * АРГУМЕНТЫ:
 *   - позиция на экране:
 *       INT X, Y;
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ:
 *   (vg4UNIT *) указатель на созданный объект;
 */
VEC *Vertexes, *VertexesProj;
INT NumOfVertexes;

/* Facet array */
INT (*Facets)[3];
INT NumOfFacets;



vg4UNIT * CowCreate( INT X, INT Y )
{
  COW *Unit;


  if ((Unit = (COW *)VG4_UnitCreate(sizeof(COW))) == NULL)
    return NULL;

  Unit->Render = (vg4UNIT_RENDER)CowRender;
  Unit->X = X;
  Unit->Y = Y;
  Unit->Who = rand() % 3;
  Unit->RandShift = 1;
  Unit->RandScale = 59;
  return (vg4UNIT *)Unit;
} /* End of 'CowCreate' function */

/* END OF 'MEGACOW.C' FILE */
