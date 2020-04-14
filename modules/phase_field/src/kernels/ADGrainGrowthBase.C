//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADGrainGrowthBase.h"

InputParameters
ADGrainGrowthBase::validParams()
{
  InputParameters params = ADAllenCahnBase<Real>::validParams();
  params.addRequiredCoupledVar("v",
                               "Array of coupled order parameter names for other order parameters");
  return params;
}

ADGrainGrowthBase::ADGrainGrowthBase(const InputParameters & parameters)
  : ADAllenCahnBase<Real>(parameters),
    _op_num(coupledComponents("v")),
    _vals(_op_num),
    _mu(getADMaterialProperty<Real>("mu"))
{
  // Loop through grains and load coupled variables into the arrays
  for (unsigned int i = 0; i < _op_num; ++i)
    _vals[i] = &adCoupledValue("v", i);
}
