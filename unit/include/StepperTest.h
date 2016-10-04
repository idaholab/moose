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

#ifndef STEPPERTEST_H
#define STEPPERTEST_H

//CPPUnit includes
#include "GuardedHelperMacros.h"

class StepperInfo;

class StepperTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(StepperTest);
  CPPUNIT_TEST(fixedPoint);
  CPPUNIT_TEST(maxRatio);
  CPPUNIT_TEST(scratch);
  CPPUNIT_TEST_SUITE_END();

public:
  void fixedPoint();
  void maxRatio();
  void scratch();
};

#endif  // STEPPERTEST_H
