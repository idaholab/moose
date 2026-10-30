//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosHeatConductionMaterial.h"

registerKokkosMaterial("HeatTransferApp", KokkosHeatConductionMaterial);

InputParameters
KokkosHeatConductionMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addParam<Real>("thermal_conductivity", 0, "The thermal conductivity value");
  params.addParam<Real>("specific_heat", 0, "The specific heat value");
  params.addParam<RealVectorValue>(
      "thermal_conductivity_gradient",
      "Gradient of a thermal conductivity varying linearly with the position, k = "
      "thermal_conductivity + thermal_conductivity_gradient . x, where x is the quadrature point "
      "on the configuration of the material. The gradient is declared as the property "
      "d_thermal_conductivity_dx.");

  params.addClassDescription("General-purpose material model for heat conduction");

  return params;
}

KokkosHeatConductionMaterial::KokkosHeatConductionMaterial(const InputParameters & parameters)
  : Material(parameters),
    _my_thermal_conductivity(getParam<Real>("thermal_conductivity")),
    _my_specific_heat(getParam<Real>("specific_heat")),
    _thermal_conductivity_gradient(isParamValid("thermal_conductivity_gradient")
                                       ? getParam<RealVectorValue>("thermal_conductivity_gradient")
                                       : RealVectorValue()),
    _position_dependent(isParamValid("thermal_conductivity_gradient")),
    _thermal_conductivity(declareKokkosProperty<Real>("thermal_conductivity")),
    _specific_heat(declareKokkosProperty<Real>("specific_heat"))
{
  if (_position_dependent)
    _d_thermal_conductivity_dx =
        declareKokkosProperty<Moose::Kokkos::Real3>("d_thermal_conductivity_dx");
}
