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
#include "MoosePartitioner.h"

/**
 * Partitions a mesh randomly using element ids as the seed for the generator.
 */
class RandomPartitioner : public MoosePartitioner
{
public:
  RandomPartitioner(const InputParameters & params);
  virtual ~RandomPartitioner();

  static InputParameters validParams();

  virtual std::unique_ptr<Partitioner> clone() const override;

protected:
  virtual void _do_partition(MeshBase & mesh, const unsigned int n) override;

  /// Total number of processors
  const unsigned int _num_procs;
};
