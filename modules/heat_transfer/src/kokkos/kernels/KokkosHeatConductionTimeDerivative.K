//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosHeatConductionTimeDerivative.h"

registerKokkosResidualObject("HeatTransferApp", KokkosHeatConductionTimeDerivative);

InputParameters
KokkosHeatConductionTimeDerivative::validParams()
{
  InputParameters params = KokkosTimeDerivative::validParams();
  params.addClassDescription("Time derivative term $\\rho c_p \\frac{\\partial T}{\\partial t}$ of "
                             "the thermal energy conservation equation.");

  params.addParam<MaterialPropertyName>(
      "specific_heat",
      "specific_heat",
      "Name of the volumetric isobaric specific heat material property");
  params.addParam<MaterialPropertyName>(
      "d_specific_heat_dT",
      "Name of the material property for the derivative of the specific heat with respect "
      "to the variable.");
  params.addParam<MaterialPropertyName>(
      "density", "density", "Property name of the density material property");
  params.addParam<MaterialPropertyName>(
      "d_density_dT",
      "Name of material property for the derivative of the density with respect to the variable.");
  return params;
}

KokkosHeatConductionTimeDerivative::KokkosHeatConductionTimeDerivative(
    const InputParameters & parameters)
  : KokkosTimeDerivative(parameters)
{
  _specific_heat = getKokkosMaterialProperty<Real>("specific_heat");

  if (hasKokkosMaterialProperty<Real>("d_specific_heat_dT"))
    _d_specific_heat_dT = getKokkosMaterialProperty<Real>("d_specific_heat_dT");

  _density = getKokkosMaterialProperty<Real>("density");

  if (hasKokkosMaterialProperty<Real>("d_density_dT"))
    _d_density_dT = getKokkosMaterialProperty<Real>("d_density_dT");
}
