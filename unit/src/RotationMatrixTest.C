//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "RotationMatrix.h"

void
rotVtoU(RealVectorValue v, RealVectorValue u)
{
  RealTensorValue ident(1, 0, 0, 0, 1, 0, 0, 0, 1);
  RealVectorValue vhat = v / v.norm();
  RealVectorValue uhat = u / u.norm();
  RealTensorValue r = RotationMatrix::rotVec1ToVec2(v, u);
  RealVectorValue rotated_v = r * vhat;
  for (unsigned i = 0; i < LIBMESH_DIM; ++i)
    EXPECT_NEAR(rotated_v(i), uhat(i), 0.0001);
  EXPECT_EQ(r * r.transpose(), ident);
  EXPECT_EQ(r.transpose() * r, ident);
}

void
rotV2DtoX(RealVectorValue v)
{
  RealVectorValue u(1, 0, 0);
  RealTensorValue ident(1, 0, 0, 0, 1, 0, 0, 0, 1);
  RealVectorValue vhat = v / v.norm();
  RealTensorValue r = RotationMatrix::rotVec2DToX(v);
  RealVectorValue rotated_v = r * vhat;
  for (unsigned i = 0; i < LIBMESH_DIM; ++i)
    EXPECT_NEAR(rotated_v(i), u(i), 0.0001);
  EXPECT_EQ(r * r.transpose(), ident);
  EXPECT_EQ(r.transpose() * r, ident);
}

TEST(RotationMatrix, rotVecToVec)
{
  // rotations of unit vectors to the x, y and z axes
  rotVtoU(RealVectorValue(1, 0, 0), RealVectorValue(1, 0, 0));
  rotVtoU(RealVectorValue(-1, 0, 0), RealVectorValue(1, 0, 0));
  rotVtoU(RealVectorValue(0, 1, 0), RealVectorValue(1, 0, 0));
  rotVtoU(RealVectorValue(0, -1, 0), RealVectorValue(1, 0, 0));
  rotVtoU(RealVectorValue(0, 0, 1), RealVectorValue(1, 0, 0));
  rotVtoU(RealVectorValue(0, 0, -1), RealVectorValue(1, 0, 0));

  rotVtoU(RealVectorValue(1, 0, 0), RealVectorValue(0, 1, 0));
  rotVtoU(RealVectorValue(-1, 0, 0), RealVectorValue(0, 1, 0));
  rotVtoU(RealVectorValue(0, 1, 0), RealVectorValue(0, 1, 0));
  rotVtoU(RealVectorValue(0, -1, 0), RealVectorValue(0, 1, 0));
  rotVtoU(RealVectorValue(0, 0, 1), RealVectorValue(0, 1, 0));
  rotVtoU(RealVectorValue(0, 0, -1), RealVectorValue(0, 1, 0));

  rotVtoU(RealVectorValue(1, 0, 0), RealVectorValue(0, 0, 1));
  rotVtoU(RealVectorValue(-1, 0, 0), RealVectorValue(0, 0, 1));
  rotVtoU(RealVectorValue(0, 1, 0), RealVectorValue(0, 0, 1));
  rotVtoU(RealVectorValue(0, -1, 0), RealVectorValue(0, 0, 1));
  rotVtoU(RealVectorValue(0, 0, 1), RealVectorValue(0, 0, 1));
  rotVtoU(RealVectorValue(0, 0, -1), RealVectorValue(0, 0, 1));

  // more arbitrary vectors
  rotVtoU(RealVectorValue(1, 2, 3), RealVectorValue(3, 2, 1));
  rotVtoU(RealVectorValue(-1, 2, -3), RealVectorValue(3, -2, 1));
  rotVtoU(RealVectorValue(9, 2, -3), RealVectorValue(-900, -2, 1));
  rotVtoU(RealVectorValue(-0.7455566879693396, -0.16322143154726376, -0.19297210504736562),
          RealVectorValue(-0.7669989857132189, -0.8822797825649573, -0.3274325939199114));
  rotVtoU(RealVectorValue(-0.7669989857132189, -0.8822797825649573, -0.3274325939199114),
          RealVectorValue(-0.5714138293736171, 0.5178596886944302, -0.1602302709779364));
  rotVtoU(RealVectorValue(-0.6195038399590516, 0.24354357871534127, -0.9637350523439685),
          RealVectorValue(0.009588924695567158, 0.3461895857140844, -0.7555301721833589));
  rotVtoU(RealVectorValue(0.6418447552756121, 0.9825056348051839, -0.46612920231628086),
          RealVectorValue(-0.8023314181426899, 0.23569860131733256, 0.24585385679502592));
  rotVtoU(RealVectorValue(0.9115348224777013, -0.1785871095274909, -0.9938009520887727),
          RealVectorValue(0.7000797283703248, -0.4967869392655946, -0.18288272103373449));
}

TEST(RotationMatrix, rotV2DtoX)
{
  // rotations of unit vectors to the x, and y axes
  rotV2DtoX(RealVectorValue(1, 0, 0));
  rotV2DtoX(RealVectorValue(-1, 0, 0));
  rotV2DtoX(RealVectorValue(0, 1, 0));
  rotV2DtoX(RealVectorValue(0, -1, 0));

  // more arbitrary vectors
  rotV2DtoX(RealVectorValue(1, 2, 0));
  rotV2DtoX(RealVectorValue(-1, 2, 0));
  rotV2DtoX(RealVectorValue(9, 2, 0));
  rotV2DtoX(RealVectorValue(-0.7455566879693396, -0.16322143154726376, 0));
  rotV2DtoX(RealVectorValue(-0.7669989857132189, -0.8822797825649573, 0));
  rotV2DtoX(RealVectorValue(-0.6195038399590516, 0.24354357871534127, 0));
  rotV2DtoX(RealVectorValue(0.6418447552756121, 0.9825056348051839, 0));
  rotV2DtoX(RealVectorValue(0.9115348224777013, -0.1785871095274909, 0));
}
