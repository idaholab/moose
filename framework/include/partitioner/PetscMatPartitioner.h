//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PETSCMATPARTITIONER_H
#define PETSCMATPARTITIONER_H

// MOOSE includes
#include "MooseEnum.h"
#include "MoosePartitioner.h"

class PetscMatPartitioner;
class MooseMesh;

template <>
InputParameters validParams<PetscMatPartitioner>();

/**
 * Partitions a mesh using a regular grid.
 */
class PetscMatPartitioner : public MoosePartitioner
{
public:
  PetscMatPartitioner(const InputParameters & params);

  virtual std::unique_ptr<Partitioner> clone() const override;

  virtual dof_id_type computeElementWeight(Elem & elm);

  virtual dof_id_type computeSideWeight(Elem & elem, Elem & side);

protected:
  virtual void _do_partition(MeshBase & mesh, const unsigned int n) override;

private:
  std::string _part_package;
  bool _apply_element_weight;
  bool _apply_side_weight;
};

#endif /* PETSCMATPARTITIONER_H */
