//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LangevinNoise.h"
#include "MooseRandom.h"

registerMooseObject("PhaseFieldApp", LangevinNoise);

InputParameters
LangevinNoise::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Source term for non-conserved Langevin noise");
  params.addRequiredParam<Real>("amplitude", "Amplitude"); // per sqrt(time)");
  params.addParam<MaterialPropertyName>(
      "multiplier",
      1.0,
      "Material property to multiply the random numbers with (defaults to 1.0 if omitted)");
  return params;
}
LangevinNoise::LangevinNoise(const InputParameters & parameters)
  : Kernel(parameters),
    _amplitude(getParam<Real>("amplitude")),
    _multiplier_prop(getMaterialProperty<Real>("multiplier"))
{
}

void
LangevinNoise::residualSetup()
{
  unsigned int rseed = _t_step;
  MooseRandom::seed(rseed);
}

Real
LangevinNoise::computeQpResidual()
{
  return -_test[_i][_qp] * (2.0 * MooseRandom::rand() - 1.0) * _amplitude * _multiplier_prop[_qp];
}
