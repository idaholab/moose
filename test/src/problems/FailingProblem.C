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

InputParameters
FailingProblem::validParams()
{
  InputParameters params = FEProblem::validParams();
  params.addRequiredParam<std::vector<unsigned int>>("fail_steps", "The timestep(s) to fail");
  return params;
}

FailingProblem::FailingProblem(const InputParameters & params)
  : FEProblem(params), _fail_steps(getParam<std::vector<unsigned int>>("fail_steps"))
{
  std::sort(_fail_steps.begin(), _fail_steps.end(), std::greater<unsigned int>());
}

bool
FailingProblem::converged()
{
  if (_fail_steps.size() > 0)
  {
    if ((unsigned int)_t_step == _fail_steps.back())
    {
      _fail_steps.pop_back();
      return false;
    }
  }

  return FEProblemBase::converged();
}
