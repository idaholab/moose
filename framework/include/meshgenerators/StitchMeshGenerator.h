//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "StitchMeshGeneratorBase.h"
#include "libmesh/replicated_mesh.h"

/**
 * Allows multiple mesh files to be "stitched" together to form a single mesh.
 */
class StitchMeshGenerator : public StitchMeshGeneratorBase
{
public:
  static InputParameters validParams();

  StitchMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  // Holds pointers to the pointers to the meshes.
  std::vector<std::unique_ptr<MeshBase> *> _mesh_ptrs;

  /// The mesh generator inputs to read
  const std::vector<MeshGeneratorName> & _input_names;

  /// Whether to renumber all boundaries in stitched meshes to prevent accidental merging
  /// of sidesets with the same id
  const bool _prevent_boundary_ids_overlap;

  /// Whether to merge boundaries if they have the same name but different boundary IDs
  const bool _merge_boundaries_with_same_name;
};
