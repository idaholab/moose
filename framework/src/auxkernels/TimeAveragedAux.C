//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeAveragedAux.h"
#include "TimeIntegrator.h"
#include "TransientBase.h"

registerMooseObject("MooseApp", TimeAveragedAux);

InputParameters
TimeAveragedAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Running time-weighted mean of a functor, starting at a given simulation time.");

  params.addRequiredParam<MooseFunctorName>("functor", "The variable or functor to time-average.");
  params.addRequiredParam<Real>("start_time", "Simulation time at which averaging starts.");
  MooseEnum schemes("implicit-euler trapezoidal-rule time-integrator", "implicit-euler");
  params.addParam<MooseEnum>(
      "time_integration_scheme",
      schemes,
      "Quadrature of the time integral of the functor. 'implicit-euler' uses the current value "
      "over each time step, 'trapezoidal-rule' the mean of the current and old values, and "
      "'time-integrator' the trapezoidal rule if every time integrator of the executioner is at "
      "least second order and 'implicit-euler' otherwise.");
  return params;
}

TimeAveragedAux::TimeAveragedAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _functor(getFunctor<Real>("functor")),
    _mean_old(uOld()),
    _start_time(getParam<Real>("start_time")),
    _trapezoidal(useTrapezoidalRule()),
    _averaging(declareRestartableData<bool>("averaging", false)),
    _t_start(declareRestartableData<Real>("t_start", 0.0))
{
  if (isNodal())
    paramError("variable", "The averaged variable must be elemental (CONSTANT MONOMIAL).");
}

bool
TimeAveragedAux::useTrapezoidalRule() const
{
  const auto & scheme = getParam<MooseEnum>("time_integration_scheme");
  if (scheme != "time-integrator")
    return scheme == "trapezoidal-rule";

  const auto transient = dynamic_cast<TransientBase *>(_app.getExecutioner());
  if (!transient)
    paramError("time_integration_scheme", "'time-integrator' requires a transient executioner.");

  // The trapezoidal rule is second order, so it only improves on the rectangle rule if the
  // functor itself is computed with a second order (or higher) time integrator
  const auto time_integrators = transient->getTimeIntegrators();
  if (time_integrators.empty())
    return false;
  for (const auto ti : time_integrators)
    if (ti->order() < 2)
      return false;
  return true;
}

void
TimeAveragedAux::timestepSetup()
{
  // Called once per time step, after time has advanced
  if (!_averaging && _t >= _start_time)
  {
    _averaging = true;
    _t_start = _t - _dt; // include the current step in the average
  }
}

Real
TimeAveragedAux::computeValue()
{
  const Moose::ElemArg elem_arg{_current_elem, false};
  const Real u = _functor(elem_arg, Moose::currentState());

  if (!_averaging)
    return u;

  // Mean of the functor over the current time step
  const Real u_step = _trapezoidal ? 0.5 * (u + _functor(elem_arg, Moose::oldState())) : u;

  const Real T = _t - _t_start; // total averaging time including this step
  return _mean_old[_qp] + (u_step - _mean_old[_qp]) * _dt / T;
}
