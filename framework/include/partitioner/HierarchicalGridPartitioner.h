//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef HIERARCHICALGRIDPARTITIONER_H
#define HIERARCHICALGRIDPARTITIONER_H

// MOOSE includes
#include "MooseEnum.h"
#include "MoosePartitioner.h"

class HierarchicalGridPartitioner;
class MooseMesh;

namespace libMesh
{
class SubdomainPartitioner;
}

template <>
InputParameters validParams<HierarchicalGridPartitioner>();

/**
 * Partitions a mesh using a regular grid.
 */
class HierarchicalGridPartitioner : public MoosePartitioner
{
public:
  HierarchicalGridPartitioner(const InputParameters & params);
  virtual ~HierarchicalGridPartitioner();

  virtual std::unique_ptr<Partitioner> clone() const override;

protected:
  virtual void _do_partition(MeshBase & mesh, const unsigned int n) override;

  MooseMesh & _mesh;

  unsigned int _nx_nodes;
  unsigned int _ny_nodes;
  unsigned int _nz_nodes;
  unsigned int _nx_procs;
  unsigned int _ny_procs;
  unsigned int _nz_procs;
};

#endif /* HIERARCHICALGRIDPARTITIONER_H */
