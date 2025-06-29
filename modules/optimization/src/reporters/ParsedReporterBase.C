//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedReporterBase.h"

// registerMooseObject("OptimizationApp", ParsedReporterBase);

InputParameters
ParsedReporterBase::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params += FunctionParserUtils<false>::validParams();
  params.addRequiredCustomTypeParam<std::string>(
      "expression", "FunctionExpression", "function expression");
  params.addParam<std::string>("name", "result", "Name of output reporter.");
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
  return params;
}

ParsedReporterBase::ParsedReporterBase(const InputParameters & parameters)
  : GeneralReporter(parameters),
    FunctionParserUtils(parameters),
    _use_t(getParam<bool>("use_t")),
    _reporter_symbols(getParam<std::vector<std::string>>("reporter_symbols"))
{
  // build reporters argument
  std::string symbol_str;
  for (const auto i : index_range(_reporter_symbols))
    symbol_str += (i == 0 ? "" : ",") + _reporter_symbols[i];

  // add time if required
  if (_use_t)
    symbol_str += (symbol_str.empty() ? "" : ",") + std::string("t");

  // Create parsed function
  _func_F = std::make_shared<SymFunction>();
  parsedFunctionSetup(_func_F,
                      getParam<std::string>("expression"),
                      symbol_str,
                      getParam<std::vector<std::string>>("constant_names"),
                      getParam<std::vector<std::string>>("constant_expressions"),
                      comm());

  // reserve storage for parameter passing buffer
  _func_params.resize(_reporter_symbols.size() + _use_t);
}
