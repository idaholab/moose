//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalVectorPostprocessor.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseTypes.h"

template <>
InputParameters
validParams<NodalVectorPostprocessor>()
{
  InputParameters params = validParams<NodalUserObject>();
  params += validParams<VectorPostprocessor>();
  return params;
}

NodalVectorPostprocessor::NodalVectorPostprocessor(const InputParameters & parameters)
  : NodalUserObject(parameters), VectorPostprocessor(parameters)
{
}
