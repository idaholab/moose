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

#ifndef MATHUTILSTEST_H
#define MATHUTILSTEST_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

class MathUtilsTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(MathUtilsTest);

  CPPUNIT_TEST(pow);

  CPPUNIT_TEST_SUITE_END();

public:
  void pow();
};

#endif // MATHUTILSTEST_H
