//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosHomogenizedHeatConduction.h"

registerKokkosResidualObject("HeatTransferApp", KokkosHomogenizedHeatConduction);

InputParameters
KokkosHomogenizedHeatConduction::validParams()
{
  InputParameters params = KernelGrad::validParams();
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

KokkosHomogenizedHeatConduction::KokkosHomogenizedHeatConduction(const InputParameters & parameters)
  : KernelGrad(parameters),
    _diffusion_coefficient(getKokkosMaterialProperty<Real>("diffusion_coefficient")),
    _component(getParam<unsigned int>("component"))
{
}
