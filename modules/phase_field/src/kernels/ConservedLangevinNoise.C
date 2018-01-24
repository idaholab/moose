//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservedLangevinNoise.h"

template <>
InputParameters
validParams<ConservedLangevinNoise>()
{
  InputParameters params = validParams<LangevinNoise>();
  params.addClassDescription("Source term for noise from a ConservativeNoise userobject");
  params.addRequiredParam<UserObjectName>(
      "noise", "ConservativeNoise userobject that produces the random numbers");
  return params;
}
ConservedLangevinNoise::ConservedLangevinNoise(const InputParameters & parameters)
  : LangevinNoise(parameters), _noise(getUserObject<ConservedNoiseInterface>("noise"))
{
}

Real
ConservedLangevinNoise::computeQpResidual()
{
  return -_test[_i][_qp] * _noise.getQpValue(_current_elem->id(), _qp) * _amplitude *
         _multiplier_prop[_qp];
}
