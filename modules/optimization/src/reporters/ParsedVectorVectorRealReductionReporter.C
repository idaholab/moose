//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedVectorVectorRealReductionReporter.h"
#include <cstddef>

registerMooseObject("OptimizationApp", ParsedVectorVectorRealReductionReporter);

InputParameters
ParsedVectorVectorRealReductionReporter::validParams()
{
  InputParameters params = ParsedReporterBase::validParams();
  params.addClassDescription("Use a parsed function to iterate through a rows of a vector of "
                             "vector and reduce it to a vector.");
  params.addRequiredParam<ReporterName>("reporter_name",
                                        "Reporter name with vector of vectors to reduce.");
  params.addRequiredParam<Real>("initial_value", "Value to intialize the reduction with.");

  // reporter_symbols are the two symbols for reduction value and current value for the reduction
  // operation, these symbols are enforced in the constructor with a mooseError
  params.set<std::vector<std::string>>("reporter_symbols") = {"reduction_value", "indexed_value"};
  params.suppressParameter<std::vector<std::string>>("reporter_symbols");

  // This reporter is for postprocessing optimization results and should be exectuted at the end of
  // execution
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
  return params;
}

ParsedVectorVectorRealReductionReporter::ParsedVectorVectorRealReductionReporter(
    const InputParameters & parameters)
  : ParsedReporterBase(parameters),
    _initial_value(getParam<Real>("initial_value")),
    _vec_of_vec_name(getParam<ReporterName>("reporter_name")),
    _output_reporter(
        declareValueByName<std::vector<Real>>(getParam<std::string>("name"), REPORTER_MODE_ROOT)),
    _reporter_data(getReporterValueByName<std::vector<std::vector<Real>>>(
        getParam<ReporterName>("reporter_name")))
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
ParsedVectorVectorRealReductionReporter::finalize()
{
  std::size_t ncols = _reporter_data.size();
  std::size_t nrows = 0;
  if (!_reporter_data.empty())
    nrows = _reporter_data[0].size();

  for (auto & reporter_vector : _reporter_data)
  {
    if (reporter_vector.size() != nrows)
      mooseError("Every vector in 'reporter_name=",
                 _vec_of_vec_name,
                 "' must be the same size.",
                 "\nFirst Vector size = ",
                 nrows,
                 "\nCurrent Vector size = ",
                 reporter_vector.size());
  }

  _output_reporter.clear();
  _output_reporter.resize(nrows, _initial_value);
  for (const auto i_row : make_range(nrows))
  {
    Real reduction = _initial_value;
    for (const auto j_col : make_range(ncols))
    {
      _func_params[0] = reduction;
      _func_params[1] = _reporter_data[j_col][i_row];

      if (_use_t)
        _func_params[2] = _t;

      reduction = evaluate(_func_F);
    }
    _output_reporter[i_row] = reduction;
  }
}
