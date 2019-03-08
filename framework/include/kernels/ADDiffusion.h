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

#include "ADKernelGrad.h"

template <ComputeStage>
class ADDiffusion;

declareADValidParams(ADDiffusion);

template <ComputeStage compute_stage>
class ADDiffusion : public ADKernelGrad<compute_stage>
{
public:
  ADDiffusion(const InputParameters & parameters);

protected:
  virtual ADVectorResidual precomputeQpResidual() override;

  usingKernelGradMembers;
};

#endif
