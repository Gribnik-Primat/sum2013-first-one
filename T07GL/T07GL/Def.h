#ifndef __DEF_H_
#define __DEF_H_

#include <windows.h>
#include <glew.h>
#include <gl/gl.h>
#include <gl/glu.h>

#pragma comment(lib, "opengl32")
#pragma comment(lib, "glu32")
#pragma comment(lib, "glew32")

#pragma warning(disable: 4305 4244 4996)

/* Общие типы */
typedef __int64 INT64;
typedef unsigned __int64 UINT64;
typedef DOUBLE DBL;
typedef FLOAT FLT;
typedef struct tagMATR
{
  DBL A[4][4];
} MATR;

typedef struct tagVEC
{
  DBL X, Y, Z;
} VEC;



#endif /* __DEF_H_ */
