//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedVectors.h"

registerMooseObject("OptimizationApp", ParsedVectors);

InputParameters
ParsedVectors::validParams()
{
  InputParameters params = ParsedReporterBase::validParams();
  params.addClassDescription("Apply parsed functions to vector entries held in reporters.");
  params.addRequiredParam<std::vector<ReporterName>>("reporter_names", "Reporter names ");
  return params;
}

ParsedVectors::ParsedVectors(const InputParameters & parameters)
  : ParsedReporterBase(parameters),
    _output_reporter(declareValueByName<std::vector<Real>>(getParam<std::string>("name"),
                                                           REPORTER_MODE_REPLICATED))
{
  // get reporters to operate on
  const std::vector<ReporterName> reporter_names(
      getParam<std::vector<ReporterName>>("reporter_names"));
  if (reporter_names.size() != _reporter_symbols.size())
    mooseError(
        "reporter_names and reporter_symbols must be the same size: \n number of reporter_names=",
        reporter_names.size(),
        "\n number of reporter_symbols=",
        _reporter_symbols.size());

  _reporter_data.resize(reporter_names.size());
  for (unsigned int i = 0; i < reporter_names.size(); i++)
    _reporter_data[i] =
        &getReporterValueByName<std::vector<Real>>(reporter_names[i], REPORTER_MODE_REPLICATED);
}

void
ParsedVectors::finalize()
{
  // check vector sizes of reporters
  const std::size_t n_rep(_reporter_data.size());
  const std::size_t entries(_reporter_data[0]->size());
  for (std::size_t j = 0; j < n_rep; ++j)
    if (entries != _reporter_data[j]->size())
    {
      const std::vector<ReporterName> reporter_names(
          getParam<std::vector<ReporterName>>("reporter_names"));
      mooseError("All vectors being operated on must be the same size.",
                 "\nsize of ",
                 reporter_names[0].getCombinedName(),
                 " = ",
                 entries,
                 "\nsize of ",
                 reporter_names[j].getCombinedName(),
                 " = ",
                 _reporter_data[j]->size());
    }

  _output_reporter.resize(entries, 0.0);
  for (std::size_t i = 0; i < entries; ++i)
  {
    for (std::size_t j = 0; j < n_rep; ++j)
      _func_params[j] = _reporter_data[j]->at(i);

    if (_use_t)
      _func_params[n_rep] = _t;

    _output_reporter[i] = evaluate(_func_F);
  }
}
