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

#ifndef MONOTONECUBICINTERPOLATIONTEST_H
#define MONOTONECUBICINTERPOLATIONTEST_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

class MonotoneCubicInterpolationTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MonotoneCubicInterpolationTest);

  CPPUNIT_TEST(fitQuadraticFunction);
  CPPUNIT_TEST(fitAkimaDataSet);
  CPPUNIT_TEST(getSampleSize);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void fitQuadraticFunction();
  void fitAkimaDataSet();
  void getSampleSize();

private:
  std::vector<double> * _x;
  std::vector<double> * _y;

  static const double _tol;
};

#endif // MONOTONECUBICINTERPOLATIONTEST_H
