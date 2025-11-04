//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatRateConvection.h"

registerMooseObject("ThermalHydraulicsApp", HeatRateConvection);

InputParameters
HeatRateConvection::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();

  params.addRequiredCoupledVar("T", "Temperature");
  params.addRequiredParam<MooseFunctorName>("T_ambient", "Ambient temperature functor");
  params.addRequiredParam<MooseFunctorName>("htc", "Ambient heat transfer coefficient functor");
  params.addParam<MooseFunctorName>("scale", 1.0, "Functor by which to scale the heat flux");

  params.addClassDescription("Integrates a convective heat flux over a boundary.");

  return params;
}

HeatRateConvection::HeatRateConvection(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),

    _T(coupledValue("T")),
    _T_ambient(getFunctor<Real>("T_ambient")),
    _htc_ambient(getFunctor<Real>("htc")),
    _scale(getFunctor<Real>("scale"))
{
}

Real
HeatRateConvection::computeQpIntegral()
{
  const Moose::ElemSideQpArg space_arg = {_current_elem, _current_side, _qp, _qrule, _q_point[_qp]};
  const auto scale = _scale(space_arg, Moose::currentState());
  const auto htc = _htc_ambient(space_arg, Moose::currentState());
  const auto T_ambient = _T_ambient(space_arg, Moose::currentState());

  return scale * htc * (T_ambient - _T[_qp]);
}
