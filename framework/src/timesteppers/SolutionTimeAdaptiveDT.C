//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolutionTimeAdaptiveDT.h"
#include "FEProblem.h"
#include "Transient.h"

#include <chrono>

registerMooseObject("MooseApp", SolutionTimeAdaptiveDT);

InputParameters
SolutionTimeAdaptiveDT::validParams()
{
  InputParameters params = TimeStepper::validParams();
  params.addClassDescription("Compute simulation timestep based on actual solution time.");
  params.addParam<Real>(
      "percent_change", 0.1, "Fraction to change the timestep by.  Should be between 0 and 1");
  params.addParam<int>(
      "initial_direction", 1, "Direction for the first step.  1 for up... -1 for down. ");
  params.addParam<bool>("adapt_log", false, "Output adaptive time step log");
  params.addRequiredParam<Real>("dt", "The timestep size between solves");

  return params;
}

SolutionTimeAdaptiveDT::SolutionTimeAdaptiveDT(const InputParameters & parameters)
  : TimeStepper(parameters),
    _direction(getParam<int>("initial_direction")),
    _percent_change(getParam<Real>("percent_change")),
    _older_sol_time_vs_dt(std::numeric_limits<Real>::max()),
    _old_sol_time_vs_dt(std::numeric_limits<Real>::max()),
    _sol_time_vs_dt(std::numeric_limits<Real>::max()),
    _adapt_log(getParam<bool>("adapt_log"))

{
  if ((_adapt_log) && (processor_id() == 0))
  {
    static const std::string log("adaptive_log");
    _adaptive_log.open(log);
    if (_adaptive_log.fail())
      mooseError("Unable to open file ", log);
    _adaptive_log << "Adaptive Times Step Log" << std::endl;
  }
}

SolutionTimeAdaptiveDT::~SolutionTimeAdaptiveDT() { _adaptive_log.close(); }

void
SolutionTimeAdaptiveDT::step()
{
  auto solve_start = std::chrono::system_clock::now();

  TimeStepper::step();

  if (converged())
  {
    auto solve_end = std::chrono::system_clock::now();
    auto elapsed_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(solve_end - solve_start).count();

    _older_sol_time_vs_dt = _old_sol_time_vs_dt;
    _old_sol_time_vs_dt = _sol_time_vs_dt;
    _sol_time_vs_dt = elapsed_time / _dt;
  }
}

Real
SolutionTimeAdaptiveDT::computeInitialDT()
{
  return getParam<Real>("dt");
}

Real
SolutionTimeAdaptiveDT::computeDT()
{
  // Ratio grew so switch direction
  if (_sol_time_vs_dt > _old_sol_time_vs_dt && _sol_time_vs_dt > _older_sol_time_vs_dt)
  {
    _direction *= -1;

    // Make sure we take at least two steps in this new direction
    _old_sol_time_vs_dt = std::numeric_limits<Real>::max();
    _older_sol_time_vs_dt = std::numeric_limits<Real>::max();
  }

  Real local_dt = _dt + _dt * _percent_change * _direction;

  if ((_adapt_log) && (processor_id() == 0))
  {
    Real out_dt = getCurrentDT();
    if (out_dt > _dt_max)
      out_dt = _dt_max;

    _adaptive_log << "***Time step: " << _t_step << ", time = " << _time + out_dt
                  << "\nCur DT: " << out_dt << "\nOlder Ratio: " << _older_sol_time_vs_dt
                  << "\nOld Ratio: " << _old_sol_time_vs_dt << "\nNew Ratio: " << _sol_time_vs_dt
                  << std::endl;
  }

  return local_dt;
}

void
SolutionTimeAdaptiveDT::rejectStep()
{
  _console << "Solve failed... cutting timestep" << std::endl;
  if (_adapt_log)
    _adaptive_log << "Solve failed... cutting timestep" << std::endl;

  TimeStepper::rejectStep();
}
