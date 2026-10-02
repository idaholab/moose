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
  /**
   * Checks whether the given boundary ID has neighbors after a stitch.
   *
   * @param[in] mesh  Mesh
   * @param[in] boundary_id  Boundary ID corresponding to the stitch surface
   * @param[in] active_side_list  List of BC tuples (elem,side,boundary ID)
   * @param[in] mg_name  Mesh generator name of secondary mesh in a stitch step
   */
  void
  checkFullBoundaryHasNeighbor(const MeshBase & mesh,
                               const BoundaryID boundary_id,
                               const std::vector<libMesh::BoundaryInfo::BCTuple> & active_side_list,
                               const MeshGeneratorName & mg_name) const;

  // Holds pointers to the pointers to the meshes.
  std::vector<std::unique_ptr<MeshBase> *> _mesh_ptrs;

  /// The mesh generator inputs to read
  const std::vector<MeshGeneratorName> & _input_names;

  /// Whether to renumber all boundaries in stitched meshes to prevent accidental merging
  /// of sidesets with the same id
  const bool _prevent_boundary_ids_overlap;

  /// Whether to merge boundaries if they have the same name but different boundary IDs
  const bool _merge_boundaries_with_same_name;

  /// Whether the boundaries in each pair must stitch along the whole of each boundary
  const bool _require_boundaries_fully_stitch;
};
