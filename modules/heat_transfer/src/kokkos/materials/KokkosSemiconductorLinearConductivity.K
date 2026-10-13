//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosSemiconductorLinearConductivity.h"

registerKokkosMaterial("HeatTransferApp", KokkosSemiconductorLinearConductivity);

InputParameters
KokkosSemiconductorLinearConductivity::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("temp", "Variable for temperature in Kelvin.");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addRequiredParam<Real>("sh_coeff_A", "Steinhart_Hart coefficient A of the material");
  params.addRequiredRangeCheckedParam<Real>(
      "sh_coeff_B", "sh_coeff_B != 0", "Steinhart_Hart coefficient B of the material.");
  params.addClassDescription(
      "Calculates electrical conductivity of a semiconductor from temperature");
  return params;
}

KokkosSemiconductorLinearConductivity::KokkosSemiconductorLinearConductivity(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _sh_coeff_A(getParam<Real>("sh_coeff_A")),
    _sh_coeff_B(getParam<Real>("sh_coeff_B")),
    _T(kokkosCoupledValue("temp")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _electric_conductivity(
        declareKokkosPropertyByName<Real>(_base_name + "electrical_conductivity"))
{
  if (!isCoupledConstant("temp"))
    _delectric_conductivity_dT = declareKokkosPropertyDerivative<Real>(
        _base_name + "electrical_conductivity", {coupledName("temp", 0)});
}
