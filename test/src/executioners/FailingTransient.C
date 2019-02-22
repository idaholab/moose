//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FailingTransient.h"

#include "MooseApp.h"

registerMooseObject("MooseTestApp", FailingTransient);

template <>
InputParameters
validParams<FailingTransient>()
{
  InputParameters params = validParams<Transient>();
  params.addRequiredParam<unsigned int>("fail_step", "The timestep to fail");
  return params;
}

FailingTransient::FailingTransient(const InputParameters & params)
  : Transient(params), _failed(false), _fail_step(getParam<unsigned int>("fail_step"))
{
}

bool
FailingTransient::augmentedFEProblemSolveFail()
{
  if (!_failed && (_t_step == static_cast<int>(_fail_step)))
  {
    _failed = true;
    return true;
  }

  return false;
}
