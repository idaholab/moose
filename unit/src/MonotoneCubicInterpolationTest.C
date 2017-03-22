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

#include "MonotoneCubicInterpolationTest.h"

// Moose includes
#include "MonotoneCubicInterpolation.h"

#include <cmath>

CPPUNIT_TEST_SUITE_REGISTRATION(MonotoneCubicInterpolationTest);

const double MonotoneCubicInterpolationTest::_tol = 1e-8;

void
MonotoneCubicInterpolationTest::setUp()
{
  _x = new std::vector<double>();
  _y = new std::vector<double>();
}

void
MonotoneCubicInterpolationTest::tearDown()
{
  delete _x;
  delete _y;
}

void
MonotoneCubicInterpolationTest::fitQuadraticFunction()
{
  _x->resize(6);
  _y->resize(6);

  std::vector<double> & x = *_x;
  std::vector<double> & y = *_y;

  x[0] = 0.;
  y[0] = 0.;
  x[1] = 2.;
  y[1] = 4.;
  x[2] = 4.;
  y[2] = 16.;
  x[3] = 6.;
  y[3] = 36.;
  x[4] = 8.;
  y[4] = 64.;
  x[5] = 10.;
  y[5] = 100.;

  MonotoneCubicInterpolation interp(*_x, *_y);

  CPPUNIT_ASSERT(std::abs(interp.sample(0.) - 0.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample(1.) - 1.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample(2.) - 4.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample(3.) - 9.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample(4.) - 16.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample(5.) - 25.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample(6.) - 36.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample(7.) - 49.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample(8.) - 64.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample(9.) - 81.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample(10.) - 100.) < _tol);

  CPPUNIT_ASSERT(std::abs(interp.sampleDerivative(0.) - 0.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sampleDerivative(1.) - 2.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sampleDerivative(2.) - 4.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sampleDerivative(3.) - 6.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sampleDerivative(4.) - 8.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sampleDerivative(5.) - 10.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sampleDerivative(6.) - 12.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sampleDerivative(7.) - 14.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sampleDerivative(8.) - 16.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sampleDerivative(9.) - 18.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sampleDerivative(10.) - 20.) < _tol);

  CPPUNIT_ASSERT(std::abs(interp.sample2ndDerivative(0.) - 2.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample2ndDerivative(1.) - 2.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample2ndDerivative(2.) - 2.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample2ndDerivative(3.) - 2.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample2ndDerivative(4.) - 2.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample2ndDerivative(5.) - 2.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample2ndDerivative(6.) - 2.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample2ndDerivative(7.) - 2.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample2ndDerivative(8.) - 2.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample2ndDerivative(9.) - 2.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample2ndDerivative(10.) - 2.) < _tol);
}

void
MonotoneCubicInterpolationTest::fitAkimaDataSet()
{
  _x->resize(11);
  _y->resize(11);

  std::vector<double> & x = *_x;
  std::vector<double> & y = *_y;

  x[0] = 0;
  y[0] = 10;
  x[1] = 2;
  y[1] = 10;
  x[2] = 3;
  y[2] = 10;
  x[3] = 5;
  y[3] = 10;
  x[4] = 6;
  y[4] = 10;
  x[5] = 8;
  y[5] = 10;
  x[6] = 9;
  y[6] = 10.5;
  x[7] = 11;
  y[7] = 15;
  x[8] = 12;
  y[8] = 50;
  x[9] = 14;
  y[9] = 60;
  x[10] = 15;
  y[10] = 85;

  MonotoneCubicInterpolation interp(*_x, *_y);

  CPPUNIT_ASSERT(std::abs(interp.sample(0.) - 10.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample(2.) - 10.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample(3.) - 10.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample(5.) - 10.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample(6.) - 10.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample(8.) - 10.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample(9.) - 10.5) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample(11.) - 15.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample(12.) - 50.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample(14.) - 60.) < _tol);
  CPPUNIT_ASSERT(std::abs(interp.sample(15.) - 85.) < _tol);

  for (double z = 0; z <= 15.; z += .1)
    CPPUNIT_ASSERT(interp.sampleDerivative(z) >= -_tol);
}

void
MonotoneCubicInterpolationTest::getSampleSize()
{
  _x->resize(6);
  _y->resize(6);

  std::vector<double> & x = *_x;
  std::vector<double> & y = *_y;

  x[0] = 0.;
  y[0] = 0.;
  x[1] = 2.;
  y[1] = 4.;
  x[2] = 4.;
  y[2] = 16.;
  x[3] = 6.;
  y[3] = 36.;
  x[4] = 8.;
  y[4] = 64.;
  x[5] = 10.;
  y[5] = 100.;

  MonotoneCubicInterpolation interp(*_x, *_y);
  CPPUNIT_ASSERT(interp.getSampleSize() == _x->size());
}
