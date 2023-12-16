//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorSum.h"

registerMooseObject("OptimizationApp", VectorSum);

InputParameters
VectorSum::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params += FunctionParserUtils<false>::validParams();
  params.addClassDescription(
      "Use a parsed function to iterate through a vector and reduce it scalar.");
  params.addParam<std::string>("name", "result", "Name of output reporter.");
  params.addRequiredCustomTypeParam<std::string>(
      "expression", "FunctionExpression", "function expression");
  params.addRequiredParam<ReporterName>("reporter_name", "Reporter name with vector to reduce.");
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

  // This reporters is for postprocessing optimization results and shold be exectuted at the end of
  // execution
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
  return params;
}

VectorSum::VectorSum(const InputParameters & parameters)
  : GeneralReporter(parameters),
    FunctionParserUtils(parameters),
    _use_t(getParam<bool>("use_t")),
    _initial_value(getParam<Real>("initial_value")),
    _vector(getReporterValueByName<std::vector<Real>>(getParam<ReporterName>("reporter_name"))),
    _output_reporter(
        declareValueByName<Real>(getParam<std::string>("name"), REPORTER_MODE_REPLICATED))
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
    paramError("expression", "Parsed function must contain the two symbols 'vi' and 'vplus'.");

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
VectorSum::finalize()
{
  Real reduction = _initial_value;
  for (std::size_t i = 0; i < _vector.size(); ++i)
  {
    _func_params[0] = reduction;
    _func_params[1] = _vector[i];

    if (_use_t)
      _func_params[2] = _t;

    reduction = evaluate(_func_F);
  }
  _output_reporter = reduction;
}
