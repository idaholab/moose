//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MoosePartitioner.h"

/**
 * Partitions a mesh based on the partitioning of the other mesh
 */
class CopyMeshPartitioner : public MoosePartitioner
{
public:
  CopyMeshPartitioner(const InputParameters & params);
  virtual ~CopyMeshPartitioner();

  static InputParameters validParams();

  virtual std::unique_ptr<Partitioner> clone() const override;

protected:
  virtual void _do_partition(MeshBase & mesh, const unsigned int n) override;

  /// A pointer to the mesh to copy the partitioning from
  const MooseMesh * _base_mesh;
};
