/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "HomogenizedHeatConduction.h"

template <>
InputParameters
validParams<HomogenizedHeatConduction>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<MaterialPropertyName>(
      "diffusion_coefficient",
      "thermal_conductivity",
      "The diffusion coefficient for the temperature gradient (Default: thermal_conductivity)");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "component",
      "component < 3",
      "An integer corresponding to the direction the variable this "
      "kernel acts in. (0 for x, 1 for y, 2 for z)");
  return params;
}

HomogenizedHeatConduction::HomogenizedHeatConduction(const InputParameters & parameters)
  : Kernel(parameters),
    _diffusion_coefficient(getMaterialProperty<Real>("diffusion_coefficient")),
    _component(getParam<unsigned int>("component"))
{
}

Real
HomogenizedHeatConduction::computeQpResidual()
{
  // Compute positive value since we are computing a residual not a rhs
  return _diffusion_coefficient[_qp] * _grad_test[_i][_qp](_component);
}
