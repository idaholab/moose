//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelGrad.h"

/**
 * AD time kernels should inherit from this class when the time portion of the weak residual is
 * multiplied by the gradient of the test function
 */
template <typename T, ComputeStage compute_stage>
class ADTimeKernelGradTempl : public ADKernelGradTempl<T, compute_stage>
{
public:
  ADTimeKernelGradTempl(const InputParameters & parameters);

protected:
  /// Holds the time derivatives at the quadrature points
  const ADTemplateVariableValue & _u_dot;

  usingTemplKernelGradMembers(T);
};

template <ComputeStage compute_stage>
using ADTimeKernelGrad = ADTimeKernelGradTempl<Real, compute_stage>;
template <ComputeStage compute_stage>
using ADVectorTimeKernelGrad = ADTimeKernelGradTempl<RealVectorValue, compute_stage>;

declareADValidParams(ADTimeKernelGrad);
declareADValidParams(ADVectorTimeKernelGrad);

#define usingTemplTimeKernelGradMembers(type)                                                      \
  usingTemplKernelMembers(type);                                                                   \
  using ADTimeKernelGradTempl<type, compute_stage>::_u_dot

#define usingTimeKernelGradMembers usingTemplTimeKernelGradMembers(Real)
#define usingVectorTimeKernelGradMembers usingTemplTimeKernelGradMembers(RealVectorValue)

