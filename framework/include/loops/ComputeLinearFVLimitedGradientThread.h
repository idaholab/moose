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
#include "MooseLinearVariableFV.h"
#include "GradientLimiterType.h"

#include <unordered_set>

// libmesh
#include "libmesh/elem_range.h"
#include "libmesh/threads.h"
#include "libmesh/system.h"

class FEProblemBase;
class SystemBase;

/**
 * Compute limited cell gradients for linear FV variables.
 * This thread currently supports limited gradients produced by scaling the raw Green-Gauss
 * gradients with a per-cell limiter coefficient (e.g. Venkatakrishnan).
 */
class ComputeLinearFVLimitedGradientThread
{
public:
  /**
   * Class constructor.
   * @param fe_problem Reference to the problem
   * @param system The system whose variables are assembled by this thread.
   * @param raw_gradient The raw gradient container used as limiter input.
   * @param temporary_limited_gradient Scratch storage for limited gradients being assembled.
   * @param limiter_type The type of the limiter which should be computed.
   * @param requested_variables Variable numbers that requested this limiter.
   */
  ComputeLinearFVLimitedGradientThread(
      FEProblemBase & fe_problem,
      SystemBase & system,
      const std::vector<std::unique_ptr<NumericVector<Number>>> & raw_gradient,
      std::vector<std::unique_ptr<NumericVector<Number>>> & temporary_limited_gradient,
      const Moose::FV::GradientLimiterType limiter_type,
      const std::unordered_set<unsigned int> & requested_variables);

  /**
   * Splitting constructor.
   * @param x Reference to the other thread
   * @param split The thread split
   */
  ComputeLinearFVLimitedGradientThread(ComputeLinearFVLimitedGradientThread & x,
                                       Threads::split split);

  using ElemInfoRange = StoredRange<MooseMesh::const_elem_info_iterator, const ElemInfo *>;
  /// Operator which is used to execute the thread over a certain iterator range.
  /// @param range The range of ElemInfos which should be computed.
  void operator()(const ElemInfoRange & range);

  /// Join threads at the end of the execution
  void join(const ComputeLinearFVLimitedGradientThread & y);

protected:
  /// Reference to the problem
  FEProblemBase & _fe_problem;

  /// The dimension of the domain
  const unsigned int _dim;

  /// The system wrapper this thread operates on.
  SystemBase & _system;

  /// Reference to the libMesh system backing the wrapper system.
  const libMesh::System & _libmesh_system;

  /// Global system number in the libMesh equation system.
  const unsigned int _system_number;

  /// Reference to the raw gradient storage used as input for limiting.
  const std::vector<std::unique_ptr<NumericVector<Number>>> & _raw_gradient;

  /// The type of the limiter we requested
  const Moose::FV::GradientLimiterType _limiter_type;

  /// Variable numbers that requested the current limiter.
  const std::unordered_set<unsigned int> & _requested_variables;

  /// Thread ID
  THREAD_ID _tid;

  /// Pointer to the current variable we are operating on.
  MooseLinearVariableFV<Real> * _current_var;

  /// Reference to the temporary limited gradient storage
  std::vector<std::unique_ptr<NumericVector<Number>>> & _temporary_limited_gradient;
};
