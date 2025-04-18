//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NumNonlinearIterations.h"

#include "FEProblem.h"
#include "SubProblem.h"
#include "SystemBase.h"

registerMooseObject("MooseApp", NumNonlinearIterations);

InputParameters
NumNonlinearIterations::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addParam<bool>(
      "accumulate_over_step",
      false,
      "When set to true, accumulates to count the total over all Picard iterations for each step");
  params.addClassDescription("Outputs the number of nonlinear iterations");

  // Not supported
  params.suppressParameter<bool>("use_displaced_mesh");
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

void
NumNonlinearIterations::finalize()
{
  if (_accumulate_over_step)
    _num_iters += _subproblem.nNonlinearIterations(_sys.number());
  else
    _num_iters = _subproblem.nNonlinearIterations(_sys.number());
}

Real
NumNonlinearIterations::getValue() const
{
  return _num_iters;
}
