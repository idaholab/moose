//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedVectorReporter.h"

registerMooseObject("OptimizationApp", ParsedVectorReporter);

InputParameters
ParsedVectorReporter::validParams()
{
  InputParameters params = ParsedReporterBase::validParams();
  params.addClassDescription("Apply parsed functions to vector entries held in reporters.");
  params.addRequiredParam<std::vector<ReporterName>>("reporter_names", "Reporter names ");
  return params;
}

ParsedVectorReporter::ParsedVectorReporter(const InputParameters & parameters)
  : ParsedReporterBase(parameters),
    _output_reporter(
        declareValueByName<std::vector<Real>>(getParam<std::string>("name"), REPORTER_MODE_ROOT))
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
        &getReporterValueByName<std::vector<Real>>(reporter_names[rep_index], REPORTER_MODE_ROOT);
}

void
ParsedVectorReporter::finalize()
{
  // check vector sizes of reporters
  const std::size_t entries(_reporter_data[0]->size());
  for (const auto rep_index : index_range(_reporter_data))
    if (entries != _reporter_data[rep_index]->size())
    {
      const std::vector<ReporterName> reporter_names(
          getParam<std::vector<ReporterName>>("reporter_names"));
      mooseError("All vectors being operated on must be the same size.",
                 "\nsize of ",
                 reporter_names[0].getCombinedName(),
                 " = ",
                 entries,
                 "\nsize of ",
                 reporter_names[rep_index].getCombinedName(),
                 " = ",
                 _reporter_data[rep_index]->size());
    }

  _output_reporter.resize(entries, 0.0);
  for (const auto i : make_range(entries))
  {
    for (const auto rep_index : index_range(_reporter_data))
      _func_params[rep_index] = _reporter_data[rep_index]->at(i);

    if (_use_t)
      _func_params[_reporter_data.size()] = _t;

    _output_reporter[i] = evaluate(_func_F);
  }
}
