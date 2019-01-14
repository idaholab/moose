//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDIFFUSIONPRECOMPUTE_H
#define ADDIFFUSIONPRECOMPUTE_H

#include "ADKernelGrad.h"

template <ComputeStage>
class ADDiffusionPrecompute;

declareADValidParams(ADDiffusionPrecompute);

template <ComputeStage compute_stage>
class ADDiffusionPrecompute : public ADKernelGrad<compute_stage>
{
public:
  ADDiffusionPrecompute(const InputParameters & parameters);

protected:
  virtual ADGradResidual precomputeQpResidual();

  usingKernelGradMembers;
};

#endif /* ADDIFFUSIONPRECOMPUTE_H */
