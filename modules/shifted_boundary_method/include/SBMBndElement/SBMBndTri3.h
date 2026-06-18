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
/// Triangle is listed first so it is initialized before SBMBndElementBase;
/// the base class constructor then receives the unit normal from
/// Triangle::normal() (see the .C file).
class SBMBndTri3 : public Triangle, public SBMBndElementBase
{
public:
  /// Constructor
  explicit SBMBndTri3(const Elem * elem);

  // Resolve name ambiguity between base classes.
  using SBMBndElementBase::normal;
  using Triangle::computeBoundingBall;
  using Triangle::intersect;
};
