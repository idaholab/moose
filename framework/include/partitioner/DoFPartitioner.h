//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DOFPARTITIONER_H
#define DOFPARTITIONER_H

#include "PetscExternalPartitioner.h"

class DoFPartitioner;
class MooseMesh;

template <>
InputParameters validParams<DoFPartitioner>();

/**
 * Partitions a mesh based on the DoFs defined on each element
 */
class DoFPartitioner : public PetscExternalPartitioner
{
public:
  DoFPartitioner(const InputParameters & params);

  virtual std::unique_ptr<Partitioner> clone() const override;

  virtual dof_id_type computeElementWeight(Elem & elm) override;
};

#endif /* DOFPARTITIONER_H */
