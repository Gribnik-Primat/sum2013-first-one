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
  INT X, Y; /* Позиция куба  */
  INT RandShift; /* Случайный сдвиг */
  DBL RandScale; /* Случайный масштаб */
  INT Who;       /* Тип объекта */
} COW;

/* Функция отрисовки куба.
 * АРГУМЕНТЫ:
 *   - указатель на "себя":
 *       COW *Unit;
 *   - указатель на систему анимации:
 *       vg4ANIM *Ani;
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ: Нет.
 */
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





static VOID CowRender( COW *Unit, vg4ANIM *Ani )
{
  
  BYTE Keys[256],KeysOld[256];
  DBL tim = (DBL)clock() / CLOCKS_PER_SEC,Wp = 0.7 * Anim.W/Anim.H,hp=0.7,PD=1.0;
  INT i,j;
  MATR WVP;
  MATR M1,M2;
  static UINT CowProg;
 

  M1 = MatrScale(59,-59,59);
  M2 = MatrTranslate(Unit->X,Unit->Y,Unit->Z);
  MatrWorld = MatrMulMatr(M1,M2);
  MatrView = MatrViewLookAt(VecSet(8,10,8),VecSet(0,0,0),VecSet(1,1,0));
  MatrProj = MatrFrustrum(-1,-1,1,1,-1,100);
  WVP = MatrMulMatr(MatrProj,MatrMulMatr(MatrWorld,MatrView)); 


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
  for (i = 0; i < NumOfVertexes; i++)
  {
    VEC v;
    v = Vertexes[i];
    v.X += Unit->X;
    v.Y += Unit->Y;
    v.Z += 10;

    VertexesProj[i] = WorldToScreen(v); 
  }

  CowProg = ShadProgInit("a.vert", "a.frag");
  glUseProgram(CowProg);

  for (i = 0; i < NumOfFacets; i++)
  {
    POINT p[3];

    for (j = 0; j < 3; j++)
    {
      p[j].x = VertexesProj[Facets[i][j]].x;         
      p[j].y = VertexesProj[Facets[i][j]].y;
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //glColor3d(0.7, 0.5, 0.3);
    glBegin(GL_TRIANGLES);
      glVertex3f(p[0].x, p[0].y, 0);
      glVertex3f(p[1].x, p[1].y, 0);
      glVertex3f(p[2].x, p[2].y, 0);
    glEnd();
  } 
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
