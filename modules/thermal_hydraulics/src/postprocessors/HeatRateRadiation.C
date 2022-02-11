//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatRateRadiation.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", HeatRateRadiation);

InputParameters
HeatRateRadiation::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();

  params.addRequiredCoupledVar("T", "Temperature");
  params.addRequiredParam<FunctionName>("T_ambient", "Ambient temperature");
  params.addRequiredParam<Real>("emissivity", "Emissivity");
  params.addParam<FunctionName>("view_factor", "1", "View factor function");
  params.addParam<Real>("stefan_boltzmann_constant", 5.670367e-8, "Stefan-Boltzmann constant");
  params.addParam<Real>(
      "scale", 1.0, "Factor by which to scale integral, like when using a 2D domain");

  params.addClassDescription("Integrates a radiative heat flux over a boundary.");

  return params;
}

HeatRateRadiation::HeatRateRadiation(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),

    _T(coupledValue("T")),
    _T_ambient(getFunction("T_ambient")),
    _emissivity(getParam<Real>("emissivity")),
    _view_factor_fn(getFunction("view_factor")),
    _sigma_stefan_boltzmann(getParam<Real>("stefan_boltzmann_constant")),
    _scale(getParam<Real>("scale"))
{
}

Real
HeatRateRadiation::computeQpIntegral()
{
  const Real T4 = MathUtils::pow(_T[_qp], 4);
  const Real T4inf = MathUtils::pow(_T_ambient.value(_t, _q_point[_qp]), 4);
  return _scale * _sigma_stefan_boltzmann * _emissivity * _view_factor_fn.value(_t, _q_point[_qp]) *
         (T4inf - T4);
}
