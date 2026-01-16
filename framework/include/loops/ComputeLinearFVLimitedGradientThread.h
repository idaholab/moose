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
 *
 * This thread currently supports limited gradients produced by scaling the raw Green-Gauss
 * gradients with a per-cell limiter coefficient (e.g. Venkatakrishnan).
 */
class ComputeLinearFVLimitedGradientThread
{
public:
  ComputeLinearFVLimitedGradientThread(FEProblemBase & fe_problem,
                                       const unsigned int linear_system_num,
                                       const Moose::FV::GradientLimiterType limiter_type);

  ComputeLinearFVLimitedGradientThread(ComputeLinearFVLimitedGradientThread & x,
                                       Threads::split split);

  using ElemInfoRange = StoredRange<MooseMesh::const_elem_info_iterator, const ElemInfo *>;

  void operator()(const ElemInfoRange & range);

  void join(const ComputeLinearFVLimitedGradientThread & y);

protected:
  FEProblemBase & _fe_problem;
  const unsigned int _dim;
  const unsigned int _linear_system_number;
  const libMesh::LinearImplicitSystem & _linear_system;
  const unsigned int _system_number;

  const Moose::FV::GradientLimiterType _limiter_type;

  THREAD_ID _tid;
  MooseLinearVariableFV<Real> * _current_var;

  std::vector<std::unique_ptr<NumericVector<Number>>> & _new_limited_gradient;
};
