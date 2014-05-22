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

CPPUNIT_TEST_SUITE_REGISTRATION( RotationMatrixTest );

RotationMatrixTest::RotationMatrixTest()
{}

RotationMatrixTest::~RotationMatrixTest()
{}

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
}

void
RotationMatrixTest::rotVtoU(RealVectorValue v, RealVectorValue u)
{
  RealTensorValue ident(1,0,0,  0,1,0,  0,0,1);
  RealVectorValue vhat = v/v.size();
  RealVectorValue uhat = u/u.size();
  RealTensorValue r = RotationMatrix::rotVec1ToVec2(v, u);
  RealVectorValue rotated_v = r*vhat;
  for (unsigned i = 0 ; i < LIBMESH_DIM ; ++i)
    CPPUNIT_ASSERT_DOUBLES_EQUAL( rotated_v(i), uhat(i), 0.0001);
  CPPUNIT_ASSERT( r*r.transpose() == ident);
  CPPUNIT_ASSERT( r.transpose()*r == ident);
}
