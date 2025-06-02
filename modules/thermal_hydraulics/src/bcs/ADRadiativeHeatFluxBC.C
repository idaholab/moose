//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADRadiativeHeatFluxBC.h"

registerMooseObject("ThermalHydraulicsApp", ADRadiativeHeatFluxBC);

InputParameters
ADRadiativeHeatFluxBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();

  params.addRequiredParam<MooseFunctorName>("T_ambient", "Ambient temperature functor");
  params.addRequiredParam<MooseFunctorName>("emissivity", "Emissivity functor");
  params.addParam<MooseFunctorName>("view_factor", 1.0, "View factor functor");
  params.addDeprecatedParam<PostprocessorName>(
      "scale_pp",
      "1.0",
      "Post-processor by which to scale boundary condition",
      "The 'scale' parameter is replacing the 'scale_pp' parameter. 'scale' is a function "
      "parameter instead of a post-processor parameter. If you need to scale from a post-processor "
      "value, use a PostprocessorFunction.");
  params.addParam<MooseFunctorName>(
      "scale", 1.0, "Functor by which to scale the boundary condition");
  params.addParam<Real>("stefan_boltzmann_constant", 5.670367e-8, "Stefan-Boltzmann constant");

  params.addClassDescription(
      "Radiative heat transfer boundary condition for a plate heat structure");

  return params;
}

ADRadiativeHeatFluxBC::ADRadiativeHeatFluxBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _T_ambient(getFunctor<ADReal>("T_ambient")),
    _emissivity(getFunctor<ADReal>("emissivity")),
    _view_factor(getFunctor<ADReal>("view_factor")),
    _scale_pp(getPostprocessorValue("scale_pp")),
    _scale(getFunctor<ADReal>("scale")),
    _sigma(getParam<Real>("stefan_boltzmann_constant"))
{
}

ADReal
ADRadiativeHeatFluxBC::computeQpResidual()
{
  const Moose::ElemSideQpArg space_arg = {_current_elem, _current_side, _qp, _qrule, _q_point[_qp]};
  const auto scale = _scale(space_arg, Moose::currentState());
  const auto emissivity = _emissivity(space_arg, Moose::currentState());
  const auto view_factor = _view_factor(space_arg, Moose::currentState());
  const auto T_ambient = _T_ambient(space_arg, Moose::currentState());

  const auto T4 = MathUtils::pow(_u[_qp], 4);
  const auto T4inf = MathUtils::pow(T_ambient, 4);

  return _test[_i][_qp] * _sigma * _scale_pp * scale * emissivity * view_factor * (T4 - T4inf);
}
