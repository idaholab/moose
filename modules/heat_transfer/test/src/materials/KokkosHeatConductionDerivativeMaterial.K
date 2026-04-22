//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosHeatConductionDerivativeMaterial.h"

registerKokkosMaterial("HeatTransferTestApp", KokkosHeatConductionDerivativeMaterial);

InputParameters
KokkosHeatConductionDerivativeMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("temp", "The temperature variable");
  return params;
}

KokkosHeatConductionDerivativeMaterial::KokkosHeatConductionDerivativeMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _thermal_conductivity(declareKokkosProperty<Real>("thermal_conductivity")),
    _d_thermal_conductivity_dT(declareKokkosProperty<Real>("d_thermal_conductivity_dT")),
    _specific_heat(declareKokkosProperty<Real>("specific_heat")),
    _d_specific_heat_dT(declareKokkosProperty<Real>("d_specific_heat_dT")),
    _density(declareKokkosProperty<Real>("density")),
    _d_density_dT(declareKokkosProperty<Real>("d_density_dT")),
    _temp(kokkosCoupledValue("temp"))
{
}
