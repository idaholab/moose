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

#ifndef INPUTPARAMETERSTEST_H
#define INPUTPARAMETERSTEST_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

class InputParametersTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(InputParametersTest);

  CPPUNIT_TEST(checkControlParamPrivateError);
  CPPUNIT_TEST(checkControlParamTypeError);
  CPPUNIT_TEST(checkControlParamValidError);
  CPPUNIT_TEST(checkSuppressedError);
  CPPUNIT_TEST(checkRangeCheckedParam);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}
  void tearDown() {}

  void checkRangeCheckedParam();
  void checkControlParamPrivateError();
  void checkControlParamTypeError();
  void checkControlParamValidError();
  void checkSuppressedError();
  void checkSetDocString();
  void checkSetDocStringError();

private:
};

#endif // INPUTPARAMETERSTEST_H
