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
 * Partitions a mesh using external petsc partitioners such as parmetis, ptscotch, chaco, party,
 * etc.
 */
class PetscExternalPartitioner : public MoosePartitioner
{
public:
  static InputParameters validParams();

  PetscExternalPartitioner(const InputParameters & params);

  virtual std::unique_ptr<Partitioner> clone() const override;

  virtual dof_id_type computeElementWeight(Elem & elm);

  virtual dof_id_type computeSideWeight(Elem & elem, unsigned int side);

  using Partitioner::partition;

  virtual void partition(MeshBase & mesh, const unsigned int n) override;

  bool applySideWeight() { return _apply_side_weight; }

  bool applyElementEeight() { return _apply_element_weight; }

  static void partitionGraph(const Parallel::Communicator & comm,
                             const std::vector<std::vector<dof_id_type>> & graph,
                             const std::vector<dof_id_type> & elem_weights,
                             const std::vector<dof_id_type> & side_weights,
                             const dof_id_type num_parts,
                             const dof_id_type num_parts_per_compute_node,
                             const std::string & part_package,
                             std::vector<dof_id_type> & partition);

  /**
   * Called immediately before partitioning
   */
  virtual void initialize(MeshBase & /* mesh */){};

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
  processor_id_type _num_parts_per_compute_node;
};
