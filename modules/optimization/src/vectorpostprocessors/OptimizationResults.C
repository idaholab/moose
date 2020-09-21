//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizationResults.h"

registerMooseObject("isopodApp", OptimizationResults);

InputParameters
OptimizationResults::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription("Prints parameter results from optimization routine.");
  return params;
}

OptimizationResults::OptimizationResults(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _results(declareVector(_app.getOutputFileBase() + "_parameters.csv"))
{
}

void
OptimizationResults::setParameterValues(const VectorPostprocessorValue & current)
{
  _results = current;
}
