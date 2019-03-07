//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef ADMATDIFFUSION_H
#define ADMATDIFFUSION_H

#include "ADDiffusion.h"

template <ComputeStage compute_stage>
class ADMatDiffusion;

declareADValidParams(ADMatDiffusion);

template <ComputeStage compute_stage>
class ADMatDiffusion : public ADDiffusion<compute_stage>
{
public:
  ADMatDiffusion(const InputParameters & parameters);

protected:
  virtual ADGradResidual precomputeQpResidual() override;

  const ADMaterialProperty(Real) & _diffusivity;

  usingKernelGradMembers;
};

#endif // ADMATDIFFUSION_H
