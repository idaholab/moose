//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef ADHEATCONDUCTION_H
#define ADHEATCONDUCTION_H

#include "ADDiffusion.h"

template <ComputeStage compute_stage>
class ADHeatConduction;

declareADValidParams(ADHeatConduction);

template <ComputeStage compute_stage>
class ADHeatConduction : public ADDiffusion<compute_stage>
{
public:
  ADHeatConduction(const InputParameters & parameters);

protected:
  virtual ADVectorResidual precomputeQpResidual() override;

  const ADMaterialProperty(Real) & _thermal_conductivity;

  usingKernelGradMembers;
};

#endif // ADHEATCONDUCTION_H
