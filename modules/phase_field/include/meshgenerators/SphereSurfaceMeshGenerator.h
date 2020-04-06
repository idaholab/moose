//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"
#include "libmesh/point.h"

/**
 * Create a sphere surface mesh based on the recursive subdivision of the faces
 * of a regular icosahedron.
 */
class SphereSurfaceMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  SphereSurfaceMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// sphere radius
  const Real _radius;

  //// Sphere center
  const Point _center;

  /// recursion levels for triangle subdivision
  const unsigned int _depth;
};
