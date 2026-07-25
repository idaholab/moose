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

/// Derived class for 3-node triangular surface elements (Tri3)
/// Triangle is listed first so it is initialized before SurfaceElement;
/// the base class constructor then receives the unit normal from
/// Triangle::normal() (see the .C file).
class SurfaceTri3 : public Triangle, public SurfaceElement
{
public:
  /// Constructor
  explicit SurfaceTri3(const Elem * elem);

  // Resolve name ambiguity between base classes.
  using SurfaceElement::normal;
  using Triangle::computeBoundingBall;
  using Triangle::intersect;
};
