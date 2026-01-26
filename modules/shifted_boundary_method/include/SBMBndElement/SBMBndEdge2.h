//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SBMBndElementBase.h"

/// Derived class for 2-node edge elements
class SBMBndEdge2 : public SBMBndElementBase
{
public:
  /// Constructor
  explicit SBMBndEdge2(const Elem * elem);

  /// Check if a line segment intersects this edge
  bool intercepted(const LineSegment & line_segment) const override;

  /// Compute the bounding ball (circle) of the edge
  Ball computeBoundingBall() const override;

protected:
  /// Compute the outward normal vector of the edge (2D)
  const Point computeNormal() const override;
};
