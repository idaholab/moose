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
#include "PetscExternalPartitioner.h"

class MooseMesh;

/**
 * Partitions a mesh based on a weighted graph
 */
class PartitionerWeightTest : public PetscExternalPartitioner
{
public:
  static InputParameters validParams();

  PartitionerWeightTest(const InputParameters & params);

  virtual std::unique_ptr<Partitioner> clone() const override;

  virtual dof_id_type computeElementWeight(Elem & elm) override;

  virtual dof_id_type computeSideWeight(Elem & elem, unsigned int side) override;
};
