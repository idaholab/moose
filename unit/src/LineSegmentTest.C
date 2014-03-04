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

#include "LineSegmentTest.h"

// libMesh includes
#include "libmesh/plane.h"

CPPUNIT_TEST_SUITE_REGISTRATION( LineSegmentTest );

LineSegmentTest::LineSegmentTest():
  _posx(Point(0,0), Point(5,0)),
  _posy(Point(0,0), Point(0,5)),
  _negy(Point(0,0), Point(0,-5)),
  _posdiag(Point(0,0), Point(5,5)),
  _negdiag(Point(0,0), Point(-5,-5)),
  _pos3x(Point(0,0,0), Point(5,0,0)),
  _neg3y(Point(0,0,0), Point(0,-5,0)),
  _pos3diag(Point(0,0,0), Point(5,5,5)),
  _neg3diag(Point(0,0,0), Point(-5,-5,-5))
{ }

LineSegmentTest::~LineSegmentTest()
{ }

void
LineSegmentTest::closestPointTest()
{
  // positive x end cases
  CPPUNIT_ASSERT( _posx.closest_point(Point(-1, 0)) == Point(0, 0) );
  CPPUNIT_ASSERT( _posx.closest_point(Point(0, -1)) == Point(0, 0) );
  CPPUNIT_ASSERT( _posx.closest_point(Point(0, 1)) == Point(0, 0) );
  CPPUNIT_ASSERT( _posx.closest_point(Point(6, 0)) == Point(5, 0) );
  CPPUNIT_ASSERT( _posx.closest_point(Point(6, -1)) == Point(5, 0) );
  CPPUNIT_ASSERT( _posx.closest_point(Point(6, 1)) == Point(5, 0) );
  CPPUNIT_ASSERT( _pos3x.closest_point(Point(-1, 0, 0)) == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _pos3x.closest_point(Point(0, -1, 0)) == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _pos3x.closest_point(Point(0, 1, 0)) == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _pos3x.closest_point(Point(6, 0, 0)) == Point(5, 0, 0) );
  CPPUNIT_ASSERT( _pos3x.closest_point(Point(6, -1, 0)) == Point(5, 0, 0) );
  CPPUNIT_ASSERT( _pos3x.closest_point(Point(6, 1, 0)) == Point(5, 0, 0) );
  // middle of the line
  CPPUNIT_ASSERT( _posx.closest_point(Point(2, 0)) == Point(2, 0) );
  CPPUNIT_ASSERT( _posx.closest_point(Point(2, 100000)) == Point(2, 0) );
  CPPUNIT_ASSERT( _posx.closest_point(Point(4, -100000)) == Point(4, 0) );
  CPPUNIT_ASSERT( _posx.closest_point(Point(0.125, -0.125)) == Point(0.125, 0) );
  CPPUNIT_ASSERT( _pos3x.closest_point(Point(2, 0, 0)) == Point(2, 0, 0) );
  CPPUNIT_ASSERT( _pos3x.closest_point(Point(2, 100000, 10000)) == Point(2, 0, 0) );
  CPPUNIT_ASSERT( _pos3x.closest_point(Point(4, -100000, 10000)) == Point(4, 0, 0) );
  CPPUNIT_ASSERT( _pos3x.closest_point(Point(0.125, -0.125, 0.125)) == Point(0.125, 0, 0) );

  // positive y end cases
  CPPUNIT_ASSERT( _posy.closest_point(Point(0, -1)) == Point(0, 0) );
  CPPUNIT_ASSERT( _posy.closest_point(Point(-1, 0)) == Point(0, 0) );
  CPPUNIT_ASSERT( _posy.closest_point(Point(1, 0)) == Point(0, 0) );
  CPPUNIT_ASSERT( _posy.closest_point(Point(0, 6)) == Point(0, 5) );
  CPPUNIT_ASSERT( _posy.closest_point(Point(-1, 6)) == Point(0, 5) );
  CPPUNIT_ASSERT( _posy.closest_point(Point(1, 6)) == Point(0, 5) );
  // middle of the line
  CPPUNIT_ASSERT( _posy.closest_point(Point(0, 2)) == Point(0, 2) );
  CPPUNIT_ASSERT( _posy.closest_point(Point(100000, 2)) == Point(0, 2) );
  CPPUNIT_ASSERT( _posy.closest_point(Point(-100000, 4)) == Point(0, 4) );
  CPPUNIT_ASSERT( _posy.closest_point(Point(-0.125, 0.125)) == Point(0, 0.125) );

  // negative y end cases
  CPPUNIT_ASSERT( _negy.closest_point(Point(0, 1)) == Point(0, 0) );
  CPPUNIT_ASSERT( _negy.closest_point(Point(-1, 0)) == Point(0, 0) );
  CPPUNIT_ASSERT( _negy.closest_point(Point(1, 0)) == Point(0, 0) );
  CPPUNIT_ASSERT( _negy.closest_point(Point(0, -6)) == Point(0, -5) );
  CPPUNIT_ASSERT( _negy.closest_point(Point(-1, -6)) == Point(0, -5) );
  CPPUNIT_ASSERT( _negy.closest_point(Point(1, -6)) == Point(0, -5) );
  CPPUNIT_ASSERT( _neg3y.closest_point(Point(0, 1, 0)) == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _neg3y.closest_point(Point(0, 0, 1)) == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _neg3y.closest_point(Point(-1, 0, 1)) == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _neg3y.closest_point(Point(-1, 0, -1)) == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _neg3y.closest_point(Point(1, 0, 0)) == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _neg3y.closest_point(Point(0, -6, 0)) == Point(0, -5, 0) );
  CPPUNIT_ASSERT( _neg3y.closest_point(Point(-1, -6, -9)) == Point(0, -5, 0) );
  CPPUNIT_ASSERT( _neg3y.closest_point(Point(1, -6, 9)) == Point(0, -5, 0) );
  // middle of the line
  CPPUNIT_ASSERT( _negy.closest_point(Point(0, -2)) == Point(0, -2) );
  CPPUNIT_ASSERT( _negy.closest_point(Point(100000, -2)) == Point(0, -2) );
  CPPUNIT_ASSERT( _negy.closest_point(Point(-100000, -4)) == Point(0, -4) );
  CPPUNIT_ASSERT( _negy.closest_point(Point(-0.125, -0.125)) == Point(0, -0.125) );
  CPPUNIT_ASSERT( _neg3y.closest_point(Point(0, -2, 0)) == Point(0, -2, 0) );
  CPPUNIT_ASSERT( _neg3y.closest_point(Point(100000, -2, -1000)) == Point(0, -2, 0) );
  CPPUNIT_ASSERT( _neg3y.closest_point(Point(-100000, -4, 333)) == Point(0, -4, 0) );
  CPPUNIT_ASSERT( _neg3y.closest_point(Point(-0.125, -0.125, 2)) == Point(0, -0.125, 0) );

  // positive diagonal end cases
  CPPUNIT_ASSERT( _posdiag.closest_point(Point(0, -1)) == Point(0, 0) );
  CPPUNIT_ASSERT( _posdiag.closest_point(Point(-1, -1)) == Point(0, 0) );
  CPPUNIT_ASSERT( _posdiag.closest_point(Point(0, 0)) == Point(0, 0) );
  CPPUNIT_ASSERT( _posdiag.closest_point(Point(6, 6)) == Point(5, 5) );
  CPPUNIT_ASSERT( _posdiag.closest_point(Point(7, 6)) == Point(5, 5) );
  CPPUNIT_ASSERT( _posdiag.closest_point(Point(6, 9)) == Point(5, 5) );
  CPPUNIT_ASSERT( _pos3diag.closest_point(Point(0, -1, 1)) == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _pos3diag.closest_point(Point(-1, -1, 0)) == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _pos3diag.closest_point(Point(0, 0, 0)) == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _pos3diag.closest_point(Point(6, 6, 6)) == Point(5, 5, 5) );
  CPPUNIT_ASSERT( _pos3diag.closest_point(Point(7, 6, 5)) == Point(5, 5, 5) );
  CPPUNIT_ASSERT( _pos3diag.closest_point(Point(6, 9, 8)) == Point(5, 5, 5) );
  // middle of the line
  CPPUNIT_ASSERT( _posdiag.closest_point(Point(2, 2)) == Point(2, 2) );
  CPPUNIT_ASSERT( _posdiag.closest_point(Point(0, 2)) == Point(1, 1) );
  CPPUNIT_ASSERT( _posdiag.closest_point(Point(2, 0)) == Point(1, 1) );
  CPPUNIT_ASSERT( _posdiag.closest_point(Point(1, 3)) == Point(2, 2) );
  CPPUNIT_ASSERT( _pos3diag.closest_point(Point(2, 2, 2)) == Point(2, 2, 2) );
  CPPUNIT_ASSERT( _pos3diag.closest_point(Point(0, 0, 4)) == Point(4./3., 4./3., 4./3.) );
  CPPUNIT_ASSERT( _pos3diag.closest_point(Point(0, 4, 4)) == Point(8./3., 8./3., 8./3.) );

  // negative diagonal end cases
  CPPUNIT_ASSERT( _negdiag.closest_point(Point(0, 1)) == Point(0, 0) );
  CPPUNIT_ASSERT( _negdiag.closest_point(Point(1, 1)) == Point(0, 0) );
  CPPUNIT_ASSERT( _negdiag.closest_point(Point(0, 0)) == Point(0, 0) );
  CPPUNIT_ASSERT( _negdiag.closest_point(Point(-6, -6)) == Point(-5, -5) );
  CPPUNIT_ASSERT( _negdiag.closest_point(Point(-7, -6)) == Point(-5, -5) );
  CPPUNIT_ASSERT( _negdiag.closest_point(Point(-6, -9)) == Point(-5, -5) );
  CPPUNIT_ASSERT( _neg3diag.closest_point(Point(0, 1, 0)) == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _neg3diag.closest_point(Point(1, 1, 1)) == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _neg3diag.closest_point(Point(0, 0, 0)) == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _neg3diag.closest_point(Point(-6, -6, -6)) == Point(-5, -5, -5) );
  CPPUNIT_ASSERT( _neg3diag.closest_point(Point(-7, -6, -9)) == Point(-5, -5, -5) );
  CPPUNIT_ASSERT( _neg3diag.closest_point(Point(-6, -9, -5)) == Point(-5, -5, -5) );
  // middle of the line
  CPPUNIT_ASSERT( _negdiag.closest_point(Point(-2, -2)) == Point(-2, -2) );
  CPPUNIT_ASSERT( _negdiag.closest_point(Point(0, -2)) == Point(-1, -1) );
  CPPUNIT_ASSERT( _negdiag.closest_point(Point(-2, 0)) == Point(-1, -1) );
  CPPUNIT_ASSERT( _negdiag.closest_point(Point(-1, -3)) == Point(-2, -2) );
  CPPUNIT_ASSERT( _neg3diag.closest_point(Point(-2, -2, -2)) == Point(-2, -2, -2) );
  CPPUNIT_ASSERT( _neg3diag.closest_point(Point(0, 0, -4)) == Point(-4./3., -4./3., -4./3.) );
  CPPUNIT_ASSERT( _neg3diag.closest_point(Point(0, -4, -4)) == Point(-8./3., -8./3., -8./3.) );
}

void
LineSegmentTest::closestNormalPointTest()
{
  Point result;
  CPPUNIT_ASSERT( _posx.closest_normal_point(Point(0, 0), result) == true );
  CPPUNIT_ASSERT( result == Point(0, 0) );
  CPPUNIT_ASSERT( _posx.closest_normal_point(Point(0, 10), result) == true );
  CPPUNIT_ASSERT( result == Point(0, 0) );
  CPPUNIT_ASSERT( _posx.closest_normal_point(Point(5, -1), result) == true );
  CPPUNIT_ASSERT( result == Point(5, 0) );
  CPPUNIT_ASSERT( _posx.closest_normal_point(Point(2, 2), result) == true );
  CPPUNIT_ASSERT( result == Point(2, 0) );
  CPPUNIT_ASSERT( _posx.closest_normal_point(Point(6, -1), result) == false );
  CPPUNIT_ASSERT( result == Point(6, 0) );
  CPPUNIT_ASSERT( _posx.closest_normal_point(Point(-9, 2), result) == false );
  CPPUNIT_ASSERT( result == Point(-9, 0) );
  CPPUNIT_ASSERT( _pos3x.closest_normal_point(Point(0, 0, 0), result) == true );
  CPPUNIT_ASSERT( result == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _pos3x.closest_normal_point(Point(0, 10, -5), result) == true );
  CPPUNIT_ASSERT( result == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _pos3x.closest_normal_point(Point(5, -1, 1), result) == true );
  CPPUNIT_ASSERT( result == Point(5, 0, 0) );
  CPPUNIT_ASSERT( _pos3x.closest_normal_point(Point(2, 2, 2), result) == true );
  CPPUNIT_ASSERT( result == Point(2, 0, 0) );
  CPPUNIT_ASSERT( _pos3x.closest_normal_point(Point(6, -1, 0), result) == false );
  CPPUNIT_ASSERT( result == Point(6, 0, 0) );
  CPPUNIT_ASSERT( _pos3x.closest_normal_point(Point(-9, 2, 4), result) == false );
  CPPUNIT_ASSERT( result == Point(-9, 0, 0) );

  CPPUNIT_ASSERT( _negy.closest_normal_point(Point(0, -5), result) == true );
  CPPUNIT_ASSERT( result == Point(0, -5) );
  CPPUNIT_ASSERT( _negy.closest_normal_point(Point(-10, 0), result) == true );
  CPPUNIT_ASSERT( result == Point(0, 0) );
  CPPUNIT_ASSERT( _negy.closest_normal_point(Point(-1, -5), result) == true );
  CPPUNIT_ASSERT( result == Point(0, -5) );
  CPPUNIT_ASSERT( _negy.closest_normal_point(Point(-2, -2), result) == true );
  CPPUNIT_ASSERT( result == Point(0, -2) );
  CPPUNIT_ASSERT( _negy.closest_normal_point(Point(-1, 6), result) == false );
  CPPUNIT_ASSERT( result == Point(0, 6) );
  CPPUNIT_ASSERT( _negy.closest_normal_point(Point(2, -9), result) == false );
  CPPUNIT_ASSERT( result == Point(0, -9) );
  CPPUNIT_ASSERT( _neg3y.closest_normal_point(Point(0, -5, 0), result) == true );
  CPPUNIT_ASSERT( result == Point(0, -5, 0) );
  CPPUNIT_ASSERT( _neg3y.closest_normal_point(Point(0, 5, 0), result) == false );
  CPPUNIT_ASSERT( result == Point(0, 5, 0) );
  CPPUNIT_ASSERT( _neg3y.closest_normal_point(Point(0, -10, 0), result) == false );
  CPPUNIT_ASSERT( result == Point(0, -10, 0) );
  CPPUNIT_ASSERT( _neg3y.closest_normal_point(Point(2, -9, -4), result) == false );
  CPPUNIT_ASSERT( result == Point(0, -9, 0) );

  CPPUNIT_ASSERT( _posdiag.closest_normal_point(Point(0, 0), result) == true );
  CPPUNIT_ASSERT( result == Point(0, 0) );
  CPPUNIT_ASSERT( _posdiag.closest_normal_point(Point(1, 0), result) == true );
  CPPUNIT_ASSERT( result == Point(0.5, 0.5) );
  CPPUNIT_ASSERT( _posdiag.closest_normal_point(Point(0, 2), result) == true );
  CPPUNIT_ASSERT( result == Point(1, 1) );
  CPPUNIT_ASSERT( _posdiag.closest_normal_point(Point(0, 12), result) == false );
  CPPUNIT_ASSERT( result == Point(6, 6) );
  CPPUNIT_ASSERT( _pos3diag.closest_normal_point(Point(0, 0, 0), result) == true );
  CPPUNIT_ASSERT( result == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _pos3diag.closest_normal_point(Point(0, 0, 4), result) == true );
  CPPUNIT_ASSERT( result == Point(4./3., 4./3., 4./3.) );
  CPPUNIT_ASSERT( _pos3diag.closest_normal_point(Point(0, 4, 4), result) == true );
  CPPUNIT_ASSERT( result == Point(8./3., 8./3., 8./3.) );

  // negative diagonal end cases
  CPPUNIT_ASSERT( _negdiag.closest_point(Point(0, 1)) == Point(0, 0) );
  CPPUNIT_ASSERT( _negdiag.closest_point(Point(1, 1)) == Point(0, 0) );
  CPPUNIT_ASSERT( _negdiag.closest_point(Point(0, 0)) == Point(0, 0) );
  CPPUNIT_ASSERT( _negdiag.closest_point(Point(-6, -6)) == Point(-5, -5) );
  CPPUNIT_ASSERT( _negdiag.closest_point(Point(-7, -6)) == Point(-5, -5) );
  CPPUNIT_ASSERT( _negdiag.closest_point(Point(-6, -9)) == Point(-5, -5) );
  CPPUNIT_ASSERT( _neg3diag.closest_point(Point(0, 1, 0)) == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _neg3diag.closest_point(Point(1, 1, 1)) == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _neg3diag.closest_point(Point(0, 0, 0)) == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _neg3diag.closest_point(Point(-6, -6, -6)) == Point(-5, -5, -5) );
  CPPUNIT_ASSERT( _neg3diag.closest_point(Point(-7, -6, -9)) == Point(-5, -5, -5) );
  CPPUNIT_ASSERT( _neg3diag.closest_point(Point(-6, -9, -5)) == Point(-5, -5, -5) );
  // middle of the line
  CPPUNIT_ASSERT( _negdiag.closest_point(Point(-2, -2)) == Point(-2, -2) );
  CPPUNIT_ASSERT( _negdiag.closest_point(Point(0, -2)) == Point(-1, -1) );
  CPPUNIT_ASSERT( _negdiag.closest_point(Point(-2, 0)) == Point(-1, -1) );
  CPPUNIT_ASSERT( _negdiag.closest_point(Point(-1, -3)) == Point(-2, -2) );
  CPPUNIT_ASSERT( _neg3diag.closest_point(Point(-2, -2, -2)) == Point(-2, -2, -2) );
  CPPUNIT_ASSERT( _neg3diag.closest_point(Point(0, 0, -4)) == Point(-4./3., -4./3., -4./3.) );
  CPPUNIT_ASSERT( _neg3diag.closest_point(Point(0, -4, -4)) == Point(-8./3., -8./3., -8./3.) );

  CPPUNIT_ASSERT( _negdiag.closest_normal_point(Point(0, 0), result) == true );
  CPPUNIT_ASSERT( result == Point(0, 0) );
  CPPUNIT_ASSERT( _negdiag.closest_normal_point(Point(-1, 0), result) == true );
  CPPUNIT_ASSERT( result == Point(-0.5, -0.5) );
  CPPUNIT_ASSERT( _negdiag.closest_normal_point(Point(0, -2), result) == true );
  CPPUNIT_ASSERT( result == Point(-1, -1) );
  CPPUNIT_ASSERT( _negdiag.closest_normal_point(Point(0, -12), result) == false );
  CPPUNIT_ASSERT( result == Point(-6, -6) );
}

void
LineSegmentTest::containsPointTest()
{
  CPPUNIT_ASSERT( _posx.contains_point(Point(0, 0)) == true );
  CPPUNIT_ASSERT( _posx.contains_point(Point(4, 0)) == true );
  CPPUNIT_ASSERT( _posx.contains_point(Point(3, 3)) == false );
  CPPUNIT_ASSERT( _posx.contains_point(Point(-3, 3)) == false );
  CPPUNIT_ASSERT( _posx.contains_point(Point(-3, -3)) == false );
  CPPUNIT_ASSERT( _posx.contains_point(Point(-.1, 0)) == false );
  CPPUNIT_ASSERT( _pos3x.contains_point(Point(0, 0, 0)) == true );
  CPPUNIT_ASSERT( _pos3x.contains_point(Point(4, 0, 0)) == true );
  CPPUNIT_ASSERT( _pos3x.contains_point(Point(3, 3, 3)) == false );
  CPPUNIT_ASSERT( _pos3x.contains_point(Point(-3, 3, 3)) == false );
  CPPUNIT_ASSERT( _pos3x.contains_point(Point(-3, -3, 3)) == false );
  CPPUNIT_ASSERT( _pos3x.contains_point(Point(-3, -3, -3)) == false );
  CPPUNIT_ASSERT( _pos3x.contains_point(Point(-.1, 0, .1)) == false );
  CPPUNIT_ASSERT( _pos3x.contains_point(Point(.1, 0, 0)) == true );
  CPPUNIT_ASSERT( _pos3x.contains_point(Point(0, 0, .1)) == false );
  CPPUNIT_ASSERT( _pos3x.contains_point(Point(0, .1, .1)) == false );

  CPPUNIT_ASSERT( _posy.contains_point(Point(0, 0)) == true );
  CPPUNIT_ASSERT( _posy.contains_point(Point(0, 4)) == true );
  CPPUNIT_ASSERT( _posy.contains_point(Point(0, 9)) == false );
  CPPUNIT_ASSERT( _posy.contains_point(Point(0, -9)) == false );
  CPPUNIT_ASSERT( _posy.contains_point(Point(3, 3)) == false );
  CPPUNIT_ASSERT( _posy.contains_point(Point(-3, 3)) == false );
  CPPUNIT_ASSERT( _posy.contains_point(Point(3, -3)) == false );
  CPPUNIT_ASSERT( _posy.contains_point(Point(-.1, 0)) == false );

  CPPUNIT_ASSERT( _negy.contains_point(Point(0, 0)) == true );
  CPPUNIT_ASSERT( _negy.contains_point(Point(0, -5)) == true );
  CPPUNIT_ASSERT( _negy.contains_point(Point(4, 0)) == false );
  CPPUNIT_ASSERT( _negy.contains_point(Point(3, 3)) == false );
  CPPUNIT_ASSERT( _negy.contains_point(Point(.1, 0)) == false );
  CPPUNIT_ASSERT( _neg3y.contains_point(Point(0, 0, 0)) == true );
  CPPUNIT_ASSERT( _neg3y.contains_point(Point(0, -5, 0)) == true );
  CPPUNIT_ASSERT( _neg3y.contains_point(Point(4, 0, 0)) == false );
  CPPUNIT_ASSERT( _neg3y.contains_point(Point(0, 4, 0)) == false );
  CPPUNIT_ASSERT( _neg3y.contains_point(Point(0, -9, 0)) == false );
  CPPUNIT_ASSERT( _neg3y.contains_point(Point(3, 3, 3)) == false );
  CPPUNIT_ASSERT( _neg3y.contains_point(Point(-3, 3, 3)) == false );
  CPPUNIT_ASSERT( _neg3y.contains_point(Point(3, -3, 3)) == false );
  CPPUNIT_ASSERT( _neg3y.contains_point(Point(3, -3, -3)) == false );
  CPPUNIT_ASSERT( _neg3y.contains_point(Point(-3, -3, -3)) == false );
  CPPUNIT_ASSERT( _neg3y.contains_point(Point(-.1, 0, .1)) == false );
  CPPUNIT_ASSERT( _neg3y.contains_point(Point(.1, 0, 0)) == false );
  CPPUNIT_ASSERT( _neg3y.contains_point(Point(0, 0, .1)) == false );
  CPPUNIT_ASSERT( _neg3y.contains_point(Point(0, .1, .1)) == false );

  CPPUNIT_ASSERT( _negdiag.contains_point(Point(0, 0)) == true );
  CPPUNIT_ASSERT( _posdiag.contains_point(Point(4, 4)) == true );
  CPPUNIT_ASSERT( _posdiag.contains_point(Point(5, 5)) == true );
  CPPUNIT_ASSERT( _posdiag.contains_point(Point(0, 3)) == false );
  CPPUNIT_ASSERT( _posdiag.contains_point(Point(-.1, 0)) == false );
  CPPUNIT_ASSERT( _pos3diag.contains_point(Point(0, 0, 0)) == true );
  CPPUNIT_ASSERT( _pos3diag.contains_point(Point(0, -5, 0)) == false );
  CPPUNIT_ASSERT( _pos3diag.contains_point(Point(4, 0, 0)) == false );
  CPPUNIT_ASSERT( _pos3diag.contains_point(Point(0, 4, 0)) == false );
  CPPUNIT_ASSERT( _pos3diag.contains_point(Point(0, -9, 0)) == false );
  CPPUNIT_ASSERT( _pos3diag.contains_point(Point(3, 3, 3)) == true );
  CPPUNIT_ASSERT( _pos3diag.contains_point(Point(-3, -3, -3)) == false );
  CPPUNIT_ASSERT( _pos3diag.contains_point(Point(5, 5, 5)) == true );
  CPPUNIT_ASSERT( _pos3diag.contains_point(Point(-5, -5, -5)) == false );
  CPPUNIT_ASSERT( _pos3diag.contains_point(Point(-3, 3, 3)) == false );
  CPPUNIT_ASSERT( _pos3diag.contains_point(Point(3, -3, 3)) == false );
  CPPUNIT_ASSERT( _pos3diag.contains_point(Point(3, -3, -3)) == false );
  CPPUNIT_ASSERT( _pos3diag.contains_point(Point(-.1, 0, .1)) == false );
  CPPUNIT_ASSERT( _pos3diag.contains_point(Point(.1, 0, 0)) == false );
  CPPUNIT_ASSERT( _pos3diag.contains_point(Point(0, 0, .1)) == false );
  CPPUNIT_ASSERT( _pos3diag.contains_point(Point(0, .1, .1)) == false );

  CPPUNIT_ASSERT( _negdiag.contains_point(Point(0, 0)) == true );
  CPPUNIT_ASSERT( _negdiag.contains_point(Point(-5, -5)) == true );
  CPPUNIT_ASSERT( _negdiag.contains_point(Point(0, 3)) == false );
  CPPUNIT_ASSERT( _negdiag.contains_point(Point(-.1, 0)) == false );
  CPPUNIT_ASSERT( _neg3diag.contains_point(Point(0, 0, 0)) == true );
  CPPUNIT_ASSERT( _neg3diag.contains_point(Point(0, -5, 0)) == false );
  CPPUNIT_ASSERT( _neg3diag.contains_point(Point(4, 0, 0)) == false );
  CPPUNIT_ASSERT( _neg3diag.contains_point(Point(0, 4, 0)) == false );
  CPPUNIT_ASSERT( _neg3diag.contains_point(Point(0, -9, 0)) == false );
  CPPUNIT_ASSERT( _neg3diag.contains_point(Point(3, 3, 3)) == false );
  CPPUNIT_ASSERT( _neg3diag.contains_point(Point(-3, -3, -3)) == true );
  CPPUNIT_ASSERT( _neg3diag.contains_point(Point(5, 5, 5)) == false );
  CPPUNIT_ASSERT( _neg3diag.contains_point(Point(-5, -5, -5)) == true );
  CPPUNIT_ASSERT( _neg3diag.contains_point(Point(-3, 3, 3)) == false );
  CPPUNIT_ASSERT( _neg3diag.contains_point(Point(3, -3, 3)) == false );
  CPPUNIT_ASSERT( _neg3diag.contains_point(Point(3, -3, -3)) == false );
  CPPUNIT_ASSERT( _neg3diag.contains_point(Point(-.1, 0, .1)) == false );
  CPPUNIT_ASSERT( _neg3diag.contains_point(Point(.1, 0, 0)) == false );
  CPPUNIT_ASSERT( _neg3diag.contains_point(Point(0, 0, .1)) == false );
  CPPUNIT_ASSERT( _neg3diag.contains_point(Point(0, .1, .1)) == false );
}

void
LineSegmentTest::planeIntersectTest()
{
  Point result;
  Plane xy; xy.xy_plane(1);
  Plane xz; xz.xz_plane(1);
  Plane yz; yz.yz_plane(1);
  Plane diag(Point(0,0,0), Point(1,1,1), Point(-1, 1, 0));

  // Test all the 3D LineSegments against all 4 planes
  CPPUNIT_ASSERT( _pos3x.intersect(xy, result) == false );
  CPPUNIT_ASSERT( _pos3x.intersect(xz, result) == false );
  CPPUNIT_ASSERT( _pos3x.intersect(yz, result) == true );
  CPPUNIT_ASSERT( result == Point(1, 0, 0) );
  CPPUNIT_ASSERT( _pos3x.intersect(diag, result) == true );
  CPPUNIT_ASSERT( result == Point(0, 0, 0) );

  CPPUNIT_ASSERT( _neg3y.intersect(xy, result) == false );
  CPPUNIT_ASSERT( _neg3y.intersect(xz, result) == true );
  CPPUNIT_ASSERT( result == Point(0, 1, 0) );                //Assuming it uses the LineSegment as a line?
  CPPUNIT_ASSERT( _neg3y.intersect(yz, result) == false );
  CPPUNIT_ASSERT( _neg3y.intersect(diag, result) == true );
  CPPUNIT_ASSERT( result == Point(0, 0, 0) );

  CPPUNIT_ASSERT( _pos3diag.intersect(xy, result) == true );
  CPPUNIT_ASSERT( result == Point(1, 1, 1) );
  CPPUNIT_ASSERT( _pos3diag.intersect(xz, result) == true );
  CPPUNIT_ASSERT( result == Point(1, 1, 1) );
  CPPUNIT_ASSERT( _pos3diag.intersect(yz, result) == true );
  CPPUNIT_ASSERT( result == Point(1, 1, 1) );
  CPPUNIT_ASSERT( _pos3diag.intersect(diag, result) == true );

  CPPUNIT_ASSERT( _neg3diag.intersect(xy, result) == true );
  CPPUNIT_ASSERT( result == Point(1, 1, 1) );
  CPPUNIT_ASSERT( _neg3diag.intersect(xz, result) == true );
  CPPUNIT_ASSERT( result == Point(1, 1, 1) );
  CPPUNIT_ASSERT( _neg3diag.intersect(yz, result) == true );
  CPPUNIT_ASSERT( result == Point(1, 1, 1) );
  CPPUNIT_ASSERT( _neg3diag.intersect(diag, result) == true );

  // Make some special lines to test
  LineSegment t1(Point(1, 1, 1), Point(-1, 1, 0));
  CPPUNIT_ASSERT( t1.intersect(diag, result) == true );

  LineSegment t2(Point(1, 1, 1), Point(1, 1, 0));
  CPPUNIT_ASSERT( t2.intersect(diag, result) == true );
  CPPUNIT_ASSERT( result == Point(1, 1, 1) );

  LineSegment t3(Point(1, 1, 1), Point(1, 1, 0));
  CPPUNIT_ASSERT( t3.intersect(diag, result) == true );
  CPPUNIT_ASSERT( result == Point(1, 1, 1) );

  LineSegment t4(Point(1, 1, 1), Point(-4, 5, 9));
  CPPUNIT_ASSERT( t4.intersect(diag, result) == true );
  CPPUNIT_ASSERT( result == Point(1, 1, 1) );

  LineSegment t5(Point(-1, 1, 0), Point(4, -5, -6));
  CPPUNIT_ASSERT( t5.intersect(diag, result) == true );
  CPPUNIT_ASSERT( result == Point(-1, 1, 0) );

  LineSegment t6(Point(0, 0, 0), Point(-94, -5, -6));
  CPPUNIT_ASSERT( t6.intersect(diag, result) == true );
  CPPUNIT_ASSERT( result == Point(0, 0, 0) );

  // Test lines parallel to the plane (first two inside the plane, second two below it)
  LineSegment t7(Point(3, 4, 1), Point(-9, 5, 1));
  CPPUNIT_ASSERT( t7.intersect(xy, result) == true );

  LineSegment t8(Point(-3, 4, 1), Point(9, -5, 1));
  CPPUNIT_ASSERT( t8.intersect(xy, result) == true );

  LineSegment t9(Point(3, 4, 0), Point(-9, 5, 0));
  CPPUNIT_ASSERT( t9.intersect(xy, result) == false );

  LineSegment t10(Point(-3, 4, 0), Point(9, -5, 0));
  CPPUNIT_ASSERT( t10.intersect(xy, result) == false );
}

void
LineSegmentTest::lineIntersectTest()
{
  Point result;
  CPPUNIT_ASSERT( _posx.intersect(_negy, result) == true );
  CPPUNIT_ASSERT( result == Point(0, 0) );
  CPPUNIT_ASSERT( _posdiag.intersect(_negy, result) == true );
  CPPUNIT_ASSERT( result == Point(0, 0) );
  CPPUNIT_ASSERT( _negdiag.intersect(_negy, result) == true );
  CPPUNIT_ASSERT( result == Point(0, 0) );
  CPPUNIT_ASSERT( _posdiag.intersect(_negdiag, result) == true );

  CPPUNIT_ASSERT( _pos3x.intersect(_neg3y, result) == true );
  CPPUNIT_ASSERT( result == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _pos3diag.intersect(_neg3y, result) == true );
  CPPUNIT_ASSERT( result == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _neg3diag.intersect(_neg3y, result) == true );
  CPPUNIT_ASSERT( result == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _pos3diag.intersect(_neg3diag, result) == true );

  LineSegment t1(Point(0, 1, 0), Point(1, 1, 0));
  CPPUNIT_ASSERT( t1.intersect(_pos3x, result) == false );
  CPPUNIT_ASSERT( t1.intersect(_pos3diag, result) == false );
  CPPUNIT_ASSERT( t1.intersect(_neg3diag, result) == false );
  // The lines formed by the line segments do intersect but not the line segments themselves
  CPPUNIT_ASSERT( t1.intersect(_neg3y, result) == false );

  LineSegment t2(Point(0, 0, 0), Point(0, 0, 1));
  /*bool ans = */ t1.intersect(t2, result);
  CPPUNIT_ASSERT( t1.intersect(t2, result) == false );
  CPPUNIT_ASSERT( t2.intersect(_pos3diag, result) == true );
  CPPUNIT_ASSERT( result == Point(0, 0, 0) );
  CPPUNIT_ASSERT( t2.intersect(_neg3diag, result) == true );
  CPPUNIT_ASSERT( result == Point(0, 0, 0) );

  LineSegment t3(Point(1, 1, 1), Point(4, -5, 7));
  CPPUNIT_ASSERT( t3.intersect(_pos3diag, result) == true );
  CPPUNIT_ASSERT( result == Point(1, 1, 1) );
  CPPUNIT_ASSERT( t3.intersect(_neg3diag, result) == false );
  CPPUNIT_ASSERT( t3.intersect(t2, result) == false );

  LineSegment t4(Point(-3, -3, -3), Point(4, -5, 7));
  CPPUNIT_ASSERT( t4.intersect(_pos3diag, result) == false );
  CPPUNIT_ASSERT( t4.intersect(_neg3diag, result) == true );
  CPPUNIT_ASSERT( result == Point(-3, -3, -3) );
  CPPUNIT_ASSERT( t4.intersect(t3, result) == true );
  CPPUNIT_ASSERT( result == Point(4, -5, 7) );

  LineSegment t5(Point(1, 0, 0), Point(-4, 0, 0));
  CPPUNIT_ASSERT( t5.intersect(_pos3x, result) == true );
  CPPUNIT_ASSERT( t5.intersect(t4, result) == false );
  CPPUNIT_ASSERT( t5.intersect(_neg3y, result) == true );
  CPPUNIT_ASSERT( result == Point(0, 0, 0) );

  // Switch some of the a.intersect(b) tests to b.intersect(a) to make sure we get the same result
  CPPUNIT_ASSERT( t2.intersect(t1, result) == false );
  CPPUNIT_ASSERT( _pos3diag.intersect(t4, result) == false );
  CPPUNIT_ASSERT( _neg3diag.intersect(t4, result) == true );
  CPPUNIT_ASSERT( result == Point(-3, -3, -3) );
  CPPUNIT_ASSERT( t3.intersect(t4, result) == true );
  CPPUNIT_ASSERT( result == Point(4, -5, 7) );
  CPPUNIT_ASSERT( _neg3y.intersect(_pos3x, result) == true );
  CPPUNIT_ASSERT( result == Point(0, 0, 0) );
  CPPUNIT_ASSERT( _pos3x.intersect(t5, result) == true );
  CPPUNIT_ASSERT( t4.intersect(t5, result) == false );
  CPPUNIT_ASSERT( _neg3y.intersect(t5, result) == true );
  CPPUNIT_ASSERT( result == Point(0, 0, 0) );
}
