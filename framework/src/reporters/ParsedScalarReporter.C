//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedScalarReporter.h"

registerMooseObject("MooseApp", ParsedScalarReporter);

InputParameters
ParsedScalarReporter::validParams()
{
  InputParameters params = ParsedReporterBase::validParams();
  params.addRequiredParam<std::vector<ReporterName>>("scalar_reporter_names",
                                                     "Scalar reporter names to apply function to.");
  params.set<std::vector<std::string>>("reporter_symbols") = {};
  params.suppressParameter<std::vector<std::string>>("reporter_symbols");
  params.addClassDescription("Applies parsed functions to scalar entries held in reporters.");
  return params;
}

ParsedScalarReporter::ParsedScalarReporter(const InputParameters & parameters)
  : ParsedReporterBase(parameters),
    _output_reporter(declareValueByName<Real>(getParam<std::string>("name"), REPORTER_MODE_ROOT))
{
  // get reporters to operate on
  const std::vector<ReporterName> scalar_reporter_names(
      getParam<std::vector<ReporterName>>("scalar_reporter_names"));

  if (scalar_reporter_names.size() != _scalar_reporter_symbols.size())
    paramError("scalar_reporter_names",
               "scalar_reporter_names and scalar_reporter_symbols must be the same size:  Number "
               "of scalar_reporter_names=",
               scalar_reporter_names.size(),
               ";  Number of scalar_reporter_symbols=",
               _scalar_reporter_symbols.size());
  _scalar_reporter_data.resize(scalar_reporter_names.size());
  for (const auto rep_index : index_range(_scalar_reporter_data))
    _scalar_reporter_data[rep_index] =
        &getReporterValueByName<Real>(scalar_reporter_names[rep_index], REPORTER_MODE_ROOT);
}

void
ParsedScalarReporter::finalize()
{
  for (const auto i : index_range(_scalar_reporter_data))
    _func_params[i] = *(_scalar_reporter_data[i]);

  if (_use_t)
    _func_params[_scalar_reporter_data.size()] = _t;

  _output_reporter = evaluate(_func_F);
}
