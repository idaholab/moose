//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"

namespace libMesh
{
class ReplicatedMesh;
}

/**
 * Reads an arbitrary set of meshes and attempts to "stitch" (join) them
 * along boundaries.
 */
class StitchedMesh : public MooseMesh
{
public:
  static InputParameters validParams();

  StitchedMesh(const InputParameters & parameters);
  StitchedMesh(const StitchedMesh & other_mesh);

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;

protected:
  /// The mesh files to read
  const std::vector<MeshFileName> & _files;

  /// Whether or not to clear (remove) the stitched boundary IDs
  const bool & _clear_stitched_boundary_ids;

  /// The raw data from the input file
  const std::vector<BoundaryName> & _stitch_boundaries;

  /// A transformed version of _stitch_boundaries into a more logical "pairwise" structure
  std::vector<std::pair<BoundaryName, BoundaryName>> _stitch_boundaries_pairs;

  // Pointer to the original "real" mesh to be stitched into
  ReplicatedMesh * _original_mesh;

  /// The meshes to be stitched together.  The first entry will be the "real" mesh
  std::vector<std::unique_ptr<ReplicatedMesh>> _meshes;
};
