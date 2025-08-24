//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "CSGPlane.h"

#include "MooseUnitUtils.h"

using namespace CSG;

/// Tests CSGSurface::getHalfspaceFromPoint
TEST(CSGSurfaceTest, testGetHalfspaceFromPoint)
{
  // Surface in the +x direction
  const std::array<Point, 3> points{Point(1, 0, 0), Point(1, 1, 0), Point(1, 0, 1)};
  CSGPlane plane("plus_x", points[0], points[1], points[2]);

  // Centroid of the points
  Point centroid;
  for (const auto & point : points)
    centroid += point;
  centroid /= 3;

  // Positive halfspace
  {
    const auto point = centroid + Point(-1, 0, 0);
    ASSERT_EQ(CSGSurface::Halfspace::POSITIVE, plane.getHalfspaceFromPoint(point));
  }
  // Negative halfspace
  {
    const auto point = centroid + Point(1, 0, 0);
    ASSERT_EQ(CSGSurface::Halfspace::NEGATIVE, plane.getHalfspaceFromPoint(point));
  }
  // Lies on the plane; error
  {
    Moose::UnitUtils::assertThrows([&centroid, &plane]() { plane.getHalfspaceFromPoint(centroid); },
                                   "ambiguously defined halfspace");
  }
}
