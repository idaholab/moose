//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalPostprocessor.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseTypes.h"

template <>
InputParameters
validParams<NodalPostprocessor>()
{
  InputParameters params = validParams<NodalUserObject>();
  params += validParams<Postprocessor>();
  return params;
}

NodalPostprocessor::NodalPostprocessor(const InputParameters & parameters)
  : NodalUserObject(parameters), Postprocessor(parameters)
{
}
