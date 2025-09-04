//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizationTestFunction.h"

InputParameters
OptimizationTestFunction::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<std::vector<Real>>("x", "Input values.");
  params.declareControllable("x");
  return params;
}

OptimizationTestFunction::OptimizationTestFunction(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _x(getParam<std::vector<Real>>("x"))
{
}

PostprocessorValue
OptimizationTestFunction::getValue() const
{
  return function(_x);
}
