//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FailingProblem.h"

#include "MooseApp.h"

registerMooseObject("MooseTestApp", FailingProblem);

template <>
InputParameters
validParams<FailingProblem>()
{
  InputParameters params = validParams<FEProblem>();
  params.addRequiredParam<unsigned int>("fail_step", "The timestep to fail");
  return params;
}

FailingProblem::FailingProblem(const InputParameters & params)
  : FEProblem(params), _failed(false), _fail_step(getParam<unsigned int>("fail_step"))
{
}

bool
FailingProblem::solve()
{
  bool converged = FEProblem::solve();
  // fail for the first time of solving this step
  if (!_failed && (_t_step == static_cast<int>(_fail_step)))
  {
    _failed = true;
    return false;
  }

  return converged;
}
