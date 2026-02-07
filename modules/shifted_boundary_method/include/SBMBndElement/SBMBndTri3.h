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
#include "Triangle.h"

/// Derived class for 3-node triangular elements (Tri3)
class SBMBndTri3 : public SBMBndElementBase, public Triangle
{
public:
  /// Constructor
  explicit SBMBndTri3(const Elem * elem);

  using SBMBndElementBase::normal;
  using Triangle::computeBoundingBall;
  using Triangle::intersect;

protected:
  /// Compute the normal vector of the triangle
  const Point computeNormal() const override;
};
