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
AdaptAndModify::endStep()
{
  if (lastSolveConverged())
  {
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

    _problem.computeUserObjects(EXEC_CUSTOM);

    // if _reset_dt is true, force the output no matter what
    _problem.output(_reset_dt);
    _problem.outputPostprocessors(_reset_dt);

    _time_old = _time;
    _t_step++;

    _problem.copyOldSolutions();
  }
  else
  {
    _time_stepper->rejectStep();
    _problem.getNonlinearSystem()._time_scheme->rejectStep();
  }
}
