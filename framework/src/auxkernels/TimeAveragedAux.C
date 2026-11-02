//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeAveragedAux.h"

registerMooseObject("MooseApp", TimeAveragedAux);

InputParameters
TimeAveragedAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Running time-weighted mean of a functor, starting at a given simulation time.");

  params.addRequiredParam<MooseFunctorName>(
      "functor", "The variable or functor to time-average.");
  params.addRequiredParam<Real>("start_time", "Simulation time at which averaging starts.");
  return params;
}

TimeAveragedAux::TimeAveragedAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _functor(getFunctor<Real>("functor")),
    _mean_old(uOld()),
    _start_time(getParam<Real>("start_time")),
    _averaging(declareRestartableData<bool>("averaging", false)),
    _t_start(declareRestartableData<Real>("t_start", 0.0))
{
  if (isNodal())
    paramError("variable", "The averaged variable must be elemental (CONSTANT MONOMIAL).");
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

  const Real T = _t - _t_start; // total averaging time including this step
  return _mean_old[_qp] + (u - _mean_old[_qp]) * _dt / T;
}
