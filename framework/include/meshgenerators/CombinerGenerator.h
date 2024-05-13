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
#include "libmesh/replicated_mesh.h"
#include "MooseEnum.h"

/**
 * Collects multiple meshes into a single (unconnected) mesh
 */
class CombinerGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  CombinerGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

  /// Fill the offset positions for each mesh
  void fillPositions();

protected:
  /**
   * Helper funciton for copying one mesh into another
   */
  void copyIntoMesh(UnstructuredMesh & destination, const UnstructuredMesh & source);

  // Holds pointers to the mesh smart pointers (to be populated later).
  const std::vector<std::unique_ptr<MeshBase> *> _meshes;

  /// The mesh generators to use
  const std::vector<MeshGeneratorName> & _input_names;

  /// The (offsets) positions for each mesh
  std::vector<Point> _positions;

  /// Boolean to control whether to prevent merging subdomains
  const bool _avoid_merging_subdomains;
  /// Boolean to control whether to prevent merging boundaries
  const bool _avoid_merging_boundaries;
};
