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

  Point circumcenter(const Elem * elem);

  std::unique_ptr<MeshBase> generate() override;

protected:
  std::unique_ptr<MeshBase> & _input;

  // Angular tolerance for determining colinearity of boundary sides when detecting primal boundary
  // vertices
  Real _boundary_node_angular_tol;

  // Dual type; either voronoi (dual nodes at primal element circumcenters) or barycentric (dual
  // nodes at primal element centroids)
  MooseEnum _dual_mesh_type;
};