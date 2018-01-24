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

#include "AdaptAndModify.h"
#include "TimeStepper.h"

// Moose includes

template <>
InputParameters
validParams<AdaptAndModify>()
{
  InputParameters params = validParams<Transient>();
  params.addParam<unsigned int>("adapt_cycles", 1, "Number of adaptivity cycles to do.");
  return params;
}

AdaptAndModify::AdaptAndModify(const InputParameters & parameters)
  : Transient(parameters), _adapt_cycles(parameters.get<unsigned int>("adapt_cycles"))
{
}

void
AdaptAndModify::incrementStepOrReject()
{
  if (_last_solve_converged)
  {
    _time_old = _time;
    _t_step++;

    _problem.advanceState();
  }
  else
  {
    _time_stepper->rejectStep();
    _time = _time_old;
  }

  _first = false;
}

void
AdaptAndModify::endStep(Real input_time)
{
  if (input_time == -1.0)
    _time = _time_old + _dt;
  else
    _time = input_time;

  _last_solve_converged = lastSolveConverged();
  if (_last_solve_converged)
  {
    // Compute the Error Indicators and Markers
    for (unsigned int i = 0; i < _adapt_cycles; i++)
    {
      // Compute the Error Indicators and Markers
      _problem.computeIndicators();
      _problem.computeMarkers();

#ifdef LIBMESH_ENABLE_AMR
      if (_problem.adaptivity().isOn())
        _problem.adaptMesh();

#endif
    }
    _problem.computeUserObjects(EXEC_CUSTOM, Moose::ALL);

    // Set the time for the next output interval if we're at or beyond an output interval
    if (_time_interval && (_time + _timestep_tolerance >= _next_interval_output_time))
      _next_interval_output_time += _time_interval_output_interval;
  }

  _problem.outputStep(EXEC_TIMESTEP_END);
}
