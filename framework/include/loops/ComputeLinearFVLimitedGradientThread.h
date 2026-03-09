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

// libmesh
#include "libmesh/elem_range.h"
#include "libmesh/threads.h"
#include "libmesh/linear_implicit_system.h"

class FEProblemBase;

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
   * @param linear_system_num The number of the linear system which is assembled by this thread
   * @param limiter_type The type of the limiter which should be computed.
   */
  ComputeLinearFVLimitedGradientThread(FEProblemBase & fe_problem,
                                       const unsigned int linear_system_num,
                                       const Moose::FV::GradientLimiterType limiter_type);

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

  /// The number of the linear system we are contributing to
  const unsigned int _linear_system_number;

  /// Reference to the linear system at libmesh level
  const libMesh::LinearImplicitSystem & _linear_system;

  /// The number of the linear system we are contributing to
  const unsigned int _system_number;

  /// The type of the limiter we requested
  const Moose::FV::GradientLimiterType _limiter_type;

  /// Thread ID
  THREAD_ID _tid;

  /// Pointer to the current variable we are operating on.
  MooseLinearVariableFV<Real> * _current_var;

  /// Reference to the new limited gradient
  std::vector<std::unique_ptr<NumericVector<Number>>> & _new_limited_gradient;
};
