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

#ifndef EULER2RGBTEST_H
#define EULER2RGBTEST_H

// CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

class Euler2RGBTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Euler2RGBTest);

  CPPUNIT_TEST(test);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void test();

private:
};

#endif // EULER2RGBTEST_H
