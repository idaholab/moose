//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details;\
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADKERNELVALUE_H
#define ADKERNELVALUE_H

#include "ADKernel.h"

#define usingKernelValueMembers usingKernelMembers

template <ComputeStage>
class ADKernelValue;

declareADValidParams(ADKernelValue);

template <ComputeStage compute_stage>
class ADKernelValue : public ADKernel<compute_stage>
{
public:
  ADKernelValue(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(MooseVariableFEBase & jvar) override;

protected:
  /**
   * Called before forming the residual for an element
   */
  virtual ADResidual precomputeQpResidual() = 0;

  virtual ADResidual computeQpResidual() final;

  usingKernelMembers;
};

#endif /* ADKERNELVALUE_H */
