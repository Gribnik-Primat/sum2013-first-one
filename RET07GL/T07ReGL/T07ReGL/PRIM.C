/* FILE NAME: PRIM.C
 * PROGRAMMER: bg3
 * DATE: 11.06.2013
 * PURPOSE: Geometry object handle.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
  

#include "anim.h"

#define BG3_PI 3.14159265358979323846


/* Глобальная переменная - цвет вершины по умолчанию */
VEC BG3_PrimDefaultColor = {1, 1, 1};

/* Функция создания примитива.
 * АРГУМЕНТЫ:
 *   - указатель на создаваемый примитив:
 *       bg3PRIM *Prim;
 *   - тип примитива:
 *       bg3PRIM_TYPE Type;
 *   - количества вершин и индексов:
 *       INT NumOfV, NumOfI;
 *   - указатели на вершины (может быть NULL):
 *       VERTEX *Vert;
 *   - указатели на индексы (может быть NULL):
 *       INT *Ind;
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ:
 *   (BOOL) TRUE при успехе, FALSE при нехватке памати.
 */
BOOL BG3_PrimCreate( bg3PRIM *Prim, bg3PRIM_TYPE Type,
                     INT NumOfV, INT NumOfI,
                     VERTEX *Vert, INT *Ind )
{
  INT nv, ni, size, i, j, k;

  memset(Prim, 0, sizeof(bg3PRIM));
  if (Type == BG3_PRIM_GRID)
    nv = NumOfV * NumOfI, ni = NumOfV * (NumOfI - 1) * 2;
  else
    nv = NumOfV, ni = NumOfI;

  /* определяем размер выделяемой памяти */
  size = sizeof(VERTEX) * nv + sizeof(INT) * ni;
  if ((Prim->V = malloc(size)) == NULL)
    return FALSE;
  memset(Prim->V, 0, size);
  Prim->Type = Type;
  Prim->I = (INT *)(Prim->V + nv);
  Prim->NumOfV = nv;
  Prim->NumOfI = ni;
  Prim->Size = size;
  if (Vert == NULL)
    for (i = 0; i < nv; i++)
      Prim->V[i].C = BG3_PrimDefaultColor;
  else
    /* копируем вершины */
    memcpy(Prim->V, Vert, sizeof(VERTEX) * nv);
  if (Ind != NULL)
    /* копируем индексы */
    memcpy(Prim->I, Ind, sizeof(INT) * ni);
  if (Type == BG3_PRIM_GRID)
  {
    /* строим индексы для сетки */
    Prim->GW = NumOfV;
    Prim->GH = NumOfI;

    k = 0;
    for (i = 0; i < Prim->GH - 1; i++)
      for (j = 0; j < Prim->GW; j++)
      {
        Prim->I[k++] = (i + 1) * Prim->GW + j;
        Prim->I[k++] = i * Prim->GW + j;
      }
  }
  return TRUE;
} /* End of 'BG3_PrimCreate' function */

/* Функция удаления примитива.
 * АРГУМЕНТЫ:
 *   - указатель на удаляемый примитив:
 *       bg3PRIM *Prim;
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ: Нет.
 */
VOID BG3_PrimClose( bg3PRIM *Prim )
{
  if (Prim->V != NULL)
    free(Prim->V);
  glDeleteBuffers(2, Prim->Buffs);
  glDeleteVertexArrays(1, &Prim->VertBuf);
  memset(Prim, 0, sizeof(bg3PRIM));
} /* End of 'BG3_PrimClose' function */

/* Функция копирования примитива.
 * АРГУМЕНТЫ:
 *   - указатель на создаваемый примитив:
 *       bg3PRIM *Prim;
 *   - указатель на копируемый примитив:
 *       bg3PRIM *PrimSrc;
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ:
 *   (BOOL) TRUE при успехе, FALSE при нехватке памати.
 */
BOOL BG3_PrimDup( bg3PRIM *Prim, bg3PRIM *PrimSrc )
{
  VERTEX *v;

  memset(Prim, 0, sizeof(bg3PRIM));
  if ((v = malloc(PrimSrc->Size)) == NULL)
    return FALSE;

  /* копируем данные */
  *Prim = *PrimSrc;
  Prim->V = v;
  Prim->I = (INT *)(Prim->V + PrimSrc->NumOfV);
  Prim->Buffs[0] = Prim->Buffs[1] = Prim->VertBuf = 0;
  return TRUE;
} /* End of 'BG3_PrimDup' function */

/* Функция рисования примитива.
 *   - указатель на создаваемый примитив:
 *       bg3PRIM *Prim;
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ: Нет.
 */
VOID BG3_PrimDraw( bg3PRIM *Prim )
{
  INT i;

  if (Prim->Buffs[0] == 0)
  {
    /* выделяем буфера в первый раз */
    glGenBuffers(2, Prim->Buffs);
    glGenVertexArrays(1, &Prim->VertBuf);

    /*** активируем буфера и копируем данные ***/

    /* данные вершин */
    glBindBuffer(GL_ARRAY_BUFFER, Prim->Buffs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTEX) * Prim->NumOfV,
      Prim->V, GL_STATIC_DRAW);
    /* данные с индексами */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Prim->Buffs[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(INT) * Prim->NumOfI,
      Prim->I, GL_STATIC_DRAW);                                                                 
  }

  /* активировали буфера */
  glBindVertexArray(Prim->VertBuf);
  glBindBuffer(GL_ARRAY_BUFFER, Prim->Buffs[0]);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Prim->Buffs[1]);

  /* установили порядок данных */
  glVertexAttribPointer(0, 3, GL_FLOAT, FALSE, sizeof(VERTEX), (VOID *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, FALSE, sizeof(VERTEX), (VOID *)(sizeof(VEC)));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 3, GL_FLOAT, FALSE, sizeof(VERTEX), (VOID *)(sizeof(VEC) + sizeof(UV)));
  glEnableVertexAttribArray(2);

  glVertexAttribPointer(3, 3, GL_FLOAT, FALSE, sizeof(VERTEX), (VOID *)(sizeof(VEC) * 2 + sizeof(UV)));
  glEnableVertexAttribArray(3);

  if (Prim->Type == BG3_PRIM_GRID)
  {
    for (i = 0; i < Prim->GH - 1; i++)
    {
      glBindVertexArray(Prim->VertBuf);
      glBindBuffer(GL_ARRAY_BUFFER, Prim->Buffs[0]);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Prim->Buffs[1]);
      glDrawElements(GL_TRIANGLE_STRIP, Prim->GW * 2, GL_UNSIGNED_INT, (VOID *)(i * Prim->GW * 2 * sizeof(UINT)));
    }
  }
  else
  {
    glBindVertexArray(Prim->VertBuf);
    glBindBuffer(GL_ARRAY_BUFFER, Prim->Buffs[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Prim->Buffs[1]);
    glDrawElements(GL_TRIANGLES, Prim->NumOfI, GL_UNSIGNED_INT, (VOID *)0);
  }
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  /* рисуем нормали: * /
  glBegin(GL_LINES);
  for (i = 0; i < Prim->NumOfV; i++)
  {
    VEC p;

    glColor3d(1, 1, 1);
    glNormal3d(1, 1, 1);
    glVertex3fv(&Prim->V[i].P.X);
    p = VecAddVec(Prim->V[i].P, VecMulNum(Prim->V[i].N, 1.30));
    glVertex3fv(&p.X);
  }
  glEnd();
  /**/
} /* End of 'BG3_PrimDraw' function */

/* Функция создания примитива плоскость.
 * АРГУМЕНТЫ:
 *   - указатель на создаваемый примитив:
 *       bg3PRIM *Prim;
 *   - размер сетки:
 *       INT W, H;
 *   - вектор начала и касательные вектора:
 *       VEC Loc, Du, Dv;
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ:
 *   (BOOL) TRUE при успехе, FALSE при нехватке памати.
 */
BOOL BG3_PrimCreatePlane( bg3PRIM *Prim, INT W, INT H,
                          VEC Loc, VEC Du, VEC Dv )
{
  INT i, j, k = 0;
  VEC norm = VecCrossVec(Du, Dv);

  if (!BG3_PrimCreate(Prim, BG3_PRIM_GRID, W, H, NULL, NULL))
    return FALSE;

  /* Строим вершины плоскости сферы */
  for (i = 0; i < H; i++)
  {
    FLT v = (FLT)i / (H - 1);
    for (j = 0; j < W; j++)
    {
      FLT u = (FLT)j / (W - 1);

      Prim->V[k].P = VecAddVec(Loc,
        VecAddVec(VecMulNum(Du, u), VecMulNum(Dv, v)));
      Prim->V[k].T = UVSet(u, v);
      Prim->V[k].N = norm;
      k++;
    }
  }

  return TRUE;
} /* End of 'BG3_PrimCreatePlane' function */

/* Функция создания примитива плоскость.
 * АРГУМЕНТЫ:
 *   - указатель на создаваемый примитив:
 *       bg3PRIM *Prim;
 *   - размер сетки:
 *       INT W, H;
 *   - вектор центра:
 *       VEC Loc;
 *   - радиус:
 *       FLT R;
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ:
 *   (BOOL) TRUE при успехе, FALSE при нехватке памати.
 */
BOOL BG3_PrimCreateSphere( bg3PRIM *Prim, INT W, INT H,
                           VEC Loc, FLT Radius )
{
  INT i, j, k = 0;

  if (!BG3_PrimCreate(Prim, BG3_PRIM_GRID, W, H, NULL, NULL))
    return FALSE;

  /* Строим вершины плоскости сферы */
  for (i = 0; i < H; i++)
  {
    FLT
      v = (FLT)i / (H - 1),
      theta = v * BG3_PI;
    for (j = 0; j < W; j++)
    {
      FLT
        u = (FLT)j / (W - 1),
        phi = u * 2 * BG3_PI,
        x = sin(theta) * sin(phi),
        y = cos(theta),
        z = sin(theta) * cos(phi);

      Prim->V[k].P = VecAddVec(Loc, VecMulNum(VecSet(x, y, z), Radius));
      Prim->V[k].T = UVSet(u, v);
      Prim->V[k].N = VecSet(x, y, z);
      k++;
    }
  }
  return TRUE;
} /* End of 'BG3_PrimCreateSphere' function */

/* END OF 'PRIM.C' FILE */

