//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h"

/**
 * AD time kernels should inherit from this class when the time portion of the weak residual is
 * multiplied by the test function
 */
template <typename T, ComputeStage compute_stage>
class ADTimeKernelValueTempl : public ADKernelValueTempl<T, compute_stage>
{
public:
  ADTimeKernelValueTempl(const InputParameters & parameters);

protected:
  /// Holds the time derivatives at the quadrature points
  const ADTemplateVariableValue & _u_dot;

  usingTemplKernelValueMembers(T);
};

template <ComputeStage compute_stage>
using ADTimeKernelValue = ADTimeKernelValueTempl<Real, compute_stage>;
template <ComputeStage compute_stage>
using ADVectorTimeKernelValue = ADTimeKernelValueTempl<RealVectorValue, compute_stage>;

declareADValidParams(ADTimeKernelValue);
declareADValidParams(ADVectorTimeKernelValue);

#define usingTemplTimeKernelValueMembers(type)                                                     \
  usingTemplKernelMembers(type);                                                                   \
  using ADTimeKernelValueTempl<type, compute_stage>::_u_dot

#define usingTimeKernelValueMembers usingTemplTimeKernelValueMembers(Real)
#define usingVectorTimeKernelValueMembers usingTemplTimeKernelValueMembers(RealVectorValue)

