/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
