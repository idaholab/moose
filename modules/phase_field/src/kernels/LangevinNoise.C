/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "LangevinNoise.h"
#include "MooseRandom.h"

template <>
InputParameters
validParams<LangevinNoise>()
{
  InputParameters params = validParams<Kernel>();
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
