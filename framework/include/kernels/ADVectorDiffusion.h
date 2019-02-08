//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADVECTORDIFFUSION_H
#define ADVECTORDIFFUSION_H

#include "ADVectorKernel.h"

template <ComputeStage>
class ADVectorDiffusion;

declareADValidParams(ADVectorDiffusion);

template <ComputeStage compute_stage>
class ADVectorDiffusion : public ADVectorKernel<compute_stage>
{
public:
  ADVectorDiffusion(const InputParameters & parameters);

protected:
  virtual ADResidual computeQpResidual() override;

  usingVectorKernelMembers;
};

#endif /* ADVECTORDIFFUSION_H */
