//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVTimeDerivative.h"

registerMooseObject("MooseApp", LinearFVTimeDerivative);

InputParameters
LinearFVTimeDerivative::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();
  params.addClassDescription("Represents the matrix and right hand side contributions of a "
                             "time derivative term in a partial differential equation.");
  params.addParam<MooseFunctorName>(
      "factor", 1.0, "A multiplier on the variable within the time derivative.");
  return params;
}

LinearFVTimeDerivative::LinearFVTimeDerivative(const InputParameters & params)
  : LinearFVElementalKernel(params),
    _factor(getFunctor<Real>("factor")),
    _time_integrator(_sys.getTimeIntegrator(_var_num)),
    _factor_history(_time_integrator.numStatesRequired(), 0.0),
    _state_args(_time_integrator.numStatesRequired(), determineState())
{
  // In case we need older states
  for (const auto i : index_range(_state_args))
    _state_args[i] = Moose::StateArg(i, Moose::SolutionIterationType::Time);
}

Real
LinearFVTimeDerivative::computeMatrixContribution()
{
  const auto elem_arg = makeElemArg(_current_elem_info->elem());
  return _time_integrator.timeDerivativeMatrixContribution(_factor(elem_arg, _state_args[0])) *
         _current_elem_volume;
}

Real
LinearFVTimeDerivative::computeRightHandSideContribution()
{
  return _time_integrator.timeDerivativeRHSContribution(_dof_id, _factor_history) *
         _current_elem_volume;
}

void
LinearFVTimeDerivative::setCurrentElemInfo(const ElemInfo * elem_info)
{
  LinearFVElementalKernel::setCurrentElemInfo(elem_info);

  const auto elem_arg = makeElemArg(_current_elem_info->elem());
  for (const auto i : index_range(_factor_history))
    _factor_history[i] = _factor(elem_arg, _state_args[i]);
}
