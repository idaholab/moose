//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedScalarReporter.h"

registerMooseObject("OptimizationApp", ParsedScalarReporter);

InputParameters
ParsedScalarReporter::validParams()
{
  InputParameters params = ParsedReporterBase::validParams();
  params.addRequiredParam<std::vector<ReporterName>>("reporter_names", "The input reporter names");
  params.addClassDescription("Applies parsed functions to scalar entries held in reporters.");
  return params;
}

ParsedScalarReporter::ParsedScalarReporter(const InputParameters & parameters)
  : ParsedReporterBase(parameters),
    _output_reporter(declareValueByName<Real>(getParam<std::string>("name"), REPORTER_MODE_ROOT))
{
  // get reporters to operate on
  const std::vector<ReporterName> reporter_names(
      getParam<std::vector<ReporterName>>("reporter_names"));
  if (reporter_names.size() != _reporter_symbols.size())
    paramError(
        "reporter_names",
        "reporter_names and reporter_symbols must be the same size:  Number of reporter_names=",
        reporter_names.size(),
        ";  Number of reporter_symbols=",
        _reporter_symbols.size());

  _reporter_data.resize(reporter_names.size());
  for (const auto rep_index : index_range(_reporter_data))
    _reporter_data[rep_index] =
        &getReporterValueByName<Real>(reporter_names[rep_index], REPORTER_MODE_ROOT);
}

void
ParsedScalarReporter::finalize()
{
  for (const auto i : index_range(_reporter_data))
    _func_params[i] = *(_reporter_data[i]);

  if (_use_t)
    _func_params[_reporter_data.size()] = _t;

  _output_reporter = evaluate(_func_F);
}
