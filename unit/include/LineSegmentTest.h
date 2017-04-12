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

#include "gtest/gtest.h"

#include "LineSegment.h"

class LineSegmentTest : public ::testing::Test
{
public:
  LineSegmentTest()
    : _posx(Point(0, 0), Point(5, 0)),
      _posy(Point(0, 0), Point(0, 5)),
      _negy(Point(0, 0), Point(0, -5)),
      _posdiag(Point(0, 0), Point(5, 5)),
      _negdiag(Point(0, 0), Point(-5, -5)),
      _pos3x(Point(0, 0, 0), Point(5, 0, 0)),
      _neg3y(Point(0, 0, 0), Point(0, -5, 0)),
      _pos3diag(Point(0, 0, 0), Point(5, 5, 5)),
      _neg3diag(Point(0, 0, 0), Point(-5, -5, -5))
  {
  }

protected:
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

#endif // LINESEGMENTTEST_H
