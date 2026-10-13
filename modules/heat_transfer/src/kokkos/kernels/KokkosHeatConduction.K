//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  InputParameters params = Moose::Kokkos::KernelGrad::validParams();
  params.addClassDescription("Diffusive heat conduction term $-\\nabla\\cdot(k\\nabla T)$ of the "
                             "thermal energy conservation equation");
  params.addParam<MaterialPropertyName>(
      "thermal_conductivity", "thermal_conductivity", "Property name of the thermal conductivity");
  params.addParam<MaterialPropertyName>(
      "d_thermal_conductivity_dT",
      "d_thermal_conductivity_dT",
      "Property name of the derivative of the thermal conductivity with respect "
      "to the variable");
  params.addParam<MaterialPropertyName>(
      "d_thermal_conductivity_dx",
      "d_thermal_conductivity_dx",
      "Property name of the gradient of the thermal conductivity with respect to the position, "
      "used in the Jacobian with respect to the displacements when the term is posed on the "
      "current configuration");
  return params;
}

KokkosHeatConduction::KokkosHeatConduction(const InputParameters & parameters)
  : Moose::Kokkos::KernelGrad(parameters)
{
  _thermal_conductivity = getKokkosMaterialProperty<Real>("thermal_conductivity");

  if (hasKokkosMaterialProperty<Real>("d_thermal_conductivity_dT"))
    _d_thermal_conductivity_dT = getKokkosMaterialProperty<Real>("d_thermal_conductivity_dT");

  if (hasKokkosMaterialProperty<Real3>("d_thermal_conductivity_dx"))
    _d_thermal_conductivity_dx = getKokkosMaterialProperty<Real3>("d_thermal_conductivity_dx");
}
