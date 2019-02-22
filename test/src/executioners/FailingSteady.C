//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FailingSteady.h"

#include "MooseApp.h"

registerMooseObject("MooseTestApp", FailingSteady);

template <>
InputParameters
validParams<FailingSteady>()
{
  InputParameters params = validParams<Steady>();
  params.addRequiredParam<unsigned int>("fail_step", "The timestep to fail");
  return params;
}

FailingSteady::FailingSteady(const InputParameters & params)
  : Steady(params), _failed(false), _fail_step(getParam<unsigned int>("fail_step"))
{
}

bool
FailingSteady::augmentedFEProblemSolveFail()
{
  if (!_failed && (_time_step == static_cast<int>(_fail_step)))
  {
    _failed = true;
    return true;
  }

  return false;
}
