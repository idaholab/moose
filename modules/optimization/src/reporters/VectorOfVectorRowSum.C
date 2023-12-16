//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorOfVectorRowSum.h"

registerMooseObject("OptimizationApp", VectorOfVectorRowSum);

InputParameters
VectorOfVectorRowSum::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params += FunctionParserUtils<false>::validParams();
  params.addClassDescription("Use a parsed function to iterate through a rows of a vector of "
                             "vector and reduce it to a vector.");

  params.addParam<std::string>("name", "result", "Name of output reporter.");
  params.addRequiredCustomTypeParam<std::string>(
      "expression", "FunctionExpression", "function expression");
  params.addRequiredParam<ReporterName>("reporter_vector_of_vectors",
                                        "Reporter name with vector of vectors to row reduce.");
  params.addRequiredParam<Real>("initial_value", "Value to intialize the reduction with.");
  params.addParam<std::vector<std::string>>(
      "constant_names",
      {},
      "Vector of constants used in the parsed function (use this for kB etc.)");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      {},
      "Vector of values for the constants in constant_names (can be an FParser expression)");
  params.addParam<bool>(
      "use_t", false, "Make time (t) variables available in the function expression.");
  // should execute on Timestep end to make sure data has been cloned into reporter from subapps
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
  return params;
}

VectorOfVectorRowSum::VectorOfVectorRowSum(const InputParameters & parameters)
  : GeneralReporter(parameters),
    FunctionParserUtils(parameters),
    _use_t(getParam<bool>("use_t")),
    _initial_value(getParam<Real>("initial_value")),
    _vec_of_vec_name(getParam<ReporterName>("reporter_vector_of_vectors")),
    _output_reporter(declareValueByName<std::vector<Real>>(getParam<std::string>("name"),
                                                           REPORTER_MODE_REPLICATED))
{
  // only two symbols for current (vi) and next (vplus) entries in vector
  std::string symbol_str("vi,vplus");

  // add time if required, probably would never need this but just in case
  if (_use_t)
    symbol_str += (symbol_str.empty() ? "" : ",") + std::string("t");

  // base function object
  _func_F = std::make_shared<SymFunction>();

  // set FParser internal feature flags
  setParserFeatureFlags(_func_F);

  // add the constant expressions
  addFParserConstants(_func_F,
                      getParam<std::vector<std::string>>("constant_names"),
                      getParam<std::vector<std::string>>("constant_expressions"));

  // parse function
  std::string function = getParam<std::string>("expression");
  // make sure the expression has the two required variables, vi and vplus
  if (function.find("vi") == std::string::npos || function.find("vplus") == std::string::npos)
    mooseError("Parsed function must contain the two symbols 'vi' and 'vplus'.");

  if (_func_F->Parse(function, symbol_str) >= 0)
    mooseError("Invalid parsed function\n", function, "\n", _func_F->ErrorMsg());

  // optimize
  if (!_disable_fpoptimizer)
    _func_F->Optimize();

  // just-in-time compile
  if (_enable_jit)
  {
    // let rank 0 do the JIT compilation first
    if (_communicator.rank() != 0)
      _communicator.barrier();

    _func_F->JITCompile();

    // wait for ranks > 0 to catch up
    if (_communicator.rank() == 0)
      _communicator.barrier();
  }

  // reserve storage for parameter passing buffer
  // 3 {current entry, next entry, time}
  _func_params.resize(3);
}

void
VectorOfVectorRowSum::finalize()
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
      mooseError("Every vector in 'reporter_vector_of_vectors=",
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
