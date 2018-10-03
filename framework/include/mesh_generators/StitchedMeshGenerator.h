//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef STITCHEDMESHGENERATOR_H
#define STITCHEDMESHGENERATOR_H

#include "MeshGenerator.h"

// Forward declarations
class StitchedMeshGenerator;

template <>
InputParameters validParams<StitchedMeshGenerator>();

/**
 * Generates individual elements given a list of nodal positions
 */
class StitchedMeshGenerator : public MeshGenerator
{
public:
  StitchedMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate();

protected:
  /// The mesh files to read
  const std::vector<MeshFileName> & _files_names;

  /// Whether or not to clear (remove) the stitched boundary IDs
  const bool & _clear_stitched_boundary_ids;

  /// A transformed version of _stitch_boundaries into a more logical "pairwise" structure
  std::vector<std::vector<std::string>> _stitch_boundaries_pairs;

  /// Pointer to the original "real" mesh to be stitched into
  ///ReplicatedMesh * _original_mesh;

  /// The meshes to be stitched together.  The first entry will be the "real" mesh
  std::vector<std::unique_ptr<ReplicatedMesh>> _meshes;
};

#endif // STITCHEDMESHGENERATOR_H

