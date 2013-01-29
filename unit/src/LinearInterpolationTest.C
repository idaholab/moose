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

#include "LinearInterpolationTest.h"

//Moose includes
#include "LinearInterpolation.h"

#include <cmath>

CPPUNIT_TEST_SUITE_REGISTRATION( LinearInterpolationTest );

const double LinearInterpolationTest::_tol = 1e-5;

void
LinearInterpolationTest::setUp()
{
  _x = new std::vector<double>( 4 );
  _y = new std::vector<double>( 4 );

  std::vector<double> & x = *_x;
  std::vector<double> & y = *_y;

  x[0] = 1.; y[0] = 0.;
  x[1] = 2.; y[1] = 5.;
  x[2] = 3.; y[2] = 6.;
  x[3] = 5.; y[3] = 8.;
}

void
LinearInterpolationTest::tearDown()
{
  delete _x;
  delete _y;
}

void
LinearInterpolationTest::constructor()
{
  LinearInterpolation interp( *_x, *_y );
  CPPUNIT_ASSERT( interp.getSampleSize() == _x->size() );
}


void
LinearInterpolationTest::sample()
{
  LinearInterpolation interp( *_x, *_y );

  CPPUNIT_ASSERT( std::abs(interp.sample( 0. ) - 0.) < _tol );
  CPPUNIT_ASSERT( std::abs(interp.sample( 1. ) - 0.) < _tol );
  CPPUNIT_ASSERT( std::abs(interp.sample( 2. ) - 5.) < _tol );
  CPPUNIT_ASSERT( std::abs(interp.sample( 3. ) - 6.) < _tol );
  CPPUNIT_ASSERT( std::abs(interp.sample( 4. ) - 7.) < _tol );
  CPPUNIT_ASSERT( std::abs(interp.sample( 5. ) - 8.) < _tol );
  CPPUNIT_ASSERT( std::abs(interp.sample( 6. ) - 8.) < _tol );

  CPPUNIT_ASSERT( interp.sample( 1.5 ) == 2.5 );
}

void
LinearInterpolationTest::getSampleSize()
{
  LinearInterpolation interp( *_x, *_y );
  CPPUNIT_ASSERT( interp.getSampleSize() == _x->size() );
}
