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

#define usingTemplKernelStabilizedMembers(type) usingTemplKernelMembers(type)
#define usingKernelStabilizedMembers usingTemplKernelStabilizedMembers(Real)
#define usingVectorKernelStabilizedMembers usingTemplKernelStabilizedMembers(RealVectorValue)

template <typename, ComputeStage>
class ADKernelStabilizedTempl;

template <ComputeStage compute_stage>
using ADKernelStabilized = ADKernelStabilizedTempl<Real, compute_stage>;
template <ComputeStage compute_stage>
using ADVectorKernelStabilized = ADKernelStabilizedTempl<RealVectorValue, compute_stage>;

declareADValidParams(ADKernelStabilized);
declareADValidParams(ADVectorKernelStabilized);

template <typename T, ComputeStage compute_stage>
class ADKernelStabilizedTempl : public ADKernelTempl<T, compute_stage>
{
public:
  ADKernelStabilizedTempl(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeADOffDiagJacobian() override;

protected:
  /**
   * Called before forming the residual for an element
   */
  virtual typename OutputTools<typename Moose::ValueType<T, compute_stage>::type>::OutputValue
  precomputeQpStrongResidual() = 0;

  virtual ADRealVectorValue computeQpStabilization() = 0;

  virtual ADResidual computeQpResidual() override final;

  usingTemplKernelMembers(T);
};

