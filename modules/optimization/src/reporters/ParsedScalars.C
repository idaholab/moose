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
  InputParameters params = GeneralReporter::validParams();
  params += FunctionParserUtils<false>::validParams();
  params.addClassDescription("Apply parsed functions to scalar entries held in reporters.");
  params.addParam<std::string>("name", "result", "Name of output reporter.");
  params.addRequiredCustomTypeParam<std::string>(
      "expression", "FunctionExpression", "function expression");
  params.addRequiredParam<std::vector<ReporterName>>("reporter_names", "Reporter names ");
  params.addRequiredParam<std::vector<std::string>>("reporter_symbols",
                                                    "Expression symbol for each reporter");
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

ParsedScalars::ParsedScalars(const InputParameters & parameters)
  : GeneralReporter(parameters),
    FunctionParserUtils(parameters),
    _use_t(getParam<bool>("use_t")),
    _output_reporter(
        declareValueByName<Real>(getParam<std::string>("name"), REPORTER_MODE_REPLICATED))
{
  // Get symbols to corresponding reporter names
  // need symbols because reporter names have a "/" and that will not feed into fparser
  const std::vector<ReporterName> reporter_names(
      getParam<std::vector<ReporterName>>("reporter_names"));
  const std::vector<std::string> reporter_symbols(
      getParam<std::vector<std::string>>("reporter_symbols"));

  if (reporter_names.size() != reporter_symbols.size())
    mooseError(
        "reporter_names and reporter_symbols must be the same size: \n number of reporter_names=",
        reporter_names.size(),
        "\n number of reporter_symbols=",
        reporter_symbols.size());

  // build reporters argument
  std::string symbol_str;
  for (std::size_t i = 0; i < reporter_symbols.size(); ++i)
    symbol_str += (i == 0 ? "" : ",") + reporter_symbols[i];

  // add time if required
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
  _func_params.resize(reporter_names.size() + _use_t);

  // get reporters to operate on
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
