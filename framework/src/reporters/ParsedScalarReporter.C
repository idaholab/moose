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
  params.suppressParameter<std::vector<std::string>>("vector_reporter_symbols");
  params.addClassDescription("Applies parsed functions to scalar entries held in reporters.");
  return params;
}

ParsedScalarReporter::ParsedScalarReporter(const InputParameters & parameters)
  : ParsedReporterBase(parameters),
    _output_reporter(declareValueByName<Real>(getParam<std::string>("name"), REPORTER_MODE_ROOT))
{
  if (!parameters.isParamValid("scalar_reporter_names"))
    paramError("scalar_reporter_names", "Required parameter when using ParsedScalarReporter.");
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
