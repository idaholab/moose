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
  return params;
}

TimeStepper::TimeStepper(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    _fe_problem(*getParam<FEProblem *>("_fe_problem")),
    _executioner(*getParam<Transient *>("_executioner")),
    _time(_fe_problem.time()),
    _time_old(_fe_problem.timeOld()),
    _t_step(_fe_problem.timeStep()),
    _dt(_fe_problem.dt()),
    _dt_min(_executioner.dtMin()),
    _dt_max(_executioner.dtMax()),
    _converged(true),
    _current_dt(1.0)
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
TimeStepper::computeStep()
{
  if (_t_step < 2)
  {
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

void
TimeStepper::step()
{
  _fe_problem.solve();
  _converged = _fe_problem.converged();
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
