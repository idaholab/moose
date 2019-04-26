//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

/**
 * All AD time kernels should inherit from this class
 *
 */
template <typename T, ComputeStage compute_stage>
class ADTimeKernelTempl : public ADKernelTempl<T, compute_stage>
{
public:
  ADTimeKernelTempl(const InputParameters & parameters);

protected:
  /// Holds the time derivatives at the quadrature points
  const ADTemplateVariableValue & _u_dot;

  usingTemplKernelMembers(T);
};

template <ComputeStage compute_stage>
using ADTimeKernel = ADTimeKernelTempl<Real, compute_stage>;
template <ComputeStage compute_stage>
using ADVectorTimeKernel = ADTimeKernelTempl<RealVectorValue, compute_stage>;

declareADValidParams(ADTimeKernel);
declareADValidParams(ADVectorTimeKernel);

#define usingTemplTimeKernelMembers(type)                                                          \
  usingTemplKernelMembers(type);                                                                   \
  using ADTimeKernelTempl<type, compute_stage>::_u_dot

#define usingTimeKernelMembers usingTemplTimeKernelMembers(Real)
#define usingVectorTimeKernelMembers usingTemplTimeKernelMembers(RealVectorValue)

