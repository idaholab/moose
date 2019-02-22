//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NumNonlinearIterations.h"

#include "FEProblem.h"
#include "SubProblem.h"
#include "FEProblemSolve.h"

registerMooseObject("MooseApp", NumNonlinearIterations);

template <>
InputParameters
validParams<NumNonlinearIterations>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addParam<bool>(
      "accumulate_over_step",
      false,
      "When set to true, accumulates to count the total over all Picard iterations for each step");
  params.addClassDescription("Outputs the number of nonlinear iterations");
  return params;
}

NumNonlinearIterations::NumNonlinearIterations(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _accumulate_over_step(getParam<bool>("accumulate_over_step")),
    _num_iters(0),
    _time(-std::numeric_limits<Real>::max())
{
}

void
NumNonlinearIterations::timestepSetup()
{
  if (_fe_problem.time() != _time)
  {
    _num_iters = 0;
    _time = _fe_problem.time();
  }
}

Real
NumNonlinearIterations::getValue()
{
  if (_accumulate_over_step)
    _num_iters += _fe_problem.getFEProblemSolve().nNonlinearIterations();
  else
    _num_iters = _fe_problem.getFEProblemSolve().nNonlinearIterations();

  return _num_iters;
}
