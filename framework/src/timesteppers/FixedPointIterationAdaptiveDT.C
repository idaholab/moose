//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FixedPointIterationAdaptiveDT.h"
#include "FEProblemBase.h"
#include "Transient.h"

registerMooseObject("MooseApp", FixedPointIterationAdaptiveDT);

InputParameters
FixedPointIterationAdaptiveDT::validParams()
{
  InputParameters params = TimeStepper::validParams();
  params.addClassDescription(
      "Computes time step size based on a target number of fixed point iterations");
  params.addRequiredRangeCheckedParam<Real>(
      "dt_initial", "dt_initial > 0", "The initial time step size");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "target_iterations", "target_iterations > 0", "The target number of fixed point iterations");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "target_window",
      "target_window >= 0",
      "The number of iterations added to and subtracted from 'target_iterations' to determine the "
      "iteration window; the time step size will increase if the iterations were below "
      "'target_iterations' - 'target_window' and will decrease if the iterations were above "
      "'target_iterations' + 'target_window'.");
  params.addRangeCheckedParam<Real>(
      "increase_factor",
      1.2,
      "increase_factor >= 1",
      "Factor by which the previous time step size will increase if the previous "
      "number of fixed point iterations was below the target window minimum "
      "('target_iterations' - 'target_window').");
  params.addRangeCheckedParam<Real>(
      "decrease_factor",
      0.8,
      "decrease_factor <= 1",
      "Factor by which the previous time step size will decrease if the previous "
      "number of fixed point iterations was above the target window maximum "
      "('target_iterations' + 'target_window').");

  return params;
}

FixedPointIterationAdaptiveDT::FixedPointIterationAdaptiveDT(const InputParameters & parameters)
  : TimeStepper(parameters),
    _dt_initial(getParam<Real>("dt_initial")),
    _target_center(getParam<unsigned int>("target_iterations")),
    _target_window(getParam<unsigned int>("target_window")),
    _target_min(_target_center - _target_window),
    _target_max(_target_center + _target_window),
    _increase_factor(getParam<Real>("increase_factor")),
    _decrease_factor(getParam<Real>("decrease_factor")),
    _dt_old(declareRestartableData<Real>("dt_old", 0.0)),
    _fp_its(declareRestartableData<unsigned int>("fp_its", 0))
{
}

void
FixedPointIterationAdaptiveDT::init()
{
  TimeStepper::init();

  if (!_fe_problem.hasMultiApps())
    mooseError("This time stepper can only be used if there are MultiApps in the problem.");

  const auto & fp_solve = _executioner.fixedPointSolve();
  const auto min_its = fp_solve.minFixedPointIts();
  const auto max_its = fp_solve.maxFixedPointIts();
  if (_target_max > max_its || _target_min < min_its)
    mooseError("The specified target iteration window, [",
               _target_min,
               ",",
               _target_max,
               "], must be within the minimum and maximum number of fixed point iterations "
               "specified for the Executioner, [",
               min_its,
               ",",
               max_its,
               "].");
}

Real
FixedPointIterationAdaptiveDT::computeInitialDT()
{
  return _dt_initial;
}

Real
FixedPointIterationAdaptiveDT::computeDT()
{
  Real dt = _dt_old;

  if (_fp_its > _target_max)
  {
    dt *= _decrease_factor;
    if (_verbose)
      _console << "Decreasing dt (" << _dt_old << ") to " << dt
               << " since number of fixed point iterations (" << _fp_its << ") is > " << _target_max
               << "." << std::endl;
  }
  else if (_fp_its < _target_min)
  {
    dt *= _increase_factor;
    if (_verbose)
      _console << "Increasing dt (" << _dt_old << ") to " << dt
               << " since number of fixed point iterations (" << _fp_its << ") is < " << _target_min
               << "." << std::endl;
  }
  else if (_verbose)
  {
    _console << "Keeping dt (" << _dt_old << ") since number of fixed point iterations (" << _fp_its
             << ") is >= " << _target_min << " and <= " << _target_max << "." << std::endl;
  }
  _console << std::flush;

  return dt;
}

void
FixedPointIterationAdaptiveDT::acceptStep()
{
  TimeStepper::acceptStep();

  _fp_its = _executioner.fixedPointSolve().numFixedPointIts();
  _dt_old = _dt;
}
