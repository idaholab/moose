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

  /**
   * Copy the contents of one mesh into another, taking into account
   * the merging options specified in class attributes.
   * @param destination The mesh to copy into
   * @param source The mesh to copy from
   */
  void copyIntoMesh(UnstructuredMesh & destination, const UnstructuredMesh & source);
};
