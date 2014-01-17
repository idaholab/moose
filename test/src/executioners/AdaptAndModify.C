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

//Moose includes

template<>
InputParameters validParams<AdaptAndModify>()
{
  InputParameters params = validParams<Transient>();
  params.addParam<unsigned int>("adapt_cycles", 1, "Number of adaptivity cylces to do.");
  return params;
}

AdaptAndModify::AdaptAndModify(const std::string & name, InputParameters parameters) :
    Transient(name, parameters),
    _adapt_cycles(parameters.get<unsigned int>("adapt_cycles"))
{}

void
AdaptAndModify::incrementStepOrReject()
{
  if(_last_solve_converged)
  {
    _time_old = _time;
    _t_step++;

    _problem.copyOldSolutions();
  }
  else
  {
    _time_stepper->rejectStep();
    _time = _time_old;
  }

  _first = false;
}

void
AdaptAndModify::endStep()
{
  _last_solve_converged = lastSolveConverged();
  if (_last_solve_converged)
  {
    // Compute the Error Indicators and Markers
    for(unsigned int i=0; i<_adapt_cycles; i++)
    {
      // Compute the Error Indicators and Markers
      _problem.computeIndicatorsAndMarkers();

#ifdef LIBMESH_ENABLE_AMR
      if (_problem.adaptivity().isOn())
      {
        _problem.adaptMesh();
        _problem.out().meshChanged();
      }
#endif
    }

    //output
    if(_time_interval)
    {
      //Force output if the current time is at an output interval
      if(std::abs(_time-_next_interval_output_time)<=_timestep_tolerance
         || (_problem.out().interval() > 1 && _t_step % _problem.out().interval() == 0))
      {
        if(_allow_output)
        {
          _problem.output(true);
          _problem.outputPostprocessors(true);
          _problem.outputRestart();
        }
      }
      //Set the time for the next output interval if we're at or beyond an output interval
      if (_time + _timestep_tolerance >= _next_interval_output_time)
      {
        _next_interval_output_time += _time_interval_output_interval;
      }
    }
    else
    {
      // if _at_sync_point is true, force the output no matter what
      if(_allow_output)
      {
        _problem.output(_at_sync_point);
        _problem.outputPostprocessors(_at_sync_point);
        _problem.outputRestart();
      }
    }
  }
}


