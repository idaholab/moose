//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedVectorVectorRealReductionReporter.h"

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
  // This reporter is for postprocessing optimization results and shold be exectuted at the end of
  // execution
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
  return params;
}

ParsedVectorVectorRealReductionReporter::ParsedVectorVectorRealReductionReporter(
    const InputParameters & parameters)
  : ParsedReporterBase(parameters),
    _initial_value(getParam<Real>("initial_value")),
    _vec_of_vec_name(getParam<ReporterName>("reporter_name")),
    _output_reporter(declareValueByName<std::vector<Real>>(getParam<std::string>("name"),
                                                           REPORTER_MODE_REPLICATED))
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
  // Reporter is gotten in finalize instead of intial_setup because it needs to work for
  // StochasticReporter which is cloned into by SamplerReporterTransfer
  const std::vector<std::vector<Real>> * vector_vector =
      &getReporterValueByName<std::vector<std::vector<Real>>>(_vec_of_vec_name,
                                                              REPORTER_MODE_REPLICATED);
  std::size_t nrows(vector_vector->at(0).size());
  std::size_t ncols(vector_vector->size());

  for (auto & reporter_vector : *vector_vector)
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
  for (std::size_t i_row = 0; i_row < nrows; ++i_row)
  {
    Real reduction = _initial_value;
    for (std::size_t j_col = 0; j_col < ncols; ++j_col)
    {
      _func_params[0] = reduction;
      _func_params[1] = vector_vector->at(j_col)[i_row];

      if (_use_t)
        _func_params[2] = _t;

      reduction = evaluate(_func_F);
    }
    _output_reporter[i_row] = reduction;
  }
}
