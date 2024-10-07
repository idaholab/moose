//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseEnum.h"
#include "MoosePartitioner.h"

class MooseMesh;

/**
 * Partitions a mesh by:
 * - reading a file of N positions
 * - forming n_process groups of positions that are clustered using Lloyd's algorithm
 * - assigning elements of the mesh based on the group of their nearest position
 */
class NearestPositionsPartitioner : public MoosePartitioner
{
public:
  NearestPositionsPartitioner(const InputParameters & params);
  virtual ~NearestPositionsPartitioner();

  static InputParameters validParams();

  virtual std::unique_ptr<Partitioner> clone() const override;

protected:
  virtual void _do_partition(MeshBase & mesh, const unsigned int n) override;

  MooseMesh & _mesh;
};
