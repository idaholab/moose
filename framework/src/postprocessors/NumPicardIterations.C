//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NumPicardIterations.h"
#include "FEProblemBase.h"

registerMooseObject("MooseApp", NumPicardIterations);

template <>
InputParameters
validParams<NumPicardIterations>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  return params;
}

NumPicardIterations::NumPicardIterations(const InputParameters & parameters)
  : GeneralPostprocessor(parameters)
{
}

Real
NumPicardIterations::getValue()
{
  return _fe_problem.numPicardIts();
}
