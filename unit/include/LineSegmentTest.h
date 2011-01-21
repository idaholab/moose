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

#ifndef LINESEGMENTTEST_H
#define LINESEGMENTTEST_H

//CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

// Moose includes
#include "LineSegment.h"

class LineSegmentTest : public CppUnit::TestFixture 
{

  CPPUNIT_TEST_SUITE( LineSegmentTest );

  CPPUNIT_TEST( closestPointTest );
  CPPUNIT_TEST( closestNormalPointTest );
  CPPUNIT_TEST( containsPointTest );
  CPPUNIT_TEST( planeIntersectTest );
  CPPUNIT_TEST( lineIntersectTest );
  
  CPPUNIT_TEST_SUITE_END();

public:
  LineSegmentTest();
  ~LineSegmentTest();

  void closestPointTest();
  void closestNormalPointTest();
  void containsPointTest();
  void planeIntersectTest();
  void lineIntersectTest();

private:
  LineSegment _posx;
  LineSegment _posy;
  LineSegment _negy;
  LineSegment _posdiag;
  LineSegment _negdiag;

  LineSegment _pos3x;
  LineSegment _neg3y;
  LineSegment _pos3diag;
  LineSegment _neg3diag;
};

#endif  // LINESEGMENTTEST_H
