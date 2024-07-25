//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizationInfo.h"
#include "Optimize.h"

registerMooseObject("OptimizationApp", OptimizationInfo);

InputParameters
OptimizationInfo::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Reports Optimization Output");

  MultiMooseEnum items("current_iterate function_value gnorm cnorm xdiff");
  params.addParam<MultiMooseEnum>(
      "items",
      items,
      "The information to output, if nothing is provided everything will be output.");
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

OptimizationInfo::OptimizationInfo(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _optimization_executioner(dynamic_cast<Optimize *>(_app.getExecutioner())),
    _items(getParam<MultiMooseEnum>("items")),
    _functionValue(declareHelper<std::vector<double>>("function_value", REPORTER_MODE_REPLICATED)),
    _gnorm(declareHelper<std::vector<double>>("gnorm", REPORTER_MODE_REPLICATED)),
    _cnorm(declareHelper<std::vector<double>>("cnorm", REPORTER_MODE_REPLICATED)),
    _xdiff(declareHelper<std::vector<double>>("xdiff", REPORTER_MODE_REPLICATED)),
    _currentIterate(declareHelper<std::vector<int>>("current_iterate", REPORTER_MODE_REPLICATED)),
    _objectiveIterate(
        (!_items.isValid() || _items.isValueSet("current_iterate"))
            ? declareValueByName<std::vector<int>>("objective_iterate", REPORTER_MODE_REPLICATED)
            : declareUnusedValue<std::vector<int>>()),
    _gradientIterate(
        (!_items.isValid() || _items.isValueSet("current_iterate"))
            ? declareValueByName<std::vector<int>>("gradient_iterate", REPORTER_MODE_REPLICATED)
            : declareUnusedValue<std::vector<int>>()),
    _hessianIterate(
        (!_items.isValid() || _items.isValueSet("current_iterate"))
            ? declareValueByName<std::vector<int>>("hessian_iterate", REPORTER_MODE_REPLICATED)
            : declareUnusedValue<std::vector<int>>()),
    _functionSolves(
        (!_items.isValid() || _items.isValueSet("current_iterate"))
            ? declareValueByName<std::vector<int>>("function_solves", REPORTER_MODE_REPLICATED)
            : declareUnusedValue<std::vector<int>>())
{
  if (!_optimization_executioner)
    mooseError("The OptimizationInfo Reporter can only be used with a Optimize Executioner");
}

void
OptimizationInfo::execute()
{
  _optimization_executioner->getOptimizeSolve().getTaoSolutionStatus(_currentIterate,
                                                                     _gnorm,
                                                                     _objectiveIterate,
                                                                     _cnorm,
                                                                     _gradientIterate,
                                                                     _xdiff,
                                                                     _hessianIterate,
                                                                     _functionValue,
                                                                     _functionSolves);
}
