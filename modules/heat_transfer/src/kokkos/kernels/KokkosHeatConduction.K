//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosHeatConduction.h"

registerKokkosResidualObject("HeatTransferApp", KokkosHeatConduction);

InputParameters
KokkosHeatConduction::validParams()
{
  InputParameters params = KernelGrad::validParams();
  params.addClassDescription("Diffusive heat conduction term $-\\nabla\\cdot(k\\nabla T)$ of the "
                             "thermal energy conservation equation");
  params.addParam<MaterialPropertyName>(
      "thermal_conductivity", "thermal_conductivity", "Property name of the thermal conductivity");
  params.addParam<MaterialPropertyName>(
      "d_thermal_conductivity_dT",
      "d_thermal_conductivity_dT",
      "Property name of the derivative of the thermal conductivity with respect "
      "to the variable");
  return params;
}

KokkosHeatConduction::KokkosHeatConduction(const InputParameters & parameters)
  : KernelGrad(parameters)
{
  _thermal_conductivity = getKokkosMaterialProperty<Real>("thermal_conductivity");

  if (hasKokkosMaterialProperty<Real>("d_thermal_conductivity_dT"))
    _d_thermal_conductivity_dT = getKokkosMaterialProperty<Real>("d_thermal_conductivity_dT");
}
