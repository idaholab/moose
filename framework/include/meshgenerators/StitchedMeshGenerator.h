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
 * Allows multiple mesh files to be "stitched" together to form a single mesh.
 */
class StitchedMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  StitchedMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  // Holds pointers to the pointers to the meshes.
  std::vector<std::unique_ptr<MeshBase> *> _mesh_ptrs;

  /// The mesh generator inputs to read
  const std::vector<MeshGeneratorName> & _input_names;

  /// Whether or not to clear (remove) the stitched boundary IDs
  const bool & _clear_stitched_boundary_ids;

  /// A transformed version of _stitch_boundaries into a more logical "pairwise" structure
  std::vector<std::vector<std::string>> _stitch_boundaries_pairs;

  /// Type of algorithm used to find matching nodes (binary or exhaustive)
  MooseEnum _algorithm;
};
