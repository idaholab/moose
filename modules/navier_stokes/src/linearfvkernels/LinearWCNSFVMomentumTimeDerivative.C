//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearWCNSFVMomentumTimeDerivative.h"

registerMooseObject("NavierStokesApp", LinearWCNSFVMomentumTimeDerivative);

InputParameters
LinearWCNSFVMomentumTimeDerivative::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();
  params.addClassDescription(
      "Represents the matrix and right hand side contributions of the conservative "
      "weakly-compressible momentum time derivative d(rho*u)/dt.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The density functor");
  return params;
}

LinearWCNSFVMomentumTimeDerivative::LinearWCNSFVMomentumTimeDerivative(
    const InputParameters & params)
  : LinearFVElementalKernel(params),
    _rho(getFunctor<Real>(NS::density)),
    _time_integrator(_sys.getTimeIntegrator(_var_num)),
    _rho_history(_time_integrator.numStatesRequired(), 0.0),
    _history_state_args(_time_integrator.numStatesRequired(),
                        Moose::StateArg(1, Moose::SolutionIterationType::Time))
{
  for (const auto i : index_range(_history_state_args))
    _history_state_args[i] = Moose::StateArg(i + 1, Moose::SolutionIterationType::Time);
}

Real
LinearWCNSFVMomentumTimeDerivative::computeMatrixContribution()
{
  const auto elem_arg = makeElemArg(_current_elem_info->elem());
  const Real rho = _rho(elem_arg, Moose::currentState());
  return _time_integrator.timeDerivativeMatrixContribution(rho) * _current_elem_volume;
}

Real
LinearWCNSFVMomentumTimeDerivative::computeRightHandSideContribution()
{
  return _time_integrator.timeDerivativeRHSContribution(_dof_id, _rho_history) *
         _current_elem_volume;
}

void
LinearWCNSFVMomentumTimeDerivative::setCurrentElemInfo(const ElemInfo * elem_info)
{
  LinearFVElementalKernel::setCurrentElemInfo(elem_info);

  const auto elem_arg = makeElemArg(_current_elem_info->elem());
  for (const auto i : index_range(_rho_history))
    _rho_history[i] = _rho(elem_arg, _history_state_args[i]);
}
