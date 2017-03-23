/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "RotationMatrixTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(RotationMatrixTest);

RotationMatrixTest::RotationMatrixTest() {}

RotationMatrixTest::~RotationMatrixTest() {}

void
RotationMatrixTest::rotVecToVecTest()
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

void
RotationMatrixTest::rotVtoU(RealVectorValue v, RealVectorValue u)
{
  RealTensorValue ident(1, 0, 0, 0, 1, 0, 0, 0, 1);
  RealVectorValue vhat = v / v.size();
  RealVectorValue uhat = u / u.size();
  RealTensorValue r = RotationMatrix::rotVec1ToVec2(v, u);
  RealVectorValue rotated_v = r * vhat;
  for (unsigned i = 0; i < LIBMESH_DIM; ++i)
    CPPUNIT_ASSERT_DOUBLES_EQUAL(rotated_v(i), uhat(i), 0.0001);
  CPPUNIT_ASSERT(r * r.transpose() == ident);
  CPPUNIT_ASSERT(r.transpose() * r == ident);
}
