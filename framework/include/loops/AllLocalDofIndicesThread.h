//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "MooseTypes.h"

#include "libmesh/elem_range.h"
#include "libmesh/parallel_object.h"

// Forward declare classes in libMesh
namespace libMesh
{
class System;
class DofMap;
}
class SubProblem;

/**
 * Grab all the (possibly semi)local dof indices for the variables passed in, in the system passed
 * in.
 */
class AllLocalDofIndicesThread : public libMesh::ParallelObject
{
public:
  AllLocalDofIndicesThread(SubProblem & problem,
                           std::vector<std::string> vars,
                           bool include_semilocal = false);
  // Splitting Constructor
  AllLocalDofIndicesThread(AllLocalDofIndicesThread & x, libMesh::Threads::split split);

  void operator()(const libMesh::ConstElemRange & range);

  void join(const AllLocalDofIndicesThread & y);

  const std::set<dof_id_type> & getDofIndices() const { return _all_dof_indices; }

  void dofIndicesSetUnion();

protected:
  SubProblem & _problem;
  libMesh::System * _sys;
  std::vector<unsigned int> _var_numbers;

  /// Whether to include semilocal dof indices
  const bool _include_semilocal;

  THREAD_ID _tid;

  std::set<dof_id_type> _all_dof_indices;
};
