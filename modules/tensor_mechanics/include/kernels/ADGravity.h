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
class ADGravity;

declareADValidParams(ADGravity);

template <ComputeStage compute_stage>
class ADGravity : public ADKernelValue<compute_stage>
{
public:
  ADGravity(const InputParameters & parameters);

protected:
  ADReal precomputeQpResidual() override;

private:
  const ADMaterialProperty(Real) & _density;

  const Real _value;

  // _alpha parameter for HHT time integration scheme
  const Real _alpha;

  usingKernelValueMembers;
};

