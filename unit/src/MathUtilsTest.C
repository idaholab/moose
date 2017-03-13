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

#include "MathUtilsTest.h"

// Moose includes
#include "MathUtils.h"

CPPUNIT_TEST_SUITE_REGISTRATION(MathUtilsTest);

void
MathUtilsTest::pow()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(MathUtils::pow(1.2345, 73), std::pow(1.2345, 73), 1e-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(MathUtils::pow(-0.99542, 58), std::pow(-0.99542, 58), 1e-5);
}
