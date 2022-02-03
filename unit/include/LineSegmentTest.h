//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "gtest_include.h"

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
