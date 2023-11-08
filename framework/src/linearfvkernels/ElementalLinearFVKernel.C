//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementalLinearFVKernel.h"
#include "Assembly.h"
#include "SubProblem.h"

InputParameters
ElementalLinearFVKernel::validParams()
{
  InputParameters params = LinearFVKernel::validParams();
  return params;
}

ElementalLinearFVKernel::ElementalLinearFVKernel(const InputParameters & params)
  : LinearFVKernel(params)
{
}
