//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedPostprocessor.h"

registerMooseObject("MooseApp", ParsedPostprocessor);

InputParameters
ParsedPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params += FunctionParserUtils<false>::validParams();

  params.addRequiredCustomTypeParam<std::string>(
      "function", "FunctionExpression", "function expression");
  params.deprecateParam("function", "expression", "05/01/2025");

  params.addParam<std::vector<PostprocessorName>>("pp_names", {}, "Post-processors arguments");
  params.addParam<std::vector<std::string>>(
      "pp_symbols", {}, "Symbol associated with each post-processor argument");
  params.addParam<std::vector<std::string>>(
      "constant_names",
      {},
      "Vector of constants used in the parsed function (use this for kB etc.)");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      {},
      "Vector of values for the constants in constant_names (can be an FParser expression)");
  params.addParam<bool>(
      "use_t", false, "Make time (t) variable available in the function expression.");

  params.addClassDescription("Computes a parsed expression with post-processors");
  return params;
}

ParsedPostprocessor::ParsedPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    FunctionParserUtils(parameters),
    _n_pp(coupledPostprocessors("pp_names")),
    _use_t(getParam<bool>("use_t")),
    _value(0.0)
{
  // build postprocessors argument
  std::string postprocessors;

  const std::vector<std::string> pp_symbols = getParam<std::vector<std::string>>("pp_symbols");
  // sanity checks
  if (!pp_symbols.empty() && (pp_symbols.size() != _n_pp))
    paramError("pp_symbols", "pp_symbols must be the same length as pp_names.");

  // coupled  postprocessors with capacity for symbol inputs
  std::vector<PostprocessorName> pp_names = getParam<std::vector<PostprocessorName>>("pp_names");
  if (pp_symbols.empty())
  {
    for (std::size_t i = 0; i < _n_pp; ++i)
      postprocessors += (i == 0 ? "" : ",") + pp_names[i];
  }
  else
    postprocessors = MooseUtils::stringJoin(pp_symbols, ",");

  // add time if required
  if (_use_t)
    postprocessors += (postprocessors.empty() ? "" : ",") + std::string("t");

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
  if (_func_F->Parse(function, postprocessors) >= 0)
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
  _func_params.resize(_n_pp + _use_t);
  _pp_values.resize(_n_pp);
  for (unsigned int i = 0; i < _n_pp; i++)
    _pp_values[i] = &getPostprocessorValue("pp_names", i);
}

void
ParsedPostprocessor::initialize()
{
}

void
ParsedPostprocessor::execute()
{
}

void
ParsedPostprocessor::finalize()
{
  for (unsigned int i = 0; i < _n_pp; i++)
    _func_params[i] = *_pp_values[i];

  if (_use_t)
    _func_params[_n_pp] = _t;

  _value = evaluate(_func_F);
}

PostprocessorValue
ParsedPostprocessor::getValue() const
{
  return _value;
}
