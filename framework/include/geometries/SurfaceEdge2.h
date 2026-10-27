//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SurfaceElement.h"
#include "LineSegment.h"

/// Derived class for 2-node edge surface elements (Edge2).
class SurfaceEdge2 : public LineSegment, public SurfaceElement
{
public:
  /// Constructor
  explicit SurfaceEdge2(const Elem * elem);

  // Resolve name ambiguity between base classes. The using-declaration also keeps
  // LineSegment's two-argument intersect() overloads reachable, which the override
  // below would otherwise hide.
  using LineSegment::intersect;
  using SurfaceElement::normal;

  // Satisfy the SurfaceElement interface by forwarding to the LineSegment primitive.
  bool intersect(const LineSegment & line_segment) const override
  {
    return LineSegment::intersect(line_segment);
  }
  Ball computeBoundingBall() const override { return LineSegment::computeBoundingBall(); }
};
