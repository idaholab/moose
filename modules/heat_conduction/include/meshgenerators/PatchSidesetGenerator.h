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

// Forward declarations

/**
 * Subdivides a sidesets into smaller patches each of which is going
 * to be a new patch
 */
class PatchSidesetGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  PatchSidesetGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

  unsigned int nPatches() const { return _n_patches; }

protected:
  /// returns the name of the _n_patches subdivisions derived from _sideset
  std::vector<BoundaryName> sidesetNameHelper(const std::string & base_name) const;

  Elem * boundaryElementHelper(MeshBase & mesh, libMesh::ElemType type) const;

  /// a function for implementing custom partitioning
  void partition(MeshBase & mesh);

  /**
   * Checks partitions and makes sure every partition has at least one elem.
   * If a partition is empty, it's removed and the remaining ones are "renamed"
   */
  void checkPartitionAndCompress(MeshBase & mesh);

  std::unique_ptr<MeshBase> & _input;

  /// dimensionality of the sidesets to partition
  unsigned int _dim;

  /// the number of patches that this sideset generator divides _sideset into
  unsigned int _n_patches;

  /// The sideset that will be subdivided
  const BoundaryName & _sideset_name;

  /// the name of the partitioner being used
  MooseEnum _partitioner_name;

  /// The sideset that will be subdivided
  subdomain_id_type _sideset;

  /// number of elements of the boundary mesh
  dof_id_type _n_boundary_mesh_elems;
};
