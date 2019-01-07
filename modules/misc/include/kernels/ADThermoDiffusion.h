//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDIFFUSION_H
#define ADDIFFUSION_H

#include "ADKernel.h"

template <ComputeStage compute_stage>
class ADThermoDiffusion;

declareADValidParams(ADThermoDiffusion);

template <ComputeStage compute_stage>
class ADThermoDiffusion : public ADKernel<compute_stage>
{
public:
  ADThermoDiffusion(const InputParameters & parameters);

protected:
  virtual ADResidual computeQpResidual() override;

  const ADVariableGradient & _grad_temp;
  const ADMaterialProperty(Real) & _soret_coeff;

  usingKernelMembers;
};

#endif /* ADDIFFUSION_H */
