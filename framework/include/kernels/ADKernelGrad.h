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

template <ComputeStage>
class ADKernelGrad;

declareADValidParams(ADKernelGrad);

template <ComputeStage compute_stage>
class ADKernelGrad : public ADKernel<compute_stage>
{
public:
  ADKernelGrad(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(MooseVariableFEBase & jvar) override;

protected:
  /**
   * Called before forming the residual for an element
   */
  virtual ADGradResidual precomputeQpResidual() = 0;

  virtual ADResidual computeQpResidual() final;

  usingKernelMembers;
};

#endif /* ADKERNELGRAD_H */
