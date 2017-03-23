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

#ifndef MATRIXTOOLSTEST_H
#define MATRIXTOOLSTEST_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

// Moose includes
#include "MatrixTools.h"

class MatrixToolsTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(MatrixToolsTest);

  CPPUNIT_TEST(matrixInversionTest1);
  CPPUNIT_TEST(matrixInversionTest2);
  CPPUNIT_TEST(matrixInversionTest3);

  CPPUNIT_TEST_SUITE_END();

public:
  MatrixToolsTest();

  void matrixInversionTest1();
  void matrixInversionTest2();
  void matrixInversionTest3();
};

#endif // MATRIXTOOLSTEST_H
