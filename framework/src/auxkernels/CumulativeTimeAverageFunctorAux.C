//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CumulativeTimeAverageFunctorAux.h"

#include <algorithm>

registerMooseObject("MooseApp", CumulativeTimeAverageFunctorAux);

InputParameters
CumulativeTimeAverageFunctorAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Running time-weighted mean of a functor, starting at a given simulation time.");

  params.addRequiredParam<MooseFunctorName>("functor", "The variable or functor to time-average.");
  params.addRequiredParam<Real>("start_time", "Simulation time at which averaging starts.");
  MooseEnum schemes("implicit-euler trapezoidal-rule", "implicit-euler");
  params.addParam<MooseEnum>(
      "time_integration_scheme",
      schemes,
      "Quadrature of the time integral of the functor. 'implicit-euler' uses the current value "
      "over each time step, 'trapezoidal-rule' the mean of the current and old values.");
  return params;
}

CumulativeTimeAverageFunctorAux::CumulativeTimeAverageFunctorAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _functor(getFunctor<Real>("functor")),
    _mean_old(uOld()),
    _start_time(getParam<Real>("start_time")),
    _scheme(getParam<MooseEnum>("time_integration_scheme").getEnum<Scheme>()),
    _averaging(declareRestartableData<bool>("averaging", false)),
    _t_start(declareRestartableData<Real>("t_start", 0.0))
{
  if (isNodal())
    paramError("variable", "The averaged variable must be elemental (CONSTANT MONOMIAL).");
}

void
CumulativeTimeAverageFunctorAux::timestepSetup()
{
  // Called once per time step, after time has advanced
  if (!_averaging && _t >= _start_time)
  {
    _averaging = true;
    // Include the part of the current step after start_time in the average
    _t_start = std::max(_t - _dt, _start_time);
  }
}

Real
CumulativeTimeAverageFunctorAux::computeValue()
{
  const Moose::ElemArg elem_arg{_current_elem, false};
  const Real u = _functor(elem_arg, Moose::currentState());

  if (!_averaging)
    return u;

  // Total averaging time including this step; the interval is empty at t = start_time
  const Real T = _t - _t_start;
  if (T <= 0)
    return u;

  // Part of the current time step after t_start
  const Real w = std::min(_dt, T);

  // Mean of the functor over that part of the time step
  Real u_step = u;
  if (_scheme == Scheme::TRAPEZOIDAL_RULE)
  {
    // Linear interpolation of the functor at the beginning of the clipped time step
    const Real u_old = _functor(elem_arg, Moose::oldState());
    const Real u_begin = u + (u_old - u) * w / _dt;
    u_step = 0.5 * (u + u_begin);
  }

  // On the first averaged time step w = T, so the old value, which is not an average, drops out
  return _mean_old[_qp] + (u_step - _mean_old[_qp]) * w / T;
}
