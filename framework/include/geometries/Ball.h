//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/point.h"
#include "libmesh/sphere.h"

using namespace libMesh;

/**
 * ball primitive.
 *
 * - In 2D: represents a circle
 * - In 3D: represents a sphere
 */
class Ball
{
public:
  Ball(const Point & c, Real r) : _c(c), _r(r) {}

  const Point & center() const { return _c; }
  Real radius() const { return _r; }

#if LIBMESH_DIM > 2
  libMesh::Sphere toSphere() const { return libMesh::Sphere(_c, _r); }
#endif

private:
  Point _c;
  Real _r;
};
