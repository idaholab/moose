//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PETSCEXTERNALPARTITIONER_H
#define PETSCEXTERNALPARTITIONER_H

// MOOSE includes
#include "MooseEnum.h"
#include "MoosePartitioner.h"

class PetscExternalPartitioner;
class MooseMesh;

template <>
InputParameters validParams<PetscExternalPartitioner>();

/**
 * Partitions a mesh using a regular grid.
 */
class PetscExternalPartitioner : public MoosePartitioner
{
public:
  PetscExternalPartitioner(const InputParameters & params);

  virtual std::unique_ptr<Partitioner> clone() const override;

  virtual dof_id_type computeElementWeight(Elem & elm);

  virtual dof_id_type computeSideWeight(Elem & elem, unsigned int side);

protected:
  virtual void _do_partition(MeshBase & mesh, const unsigned int n) override;

private:
  /*
   * Do a partition before we call the partitioner
   * It should be used if the mesh is unpartitioned or the number of parts
   * does not equal to the number of processors
   */
  void preLinearPartition(MeshBase & mesh);

  std::string _part_package;
  bool _apply_element_weight;
  bool _apply_side_weight;
};

#endif /* PETSCEXTERNALPARTITIONER_H */
