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

#include "SolutionTimeAdaptiveStepper.h"
#include "FEProblem.h"
#include "Transient.h"
#include "MooseApp.h"

template<>
InputParameters validParams<SolutionTimeAdaptiveStepper>()
{
  InputParameters params = validParams<Stepper>();

  params.addParam<Real>("percent_change", 0.1, "Percentage to change the timestep by.  Should be between 0 and 1");
  params.addParam<int>("initial_direction", 1, "Direction for the first step.  1 for up... -1 for down. ");
  params.addRequiredParam<Real>("dt", "Initial timestep size.");

  return params;
}

SolutionTimeAdaptiveStepper::SolutionTimeAdaptiveStepper(const InputParameters & parameters) :
    Stepper(parameters),
    _direction(getParam<int>("initial_direction")),
    _percent_change(getParam<Real>("percent_change")),
    _input_dt(getParam<Real>("dt"))
{
}

Real
SolutionTimeAdaptiveStepper::computeInitialDT()
{
  return _input_dt;
}

Real
SolutionTimeAdaptiveStepper::computeDT()
{
  if (_step_count == 0)
    return _input_dt;

  Real ratio = _solve_time_secs[0] / _dt[0];
  Real prev_ratio = _solve_time_secs[1] / _dt[1];
  Real prev_prev_ratio = _solve_time_secs[2] / _dt[2];

  _n_steps++;

  if (ratio > prev_ratio && ratio > prev_prev_ratio && _n_steps > 1)
  {
    _direction *= -1;
    _n_steps = 0;
  }

  return _dt[0] + _dt[0] * _percent_change * _direction;
}

Real
SolutionTimeAdaptiveStepper::computeFailedDT()
{
  return 0.5 * _dt[0]; // _dt[0] was the dt actually used in the last timestep
}
