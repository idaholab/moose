//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NumLinearIterations.h"

#include "FEProblemBase.h"
#include "FEProblemSolve.h"

registerMooseObject("MooseApp", NumLinearIterations);

template <>
InputParameters
validParams<NumLinearIterations>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  return params;
}

NumLinearIterations::NumLinearIterations(const InputParameters & parameters)
  : GeneralPostprocessor(parameters)
{
}

Real
NumLinearIterations::getValue()
{
  return _fe_problem.getFEProblemSolve().nLinearIterations();
}
