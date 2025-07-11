//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionParserUtils.h"

// MOOSE includes
#include "InputParameters.h"
#include "MooseEnum.h"

template <bool is_ad>
InputParameters
FunctionParserUtils<is_ad>::validParams()
{
  InputParameters params = emptyInputParameters();

  params.addParam<bool>(
      "enable_jit",
#ifdef LIBMESH_HAVE_FPARSER_JIT
      true,
#else
      false,
#endif
      "Enable just-in-time compilation of function expressions for faster evaluation");
  params.addParam<bool>(
      "enable_ad_cache", true, "Enable caching of function derivatives for faster startup time");
  params.addParam<bool>(
      "enable_auto_optimize", true, "Enable automatic immediate optimization of derivatives");
  params.addParam<bool>(
      "disable_fpoptimizer", false, "Disable the function parser algebraic optimizer");
  MooseEnum evalerror("nan nan_warning error exception", "nan");
  params.addParam<MooseEnum>("evalerror_behavior",
                             evalerror,
                             "What to do if evaluation error occurs. Options are to pass a nan, "
                             "pass a nan with a warning, throw a error, or throw an exception");

  params.addParamNamesToGroup(
      "enable_jit enable_ad_cache enable_auto_optimize disable_fpoptimizer evalerror_behavior",
      "Parsed expression advanced");
  params.addParam<Real>("epsilon", 0, "Fuzzy comparison tolerance");
  return params;
}

template <bool is_ad>
const char * FunctionParserUtils<is_ad>::_eval_error_msg[] = {
    "Unknown",
    "Division by zero",
    "Square root of a negative value",
    "Logarithm of negative value",
    "Trigonometric error (asin or acos of illegal value)",
    "Maximum recursion level reached"};

template <bool is_ad>
FunctionParserUtils<is_ad>::FunctionParserUtils(const InputParameters & parameters)
  : _enable_jit(parameters.isParamValid("enable_jit") && parameters.get<bool>("enable_jit")),
    _enable_ad_cache(parameters.get<bool>("enable_ad_cache")),
    _disable_fpoptimizer(parameters.get<bool>("disable_fpoptimizer")),
    _enable_auto_optimize(parameters.get<bool>("enable_auto_optimize") && !_disable_fpoptimizer),
    _evalerror_behavior(parameters.get<MooseEnum>("evalerror_behavior").getEnum<FailureMethod>()),
    _quiet_nan(std::numeric_limits<Real>::quiet_NaN()),
    _epsilon(parameters.get<Real>("epsilon"))
{
#ifndef LIBMESH_HAVE_FPARSER_JIT
  if (_enable_jit)
  {
    mooseWarning("Tried to enable FParser JIT but libmesh does not have it compiled in.");
    _enable_jit = false;
  }
#endif
}

template <bool is_ad>
void
FunctionParserUtils<is_ad>::setParserFeatureFlags(SymFunctionPtr & parser) const
{
  parser->SetADFlags(SymFunction::ADCacheDerivatives, _enable_ad_cache);
  parser->SetADFlags(SymFunction::ADAutoOptimize, _enable_auto_optimize);
}

template <bool is_ad>
GenericReal<is_ad>
FunctionParserUtils<is_ad>::evaluate(SymFunctionPtr & parser, const std::string & name)
{
  return evaluate(parser, _func_params, name);
}

template <bool is_ad>
GenericReal<is_ad>
FunctionParserUtils<is_ad>::evaluate(SymFunctionPtr & parser,
                                     const std::vector<GenericReal<is_ad>> & func_params,
                                     const std::string & name)
{
  // null pointer is a shortcut for vanishing derivatives, see functionsOptimize()
  if (parser == NULL)
    return 0.0;

  // set desired epsilon
  auto tmp_eps = parser->epsilon();
  parser->setEpsilon(_epsilon);

  // evaluate expression
  auto result = parser->Eval(func_params.data());

  // restore epsilon
  parser->setEpsilon(tmp_eps);

  // fetch fparser evaluation error (set to unknown if the JIT result is nan)
  int error_code = _enable_jit ? (std::isnan(result) ? -1 : 0) : parser->EvalError();

  // no error
  if (error_code == 0)
    return result;

  // hard fail or return not a number
  switch (_evalerror_behavior)
  {
    case FailureMethod::nan:
      return _quiet_nan;

    case FailureMethod::nan_warning:
      mooseWarning("In ",
                   name,
                   ": Parsed function evaluation encountered an error: ",
                   _eval_error_msg[(error_code < 0 || error_code > 5) ? 0 : error_code]);
      return _quiet_nan;

    case FailureMethod::error:
      mooseError("In ",
                 name,
                 ": Parsed function evaluation encountered an error: ",
                 _eval_error_msg[(error_code < 0 || error_code > 5) ? 0 : error_code]);

    case FailureMethod::exception:
      mooseException("In ",
                     name,
                     ": Parsed function evaluation encountered an error: ",
                     _eval_error_msg[(error_code < 0 || error_code > 5) ? 0 : error_code],
                     "\n Cutting timestep");
  }

  return _quiet_nan;
}

template <bool is_ad>
void
FunctionParserUtils<is_ad>::addFParserConstants(
    SymFunctionPtr & parser,
    const std::vector<std::string> & constant_names,
    const std::vector<std::string> & constant_expressions) const
{
  // check constant vectors
  unsigned int nconst = constant_expressions.size();
  if (nconst != constant_names.size())
    mooseError("The parameter vectors constant_names (size " +
               std::to_string(constant_names.size()) + ") and constant_expressions (size " +
               std::to_string(nconst) + ") must have equal length.");

  // previously evaluated constant_expressions may be used in following constant_expressions
  std::vector<Real> constant_values(nconst);

  for (unsigned int i = 0; i < nconst; ++i)
  {
    // no need to use dual numbers for the constant expressions
    auto expression = std::make_shared<FunctionParserADBase<Real>>();

    // add previously evaluated constants
    for (unsigned int j = 0; j < i; ++j)
      if (!expression->AddConstant(constant_names[j], constant_values[j]))
        mooseError("Invalid constant name: ", constant_names[j], " and value ", constant_values[j]);

    // build the temporary constant expression function
    if (expression->Parse(constant_expressions[i], "") >= 0)
      mooseError("Invalid constant expression\n",
                 constant_expressions[i],
                 "\n in parsed function object.\n",
                 expression->ErrorMsg());

    constant_values[i] = expression->Eval(NULL);

    if (!parser->AddConstant(constant_names[i], constant_values[i]))
      mooseError("Invalid constant name in parsed function object");
  }
}

template <>
void
FunctionParserUtils<false>::functionsOptimize(SymFunctionPtr & parsed_function)
{
  // set desired epsilon for optimization!
  auto tmp_eps = parsed_function->epsilon();
  parsed_function->setEpsilon(_epsilon);

  // base function
  if (!_disable_fpoptimizer)
    parsed_function->Optimize();
  if (_enable_jit && !parsed_function->JITCompile())
    mooseInfo("Failed to JIT compile expression, falling back to byte code interpretation.");

  parsed_function->setEpsilon(tmp_eps);
}

template <>
void
FunctionParserUtils<true>::functionsOptimize(SymFunctionPtr & parsed_function)
{
  // set desired epsilon for optimization!
  auto tmp_eps = parsed_function->epsilon();
  parsed_function->setEpsilon(_epsilon);

  // base function
  if (!_disable_fpoptimizer)
    parsed_function->Optimize();
  if (!_enable_jit || !parsed_function->JITCompile())
    mooseError("AD parsed objects require JIT compilation to be enabled and working.");

  parsed_function->setEpsilon(tmp_eps);
}

template <bool is_ad>
void
FunctionParserUtils<is_ad>::parsedFunctionSetup(
    SymFunctionPtr & function,
    const std::string & expression,
    const std::string & variables,
    const std::vector<std::string> & constant_names,
    const std::vector<std::string> & constant_expressions,
    const libMesh::Parallel::Communicator & comm) const
{
  // set FParser internal feature flags
  setParserFeatureFlags(function);

  // add the constant expressions
  addFParserConstants(function, constant_names, constant_expressions);

  // parse function
  if (function->Parse(expression, variables) >= 0)
    mooseError("Invalid function\n", expression, "\nError:\n", function->ErrorMsg());

  // optimize
  if (!_disable_fpoptimizer)
    function->Optimize();

  // just-in-time compile
  if (_enable_jit)
  {
    // let rank 0 do the JIT compilation first
    if (comm.rank() != 0)
      comm.barrier();

    function->JITCompile();

    // wait for ranks > 0 to catch up
    if (comm.rank() == 0)
      comm.barrier();
  }
}

// explicit instantiation
template class FunctionParserUtils<false>;
template class FunctionParserUtils<true>;
