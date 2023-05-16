//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeStepper.h"
#include "FEProblem.h"
#include "Transient.h"
#include "MooseApp.h"

InputParameters
TimeStepper::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.addParam<bool>(
      "reset_dt", false, "Use when restarting a calculation to force a change in dt.");
  params.addRangeCheckedParam<Real>(
      "cutback_factor_at_failure",
      0.5,
      "cutback_factor_at_failure>0 & cutback_factor_at_failure<1",
      "Factor to apply to timestep if a time step fails to converge.");
  params.addParam<bool>("enable", true, "whether or not to enable the time stepper");
  params.declareControllable("enable");

  params.registerBase("TimeStepper");
  params.registerSystemAttributeName("TimeStepper");

  return params;
}

TimeStepper::TimeStepper(const InputParameters & parameters)
  : MooseObject(parameters),
    Restartable(this, "TimeSteppers"),
    ScalarCoupleable(this),
    _fe_problem(parameters.have_parameter<FEProblemBase *>("_fe_problem_base")
                    ? *getParam<FEProblemBase *>("_fe_problem_base")
                    : *getParam<FEProblem *>("_fe_problem")),
    _executioner(*getCheckedPointerParam<Transient *>("_executioner")),
    _time(_fe_problem.time()),
    _time_old(_fe_problem.timeOld()),
    _t_step(_fe_problem.timeStep()),
    _dt(_fe_problem.dt()),
    _dt_min(_executioner.dtMin()),
    _dt_max(_executioner.dtMax()),
    _end_time(_executioner.endTime()),
    _sync_times(_app.getOutputWarehouse().getSyncTimes()),
    _timestep_tolerance(_executioner.timestepTol()),
    _verbose(_executioner.verbose()),
    _converged(true),
    _cutback_factor_at_failure(getParam<Real>("cutback_factor_at_failure")),
    _reset_dt(getParam<bool>("reset_dt")),
    _has_reset_dt(false),
    _failure_count(0),
    _current_dt(declareRestartableData<Real>("current_dt", 1.0))
{
}

TimeStepper::~TimeStepper() {}

void
TimeStepper::init()
{
}

void
TimeStepper::preExecute()
{
  // Delete all sync times that are at or before the begin time
  while (!_sync_times.empty() && _time + _timestep_tolerance >= *_sync_times.begin())
    _sync_times.erase(_sync_times.begin());
}

void
TimeStepper::computeStep()
{
  if (_t_step < 2 || (_reset_dt && !_has_reset_dt))
  {
    _has_reset_dt = true;

    if (converged())
      _current_dt = computeInitialDT();
    else
      _current_dt = computeFailedDT();
  }
  else
  {
    if (converged())
      _current_dt = computeDT();
    else
      _current_dt = computeFailedDT();
  }
  if (_current_dt < -TOLERANCE)
    mooseError("Negative time step detected :" + std::to_string(_current_dt) +
               " Investigate the TimeStepper to resolve this error");
}

bool
TimeStepper::constrainStep(Real & dt)
{
  bool at_sync_point = false;

  std::ostringstream diag;

  // Don't let the time step size exceed maximum time step size
  if (dt > _dt_max)
  {
    dt = _dt_max;
    diag << "Limiting dt to dtmax: " << std::setw(9) << std::setprecision(6) << std::setfill('0')
         << std::showpoint << std::left << _dt_max << std::endl;
  }

  // Don't allow time step size to be smaller than minimum time step size
  if (dt < _dt_min)
  {
    dt = _dt_min;
    diag << "Increasing dt to dtmin: " << std::setw(9) << std::setprecision(6) << std::setfill('0')
         << std::showpoint << std::left << _dt_min << std::endl;
  }

  // Don't let time go beyond simulation end time (unless we're doing a half transient)
  if (_time + dt > _end_time && !_app.halfTransient())
  {
    dt = _end_time - _time;
    diag << "Limiting dt for end_time: " << std::setw(9) << std::setprecision(6)
         << std::setfill('0') << std::showpoint << std::left << _end_time << " dt: " << std::setw(9)
         << std::setprecision(6) << std::setfill('0') << std::showpoint << std::left << dt
         << std::endl;
  }

  // Adjust to a sync time if supplied
  if (!_sync_times.empty() && _time + dt + _timestep_tolerance >= (*_sync_times.begin()))
  {
    dt = *_sync_times.begin() - _time;
    diag << "Limiting dt for sync_time: " << std::setw(9) << std::setprecision(6)
         << std::setfill('0') << std::showpoint << std::left << *_sync_times.begin()
         << " dt: " << std::setw(9) << std::setprecision(6) << std::setfill('0') << std::showpoint
         << std::left << dt << std::endl;

    if (dt <= 0.0)
    {
      _console << diag.str();
      mooseError("Adjusting to sync_time resulted in a non-positive time step.  dt: ",
                 dt,
                 " sync_time: ",
                 *_sync_times.begin(),
                 " time: ",
                 _time);
    }

    at_sync_point = true;
  }

  if (_verbose)
  {
    _console << diag.str();
  }

  return at_sync_point;
}

void
TimeStepper::step()
{
  _converged = _executioner.timeStepSolveObject()->solve();

  if (!_converged)
    _failure_count++;
}

void
TimeStepper::acceptStep()
{
  // If there are sync times at or before the current time, delete them
  while (!_sync_times.empty() && _time + _timestep_tolerance >= *_sync_times.begin())
  {
    _sync_times.erase(_sync_times.begin());
  }
}

void
TimeStepper::rejectStep()
{
  _fe_problem.restoreSolutions();
}

unsigned int
TimeStepper::numFailures() const
{
  return _failure_count;
}

bool
TimeStepper::converged() const
{
  return _converged;
}

Real
TimeStepper::computeFailedDT()
{
  if (_dt <= _dt_min)
    mooseError("Solve failed and timestep already at or below dtmin, cannot continue!");

  // cut the time step
  if (_cutback_factor_at_failure * _dt >= _dt_min)
    return _cutback_factor_at_failure * _dt;
  else // (_cutback_factor_at_failure * _current_dt < _dt_min)
    return _dt_min;
}

void
TimeStepper::forceTimeStep(Real dt)
{
  _current_dt = dt;
}

void
TimeStepper::forceNumSteps(const unsigned int num_steps)
{
  _executioner.forceNumSteps(num_steps);
}
