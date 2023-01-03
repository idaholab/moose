//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HomogenizedHeatConduction.h"

registerMooseObject("HeatConductionApp", HomogenizedHeatConduction);

InputParameters
HomogenizedHeatConduction::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Kernel for asymptotic expansion homogenization for thermal conductivity");
  params.addParam<MaterialPropertyName>("diffusion_coefficient",
                                        "thermal_conductivity",
                                        "The diffusion coefficient for the temperature gradient");
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
