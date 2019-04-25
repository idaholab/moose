//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"

class WorkBalance;

template <>
InputParameters validParams<WorkBalance>();

/**
 * Compute several metrics for each MPI process.
 *
 * Note: this is somewhat expensive.  It does loops over elements, sides and nodes
 */
class WorkBalance : public GeneralVectorPostprocessor
{
public:
  WorkBalance(const InputParameters & parameters);

  enum SystemEnum
  {
    // Ordered this way because NL is always system 0 and Aux is 1
    ALL = -1,
    NL,
    AUX,
  };

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  /// The system to count DoFs from
  int _system;

  bool _sync_to_all_procs;

  dof_id_type _local_num_elems;
  dof_id_type _local_num_nodes;
  dof_id_type _local_num_dofs;
  dof_id_type _local_num_partition_sides;
  Real _local_partition_surface_area;

  VectorPostprocessorValue & _pid;
  VectorPostprocessorValue & _num_elems;
  VectorPostprocessorValue & _num_nodes;
  VectorPostprocessorValue & _num_dofs;
  VectorPostprocessorValue & _num_partition_sides;
  VectorPostprocessorValue & _partition_surface_area;
};

