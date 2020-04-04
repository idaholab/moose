//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalVariableVectorPostprocessor.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseTypes.h"

InputParameters
NodalVariableVectorPostprocessor::validParams()
{
  InputParameters params = NodalVectorPostprocessor::validParams();
  params.addRequiredCoupledVar(
      "variable", "The names of the variables that this VectorPostprocessor operates on");
  return params;
}

NodalVariableVectorPostprocessor::NodalVariableVectorPostprocessor(
    const InputParameters & parameters)
  : NodalVectorPostprocessor(parameters)
{
}
