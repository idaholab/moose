//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
class LinearFVFluxKernel;

/**
 * Adds contributions from face terms discretized using the finite volume method to
 * the matrix and right hand side of a linear system.
 */
class ComputeLinearFVFaceThread
{
public:
  /**
   * Class constructor.
   * @param fe_problem Reference to the problem
   * @param linear_system_num The number of the linear system which is assembled by this thread
   * @param mode Computation mode (rhs, matrix or both)
   * @param tags The vector/matrix tags this thread should contribute to. These are used to query
   * the warehouse for the objects that should contribute to the linear system matrix and right hand
   * side. When mode == FullSystem, these tags should be vector tags
   */
  ComputeLinearFVFaceThread(FEProblemBase & fe_problem,
                            const unsigned int linear_system_num,
                            const Moose::FV::LinearFVComputationMode mode,
                            const std::set<TagID> & tags);

  /**
   * Splitting constructor.
   * @param x Reference to the other thread
   * @param split The thread split
   */
  ComputeLinearFVFaceThread(ComputeLinearFVFaceThread & x, Threads::split split);

  using FaceInfoRange = StoredRange<MooseMesh::const_face_info_iterator, const FaceInfo *>;
  /// Operator which is used to execute the thread over a certain iterator range.
  /// @param range The range of FaceInfos which should be computed.
  void operator()(const FaceInfoRange & range);

  /// Join threads at the end of the execution
  void join(const ComputeLinearFVFaceThread & /*y*/);

protected:
  /// Fetch LinearFVFluxKernels for a given block. We only call this when
  /// we transition from one block to another.
  void fetchSystemContributionObjects();

  /// Print list of executed object types together with the execution order
  void printGeneralExecutionInformation() const;

  /// Print ordering of objects executed on each block
  void printBlockExecutionInformation() const;

  /// Reference to the problem
  FEProblemBase & _fe_problem;

  /// The number of the linear system we are contributing to
  const unsigned int _linear_system_number;

  /// The mode in which this thread is operating
  const Moose::FV::LinearFVComputationMode _mode;

  /// The vector/matrix tags this thread contributes to
  const std::set<TagID> _tags;

  /// The subdomain for the current element
  SubdomainID _subdomain;

  /// The subdomain for the last element
  SubdomainID _old_subdomain;

  /// The subdomain for the current neighbor
  SubdomainID _neighbor_subdomain;

  /// The subdomain for the last neighbor
  SubdomainID _old_neighbor_subdomain;

  /// Thread ID
  THREAD_ID _tid;

  /// LinearFVFluxKernels for the element
  std::vector<LinearFVFluxKernel *> _elem_fv_flux_kernels;

  /// LinearFVFluxKernels for the neighbor
  std::vector<LinearFVFluxKernel *> _neighbor_fv_flux_kernels;

  /// Combined LinearFVFluxKernels which will be used to contribute to a system
  std::set<LinearFVFluxKernel *> _fv_flux_kernels;
};
