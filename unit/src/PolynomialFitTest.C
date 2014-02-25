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

#include "PolynomialFitTest.h"

//Moose includes
#include "PolynomialFit.h"

#include <cmath>

CPPUNIT_TEST_SUITE_REGISTRATION( PolynomialFitTest );

const double PolynomialFitTest::_tol = 1e-5;

void
PolynomialFitTest::setUp()
{
  _x = new std::vector<double>( 4 );
  _y = new std::vector<double>( 4 );

  std::vector<double> & x = *_x;
  std::vector<double> & y = *_y;

  x[0] = -1.; y[0] = 1.;
  x[1] =  0.; y[1] = 0.;
  x[2] =  1.; y[2] = 1.;
}

void
PolynomialFitTest::tearDown()
{
  delete _x;
  delete _y;
}

void
PolynomialFitTest::constructor()
{
  PolynomialFit poly( *_x, *_y, 2 );
  CPPUNIT_ASSERT( poly.getSampleSize() == _x->size() );
}

void
PolynomialFitTest::sample()
{
  PolynomialFit poly( *_x, *_y, 2 );
  poly.generate();

  CPPUNIT_ASSERT( std::abs(poly.sample( -2. ) - 4) < _tol );
  CPPUNIT_ASSERT( std::abs(poly.sample( -1. ) - 1) < _tol );
  CPPUNIT_ASSERT( std::abs(poly.sample(  0. ) - 0) < _tol );
  CPPUNIT_ASSERT( std::abs(poly.sample(  1. ) - 1) < _tol );
  CPPUNIT_ASSERT( std::abs(poly.sample(  2. ) - 4) < _tol );
}

void
PolynomialFitTest::getSampleSize()
{
  PolynomialFit poly( *_x, *_y, 2 );
  CPPUNIT_ASSERT( poly.getSampleSize() == _x->size() );
}
