//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatRateRadiation.h"

registerMooseObject("ThermalHydraulicsApp", HeatRateRadiation);

InputParameters
HeatRateRadiation::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();

  params.addRequiredCoupledVar("T", "Temperature");
  params.addRequiredParam<MooseFunctorName>("T_ambient", "Ambient temperature functor");
  params.addRequiredParam<MooseFunctorName>("emissivity", "Emissivity functor");
  params.addParam<MooseFunctorName>("view_factor", 1.0, "View factor functor");
  params.addParam<Real>("stefan_boltzmann_constant", 5.670367e-8, "Stefan-Boltzmann constant");
  params.addParam<MooseFunctorName>("scale", 1.0, "Functor by which to scale the heat flux");

  params.addClassDescription("Integrates a radiative heat flux over a boundary.");

  return params;
}

HeatRateRadiation::HeatRateRadiation(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),

    _T(coupledValue("T")),
    _T_ambient(getFunctor<Real>("T_ambient")),
    _emissivity(getFunctor<Real>("emissivity")),
    _view_factor(getFunctor<Real>("view_factor")),
    _sigma_stefan_boltzmann(getParam<Real>("stefan_boltzmann_constant")),
    _scale(getFunctor<Real>("scale"))
{
}

Real
HeatRateRadiation::computeQpIntegral()
{
  const Moose::ElemSideQpArg space_arg = {_current_elem, _current_side, _qp, _qrule, _q_point[_qp]};
  const auto scale = _scale(space_arg, Moose::currentState());
  const auto emissivity = _emissivity(space_arg, Moose::currentState());
  const auto view_factor = _view_factor(space_arg, Moose::currentState());
  const auto T_ambient = _T_ambient(space_arg, Moose::currentState());

  const Real T4 = MathUtils::pow(_T[_qp], 4);
  const Real T4inf = MathUtils::pow(T_ambient, 4);

  return scale * _sigma_stefan_boltzmann * emissivity * view_factor * (T4inf - T4);
}
