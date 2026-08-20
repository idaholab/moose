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
#include "Triangle.h"

/// Derived class for 3-node triangular surface elements (Tri3).
class SurfaceTri3 : public Triangle, public SurfaceElement
{
public:
  /// Constructor
  explicit SurfaceTri3(const Elem * elem);

  // Resolve name ambiguity between base classes. The using-declaration also keeps
  // Triangle's two-argument intersect() overload reachable, which the override
  // below would otherwise hide.
  using SurfaceElement::normal;
  using Triangle::intersect;

  // Satisfy the SurfaceElement interface by forwarding to the Triangle primitive.
  bool intersect(const LineSegment & line_segment) const override
  {
    return Triangle::intersect(line_segment);
  }
  Ball computeBoundingBall() const override { return Triangle::computeBoundingBall(); }
};
