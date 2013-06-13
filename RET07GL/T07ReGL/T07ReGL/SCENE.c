#include <math.h>
#include <time.h>

#include "def.h"
#include "units.h"
#include "keycodes.h"


/* Function definitions */
VOID SetCamera( bg3ANIM * Ani, VEC pos, VEC lookAt, VEC up );

static MATR CowMatrix()
{
  return MatrMulMatr(MatrRotateX(90), MatrScale(15, 15, 15));
}



VOID BG3_SceneCreate(bg3ANIM * Ani)
{
  POLYGONAL_MODEL * models;
  MATR modelMatr;
  
  
  Ani->camera.Wh = 2;
  Ani->camera.Hh = 2;
  Ani->camera.PD = 1;
  Ani->camera.projMatr = MatrFrustum(-Ani->camera.Wh/2, Ani->camera.Wh/2,
                                         -Ani->camera.Hh/2, Ani->camera.Hh/2, 1, 10000);
  
  models = loadObjModel("cow_new1.object");
  modelMatr = CowMatrix();

  
   srand(30);
   BG3_AnimAdd(CreateControlledUnit((bg3UNITPOS*)PolyUnitCreate(0, 0, 0,models, modelMatr), 1));

}

VOID BG3_SceneResponse(bg3ANIM * ani)
{
  bg3UNITPOS * unit = (bg3UNITPOS *)ani->units[0];
  DBL time = clock();
  if (BG3_KeyNewPressed('P'))
  {
      ani->isPaused = !ani->isPaused;
  }
  
  SetCamera(ani, VecSet(200, 200, 200), VecSet(0,0,0), VecSet(0, 0, 1));
}

VOID SetCamera( bg3ANIM * Ani, VEC pos, VEC lookAt, VEC up )
{
  Ani->camera.Pos = pos;
  Ani->camera.At = lookAt;
  Ani->camera.Up = up;
  Ani->camera.viewMatr = MatrLookAt(pos, lookAt, up);
  Ani->camera.MVP = MatrMulMatr(Ani->camera.viewMatr, Ani->camera.projMatr);
}