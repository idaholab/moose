//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedReporterBase.h"

InputParameters
ParsedReporterBase::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params += FunctionParserUtils<false>::validParams();
  params.addRequiredCustomTypeParam<std::string>(
      "expression", "FunctionExpression", "function expression");
  params.addParam<std::string>("name", "result", "Name of output reporter.");
  params.addParam<std::vector<std::string>>(
      "vector_reporter_symbols", {}, "Expression symbol for each reporter");
  params.addParam<std::vector<std::string>>(
      "scalar_reporter_symbols",
      {},
      "Expression symbol for each scalar reporter, i.e. postprocessors");
  params.addParam<std::vector<std::string>>(
      "constant_names",
      {},
      "Vector of constants used in the parsed function (use this for kB etc.)");
  params.addParam<std::vector<ReporterName>>("scalar_reporter_names",
                                             "Scalar reporter names to apply function to.");
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
    _vector_reporter_symbols(getParam<std::vector<std::string>>("vector_reporter_symbols")),
    _scalar_reporter_symbols(getParam<std::vector<std::string>>("scalar_reporter_symbols"))
{
  // checking that symbols and names vectors are the same size
  if (parameters.isParamValid("scalar_reporter_names"))
  {
    // get scalar reporter can be checked and gotten here if input
    // Vector reporters must be handled differently by each derived class
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

  // build reporters argument; order in derived classes must use this order
  // first add vector reporter symbols
  std::string symbol_str;
  for (const auto i : index_range(_vector_reporter_symbols))
    symbol_str += (symbol_str.empty() ? "" : ",") + _vector_reporter_symbols[i];
  // next add scalar reporter symbols
  for (const auto i : index_range(_scalar_reporter_symbols))
    symbol_str += (symbol_str.empty() ? "" : ",") + _scalar_reporter_symbols[i];

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
  _func_params.resize(_vector_reporter_symbols.size() + _scalar_reporter_symbols.size() + _use_t);
}
