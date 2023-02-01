//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SemiconductorLinearConductivity.h"
#include "libmesh/quadrature.h"

registerMooseObject("HeatConductionApp", SemiconductorLinearConductivity);

InputParameters
SemiconductorLinearConductivity::validParams()
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

SemiconductorLinearConductivity::SemiconductorLinearConductivity(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _sh_coeff_A(getParam<Real>("sh_coeff_A")),
    _sh_coeff_B(getParam<Real>("sh_coeff_B")),
    _T(coupledValue("temp")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _electric_conductivity(declareProperty<Real>(_base_name + "electrical_conductivity")),
    _delectric_conductivity_dT(
        isCoupledConstant("temp")
            ? nullptr
            : &declarePropertyDerivative<Real>(_base_name + "electrical_conductivity",
                                               coupledName("temp", 0)))
{
}

void
SemiconductorLinearConductivity::computeQpProperties()
{
  mooseAssert(MooseUtils::absoluteFuzzyGreaterThan(_T[_qp], 0.0),
              "Encountered zero or negative temperature in SemiconductorLinearConductivity");

  mooseAssert(_sh_coeff_B != 0, "Divided by zero as _sh_coeff_B = 0");

  _electric_conductivity[_qp] = exp((_sh_coeff_A - 1 / _T[_qp]) / _sh_coeff_B);
  if (_delectric_conductivity_dT)
    (*_delectric_conductivity_dT)[_qp] =
        _electric_conductivity[_qp] / (_sh_coeff_B * _T[_qp] * _T[_qp]);
}
