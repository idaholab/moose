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

#include "NumNonlinearIterations.h"

#include "FEProblem.h"
#include "SubProblem.h"

template <>
InputParameters
validParams<NumNonlinearIterations>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addParam<bool>(
      "accumulate_over_step",
      false,
      "When set to true, accumulates to count the total over all Picard iterations for each step");
  return params;
}

NumNonlinearIterations::NumNonlinearIterations(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _fe_problem(dynamic_cast<FEProblemBase *>(&_subproblem)),
    _accumulate_over_step(getParam<bool>("accumulate_over_step")),
    _num_iters(0),
    _time(-std::numeric_limits<Real>::max())
{
  if (!_fe_problem)
    mooseError("Couldn't cast to FEProblemBase");
}

void
NumNonlinearIterations::timestepSetup()
{
  if (_fe_problem->time() != _time)
  {
    _num_iters = 0;
    _time = _fe_problem->time();
  }
}

Real
NumNonlinearIterations::getValue()
{
  if (_accumulate_over_step)
    _num_iters += _subproblem.nNonlinearIterations();
  else
    _num_iters = _subproblem.nNonlinearIterations();

  return _num_iters;
}
