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

/// Derived class for 2-node edge surface elements (Edge2)
/// LineSegment is listed first so it is initialized before SurfaceElement;
/// the base class constructor then receives the unit normal from
/// LineSegment::normal() (see the .C file).
class SurfaceEdge2 : public LineSegment, public SurfaceElement
{
public:
  /// Constructor
  explicit SurfaceEdge2(const Elem * elem);

  // Resolve name ambiguity between base classes.
  using LineSegment::computeBoundingBall;
  using LineSegment::intersect;
  using SurfaceElement::normal;
};
