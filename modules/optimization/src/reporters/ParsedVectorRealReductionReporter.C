//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedVectorRealReductionReporter.h"

registerMooseObject("OptimizationApp", ParsedVectorRealReductionReporter);

InputParameters
ParsedVectorRealReductionReporter::validParams()
{
  InputParameters params = ParsedReporterBase::validParams();
  params.addClassDescription(
      "Use a parsed function to iterate through a vector and reduce it scalar.");
  params.addRequiredParam<ReporterName>("reporter_name", "Reporter name with vector to reduce.");
  params.addRequiredParam<Real>("initial_value", "Value to intialize the reduction with.");
  // reporter_symbols are the two symbols for reduction value and current value for the reduction
  // operation, these symbols are enforced in the constructor with a mooseError
  params.set<std::vector<std::string>>("reporter_symbols") = {"reduction_value", "indexed_value"};
  // This reporter is for postprocessing optimization results and shold be exectuted at the end of
  // execution
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
  return params;
}

ParsedVectorRealReductionReporter::ParsedVectorRealReductionReporter(
    const InputParameters & parameters)
  : ParsedReporterBase(parameters),
    _initial_value(getParam<Real>("initial_value")),
    _reporter_data(
        getReporterValueByName<std::vector<Real>>(getParam<ReporterName>("reporter_name"))),
    _output_reporter(
        declareValueByName<Real>(getParam<std::string>("name"), REPORTER_MODE_REPLICATED))
{
  // parse function
  std::string function = getParam<std::string>("expression");
  // make sure the expression has the two required variables, vi and vplus
  if (function.find("reduction_value") == std::string::npos ||
      function.find("indexed_value") == std::string::npos)
    paramError(
        "expression",
        "Parsed function must contain the two symbols 'reduction_value' and 'indexed_value'.");
}

void
ParsedVectorRealReductionReporter::finalize()
{
  Real reduction = _initial_value;
  for (std::size_t i = 0; i < _reporter_data.size(); ++i)
  {
    _func_params[0] = reduction;
    _func_params[1] = _reporter_data[i];

    if (_use_t)
      _func_params[2] = _t;

    reduction = evaluate(_func_F);
  }
  _output_reporter = reduction;
}
