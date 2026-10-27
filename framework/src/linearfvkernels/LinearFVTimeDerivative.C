//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  params.addParam<bool>(
      "conservative_form",
      true,
      "Whether to assemble the time derivative of the product, d(cu)/dt, rather than the "
      "multiplier times the transient of the variable, c du/dt. The conservative form is the "
      "default because it is the more general of the two: where the multiplier does not vary in "
      "time every state of it is the same value, the product form collapses to c du/dt exactly. "
      "Where it does vary they differ, and it is the conservative form that most conservation laws "
      "ask for. Set this to false to recover the multiplier held outside the derivative.");
  return params;
}

LinearFVTimeDerivative::LinearFVTimeDerivative(const InputParameters & params)
  : LinearFVElementalKernel(params),
    _factor(getFunctor<Real>("factor")),
    _time_integrator(_sys.getTimeIntegrator(_var_num)),
    _conservative_form(getParam<bool>("conservative_form")),
    _factor_history(_time_integrator.numStatesRequired(), 0.0),
    _state_args(_time_integrator.numStatesRequired(), determineState()),
    _current_state(Moose::StateArg(0, Moose::SolutionIterationType::Time))
{
  // The integrators pair factor_history[i] with the solution i+1 steps back: implicit Euler uses
  // factors[0] against the old solution, and BDF2 adds factors[1] against the older one. Which
  // multiplier belongs in each slot is the whole difference between the two forms. Note that
  // where the multiplier is constant in time the two slots hold the same number either way, so
  // the default costs nothing and changes nothing for such a problem.
  //
  //   c du/dt   wants the one current multiplier against every solution state,
  //             c^{n+1} ( w_0 u^{n+1} + w_1 u^n + w_2 u^{n-1} ) / dt
  //
  //   d(cu)/dt  wants each multiplier taken at the same time as the solution it multiplies,
  //             ( w_0 c^{n+1} u^{n+1} + w_1 c^n u^n + w_2 c^{n-1} u^{n-1} ) / dt
  //
  // so slot i takes state i+1 in the conservative form and state 0 otherwise. Note that the
  // matrix keeps the current multiplier either way, since it scales u^{n+1}; that is why it no
  // longer reads _state_args[0], which is the old multiplier in the conservative form.
  //
  // Indexing the history by i, which is what this did originally, is neither: it hands every
  // multiplier the solution one step older than itself and converges to c u' - c' u / 2, so
  // refining the time step does not recover either answer. Implicit Euler asks for a single state
  // and so cannot expose it; BDF2 with a multiplier that varies in time does.
  for (const auto i : index_range(_state_args))
    _state_args[i] =
        Moose::StateArg(_conservative_form ? i + 1 : 0, Moose::SolutionIterationType::Time);
}

Real
LinearFVTimeDerivative::computeMatrixContribution()
{
  const auto elem_arg = makeElemArg(_current_elem_info->elem());
  return _time_integrator.timeDerivativeMatrixContribution(_factor(elem_arg, _current_state)) *
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
