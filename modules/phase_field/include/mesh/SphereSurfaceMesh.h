//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPHERESURFACEMESH_H
#define SPHERESURFACEMESH_H

#include "MooseMesh.h"
#include "libmesh/point.h"

class SphereSurfaceMesh;

template <>
InputParameters validParams<SphereSurfaceMesh>();

/**
 * Create a sphere surface mesh based on the recursive subdovision of the faces
 * of a regular icosahedron.
 */
class SphereSurfaceMesh : public MooseMesh
{
public:
  SphereSurfaceMesh(const InputParameters & parameters);
  SphereSurfaceMesh(const SphereSurfaceMesh & other_mesh) = default;

  // No copy
  SphereSurfaceMesh & operator=(const SphereSurfaceMesh & other_mesh) = delete;
  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;

protected:
  /// sphere radius
  const Real _radius;

  //// Sphere center
  const Point _center;

  /// recursion levels for triangle subdivision
  const unsigned int _depth;
};

#endif // SPHERESURFACEMESH_H
