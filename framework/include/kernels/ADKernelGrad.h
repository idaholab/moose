//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADKERNELGRAD_H
#define ADKERNELGRAD_H

#include "ADKernel.h"

#define usingKernelGradMembers usingKernelMembers
#define usingVectorKernelGradMembers usingVectorKernelMembers

template <typename, ComputeStage>
class ADKernelGradTempl;

template <ComputeStage compute_stage>
using ADKernelGrad = ADKernelGradTempl<Real, compute_stage>;
template <ComputeStage compute_stage>
using ADVectorKernelGrad = ADKernelGradTempl<RealVectorValue, compute_stage>;

declareADValidParams(ADKernelGrad);
declareADValidParams(ADVectorKernelGrad);

template <typename T, ComputeStage compute_stage>
class ADKernelGradTempl : public ADKernelTempl<T, compute_stage>
{
public:
  ADKernelGradTempl(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeADOffDiagJacobian() override;

protected:
  /**
   * Called before forming the residual for an element
   */
  virtual ADGradResidualTempl precomputeQpResidual() = 0;

  virtual ADResidual computeQpResidual() override final;

  usingTemplKernelMembers(T);
};

#endif /* ADKERNELGRAD_H */
