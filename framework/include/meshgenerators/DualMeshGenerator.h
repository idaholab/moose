//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"

/**
 * MeshGenerator for creating dual mesh
 */
class DualMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  DualMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  std::unique_ptr<MeshBase> & _input;

  /// Determines the type of dual mesh to generate: Voronoi or Barycentric.
  MooseEnum _dual_mesh_type;

  /// Angular tolerance, in radians, for determining colinearity of boundary sides when detecting primal boundary
  /// vertices. If the sides make an angle greater than this, their shared point is considered a vertex and is added to the dual mesh.
  Real _boundary_node_angular_tol;

  /// Relative tolerance for geometric determinations, scaled by the primal mesh's bounding box size.
  /// For Voronoi duals, determines the size of the circumscribing square.
  Real _geometry_relative_tol;
};
