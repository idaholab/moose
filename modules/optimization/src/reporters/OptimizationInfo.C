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
#include "isopodApp.h"
#include "IsopodAppTypes.h"

registerMooseObject("isopodApp", OptimizationInfo);

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
  auto & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_FORWARD, EXEC_ADJOINT, EXEC_HESSIAN);
  return params;
}

OptimizationInfo::OptimizationInfo(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _optimization_executioner(dynamic_cast<Optimize *>(_app.getExecutioner())),
    _items(getParam<MultiMooseEnum>("items")),
    _currentIterate(declareHelper<int>("current_iterate", REPORTER_MODE_REPLICATED)),
    _objectiveIterate((!_items.isValid() || _items.contains("current_iterate"))
                          ? declareValueByName<int>("objective_iterate", REPORTER_MODE_REPLICATED)
                          : declareUnusedValue<int>()),
    _gradientIterate((!_items.isValid() || _items.contains("current_iterate"))
                         ? declareValueByName<int>("gradient_iterate", REPORTER_MODE_REPLICATED)
                         : declareUnusedValue<int>()),
    _hessianIterate((!_items.isValid() || _items.contains("current_iterate"))
                        ? declareValueByName<int>("hessian_i:wterate", REPORTER_MODE_REPLICATED)
                        : declareUnusedValue<int>()),
    _functionValue(declareHelper<double>("function_value", REPORTER_MODE_REPLICATED)),
    _gnorm(declareHelper<double>("gnorm", REPORTER_MODE_REPLICATED)),
    _cnorm(declareHelper<double>("cnorm", REPORTER_MODE_REPLICATED)),
    _xdiff(declareHelper<double>("xdiff", REPORTER_MODE_REPLICATED))
{
  if (!_optimization_executioner)
    mooseError("The OptimizationInfo Reporter can only be used with a Optimize Executioner");
}

void
OptimizationInfo::initialize()
{
}

void
OptimizationInfo::execute()
{
  std::cout << "HERE" << std::endl;
  _optimization_executioner->getOptimizeSolve().getTaoSolutionStatus(_currentIterate,
                                                                     _gnorm,
                                                                     _objectiveIterate,
                                                                     _cnorm,
                                                                     _gradientIterate,
                                                                     _xdiff,
                                                                     _hessianIterate,
                                                                     _functionValue);
}
