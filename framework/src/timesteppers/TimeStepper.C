/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "TimeStepper.h"
#include "FEProblem.h"
#include "Transient.h"

template<>
InputParameters validParams<TimeStepper>()
{
  InputParameters params = validParams<MooseObject>();

  params.addPrivateParam<std::string>("built_by_action", "setup_time_stepper");
  params.addParam<bool>("reset_dt", false, "Use when restarting a calculation to force a change in dt.");
  return params;
}

TimeStepper::TimeStepper(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    Restartable(name, parameters, "TimeSteppers"),
    _fe_problem(*getParam<FEProblem *>("_fe_problem")),
    _executioner(*getParam<Transient *>("_executioner")),
    _time(_fe_problem.time()),
    _time_old(_fe_problem.timeOld()),
    _t_step(_fe_problem.timeStep()),
    _dt(_fe_problem.dt()),
    _dt_min(_executioner.dtMin()),
    _dt_max(_executioner.dtMax()),
    _end_time(_executioner.endTime()),
    _sync_times(_executioner.syncTimes()),
    _timestep_tolerance(_executioner.timestepTol()),
    _converged(true),
    _reset_dt(getParam<bool>("reset_dt")),
    _has_reset_dt(false),
    _current_dt(declareRestartableData("current_dt",1.0))
{
}

TimeStepper::~TimeStepper()
{
}

void
TimeStepper::init()
{
}

void
TimeStepper::preExecute()
{
  // Delete all sync times that are at or before the begin time
  while (!_sync_times.empty() && _time + _timestep_tolerance >= *_sync_times.begin())
  {
    _sync_times.erase(_sync_times.begin());
  }
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
}

bool
TimeStepper::constrainStep(Real &dt)
{
  bool force_output = false;

  // Don't let the time step size exceed maximum time step size
  if (dt > _dt_max)
    dt = _dt_max;

  // Don't allow time step size to be smaller than minimum time step size
  if (dt < _dt_min)
    dt = _dt_min;

  // Don't let time go beyond simulation end time
  if (_time + dt > _end_time)
    dt = _end_time - _time;

  // Adjust to a sync time if supplied
  if (!_sync_times.empty() && _time + dt + _timestep_tolerance >= (*_sync_times.begin()))
  {
    dt = *_sync_times.begin() - _time;

    if (dt <= 0.0)
      mooseError("Adjusting to sync time resulted in a non-positive time step.  dt: "<<dt<<" sync time: "<<*_sync_times.begin()<<" time: "<<_time);

    force_output = true;
  }

  return force_output;
}

void
TimeStepper::step()
{
  _fe_problem.solve();
  _converged = _fe_problem.converged();
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

bool
TimeStepper::converged()
{
  return _converged;
}

Real
TimeStepper::computeFailedDT()
{
  if (_dt <= _dt_min)
    mooseError("Solve failed and timestep already at or below dtmin, cannot continue!");

  // cut the time step in a half
  if (0.5 * _dt >= _dt_min)
    return 0.5 * _dt;
  else // (0.5 * _current_dt < _dt_min)
    return _dt_min;
}

void
TimeStepper::forceTimeStep(Real dt)
{
  _current_dt = dt;
}
