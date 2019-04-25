//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h"

template <ComputeStage>
class ADConvectionPrecompute;

declareADValidParams(ADConvectionPrecompute);

template <ComputeStage compute_stage>
class ADConvectionPrecompute : public ADKernelValue<compute_stage>
{
public:
  ADConvectionPrecompute(const InputParameters & parameters);

protected:
  ADResidual precomputeQpResidual() override;

private:
  RealVectorValue _velocity;

  usingKernelValueMembers;
};

