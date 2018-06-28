//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PARTITIONERWEIGHT_H
#define PARTITIONERWEIGHT_H

// MOOSE includes
#include "MooseEnum.h"
#include "PetscMatPartitioner.h"

class PartitionerWeight;
class MooseMesh;

template <>
InputParameters validParams<PartitionerWeight>();

/**
 * Partitions a mesh based on a weighted graph
 */
class PartitionerWeight : public PetscMatPartitioner
{
public:
  PartitionerWeight(const InputParameters & params);

  virtual std::unique_ptr<Partitioner> clone() const override;

  virtual dof_id_type computeElementWeight(Elem & elm) override;

  virtual dof_id_type computeSideWeight(Elem & elem, Elem & side) override;
};

#endif /* PARTITIONERWEIGHT_H */
