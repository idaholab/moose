//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "MooseMesh.h"
#include "MathFVUtils.h"

// libmesh
#include "libmesh/elem_range.h"
#include "libmesh/threads.h"

class FEProblemBase;
class LinearFVElementalKernel;

/**
 * Adds contributions from volumetric terms discretized using the finite volume method to
 * the matrix and right hand side of a linear system.
 */
class ComputeLinearFVElementalThread
{
public:
  /**
   * Class constructor.
   * @param fe_problem Reference to the problem
   * @param linear_system_num The number of the linear system which is assembled by this thread
   * @param mode Computation mode (rhs, matrix or both)
   * @param vector_tags The vector tags this thread should contribute to. These are used to
   * query the warehouse for the objects that should contribute to the right hand side.
   * @param matrix_tags The matrix tags this thread should contribute to. These are used to
   * query the warehouse for the objects that should contribute to the matrix.
   */
  ComputeLinearFVElementalThread(FEProblemBase & fe_problem,
                                 const unsigned int linear_system_num,
                                 const std::set<TagID> & vector_tags,
                                 const std::set<TagID> & matrix_tags);
  /**
   * Splitting constructor.
   * @param x Reference to the other thread
   * @param split The thread split
   */
  ComputeLinearFVElementalThread(ComputeLinearFVElementalThread & x, Threads::split split);

  using ElemInfoRange = StoredRange<MooseMesh::const_elem_info_iterator, const ElemInfo *>;
  /// Operator which is used to execute the thread over a certain iterator range.
  /// @param range The range of ElemInfos which should be computed.
  void operator()(const ElemInfoRange & range);

  /// Join threads at the end of the execution
  void join(const ComputeLinearFVElementalThread & /*y*/);

protected:
  /// Setup the contribution objects before we start the loop.
  void setupSystemContributionObjects();

  /// Fetch contribution objects that belong to a specific spatial subdomain
  void fetchBlockSystemContributionObjects();

  /// Print list of object types executed and in which order
  void printGeneralExecutionInformation() const;

  /// Print ordering of objects executed on each block
  void printBlockExecutionInformation() const;

  /// Reference to the problem
  FEProblemBase & _fe_problem;

  /// The number of the linear system we are contributing to
  const unsigned int _system_number;

  /// The vector tags this thread contributes to
  const std::set<TagID> & _vector_tags;

  /// The matrix tags this thread contributes to
  const std::set<TagID> & _matrix_tags;

  /// Thread ID
  THREAD_ID _tid;

  /// The subdomain for the current element
  SubdomainID _subdomain;

  /// The subdomain for the last element
  SubdomainID _old_subdomain;

  /// The set of cached elemental kernels which will be executed on a given element.
  /// This member variable is changed on a per-block basis.
  std::vector<LinearFVElementalKernel *> _fv_kernels;

private:
  /// Boolean that is used to check if the kernels are ready to start contributing to
  /// the system
  bool _system_contrib_objects_ready;
};
