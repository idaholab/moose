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
class PatchSidesetGenerator;

template <>
InputParameters validParams<PatchSidesetGenerator>();

/**
 * Subdivides a sidesets into smaller patches each of which is going
 * to be a new patch
 */
class PatchSidesetGenerator : public MeshGenerator
{
public:
  PatchSidesetGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// returns the name of the _n_patches subdivisions derived from _sideset
  std::vector<BoundaryName> sidesetNameHelper(const std::string & base_name) const;

  Elem * boundaryElementHelper(std::unique_ptr<libMesh::ReplicatedMesh> & mesh, int type) const;

  void setPartitionerHelper(std::unique_ptr<libMesh::ReplicatedMesh> & mesh) const;

  std::unique_ptr<MeshBase> & _input;

  /// the number of patches that this sideset generator divides _sideset into
  unsigned int _n_patches;

  /// The sideset that will be subdivided
  const boundary_id_type & _sideset;

  /// the name of the partitioner being used
  MooseEnum _partitioner_name;
};
