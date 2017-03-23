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

#ifndef ROTATIONMATRIXTEST_H
#define ROTATIONMATRIXTEST_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

// Moose includes
#include "RotationMatrix.h"

class RotationMatrixTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(RotationMatrixTest);

  CPPUNIT_TEST(rotVecToVecTest);

  CPPUNIT_TEST_SUITE_END();

public:
  RotationMatrixTest();
  ~RotationMatrixTest();

  void rotVecToVecTest();
  void rotVtoU(RealVectorValue v, RealVectorValue u);
};

#endif // ROTATIONMATRIXTEST_H
