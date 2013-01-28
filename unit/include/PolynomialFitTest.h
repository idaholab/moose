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

#ifndef POLYNOMIALFITTEST_H
#define POLYNOMIALFITTEST_H

//CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

class PolynomialFitTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( PolynomialFitTest );

  CPPUNIT_TEST( constructor );
  CPPUNIT_TEST( sample );
  CPPUNIT_TEST( getSampleSize );

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void constructor();
  void sample();
  void getSampleSize();

private:
  std::vector<double> * _x;
  std::vector<double> * _y;

  static const double _tol = 1e-5;
};

#endif  // POLYNOMIALFITTEST_H
