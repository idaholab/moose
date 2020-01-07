//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NumAdaptivityCycles.h"
#include "FEProblem.h"

registerMooseObject("MooseTestApp", NumAdaptivityCycles);

InputParameters
NumAdaptivityCycles::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  return params;
}

NumAdaptivityCycles::NumAdaptivityCycles(const InputParameters & parameters)
  : GeneralPostprocessor(parameters)
{
}

Real
NumAdaptivityCycles::getValue()
{
  return _fe_problem.getNumCyclesCompleted();
}
