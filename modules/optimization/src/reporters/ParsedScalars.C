//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedScalars.h"

registerMooseObject("OptimizationApp", ParsedScalars);

InputParameters
ParsedScalars::validParams()
{
  InputParameters params = ParsedReporterBase::validParams();
  params.addRequiredParam<std::vector<ReporterName>>("reporter_names", "Reporter names ");
  params.addClassDescription("Apply parsed functions to scalar entries held in reporters.");
  return params;
}

ParsedScalars::ParsedScalars(const InputParameters & parameters)
  : ParsedReporterBase(parameters),
    _output_reporter(
        declareValueByName<Real>(getParam<std::string>("name"), REPORTER_MODE_REPLICATED))
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
    _reporter_data[i] = &getReporterValueByName<Real>(reporter_names[i], REPORTER_MODE_REPLICATED);
}

void
ParsedScalars::finalize()
{
  // check vector sizes of reporters
  const std::size_t n_rep(_reporter_data.size());

  for (std::size_t j = 0; j < n_rep; ++j)
    _func_params[j] = *(_reporter_data[j]);

  if (_use_t)
    _func_params[n_rep] = _t;

  _output_reporter = evaluate(_func_F);
}
