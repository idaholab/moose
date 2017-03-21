/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "FunctionParserUtils.h"

template <>
InputParameters
validParams<FunctionParserUtils>()
{
  InputParameters params = emptyInputParameters();

#ifdef LIBMESH_HAVE_FPARSER_JIT
  params.addParam<bool>(
      "enable_jit",
      true,
      "Enable just-in-time compilation of function expressions for faster evaluation");
  params.addParamNamesToGroup("enable_jit", "Advanced");
#endif
  params.addParam<bool>(
      "enable_ad_cache", true, "Enable cacheing of function derivatives for faster startup time");
  params.addParam<bool>(
      "enable_auto_optimize", true, "Enable automatic immediate optimization of derivatives");
  params.addParam<bool>(
      "disable_fpoptimizer", false, "Disable the function parser algebraic optimizer");
  params.addParam<bool>(
      "fail_on_evalerror",
      false,
      "Fail fatally if a function evaluation returns an error code (otherwise just pass on NaN)");
  params.addParamNamesToGroup("enable_ad_cache", "Advanced");
  params.addParamNamesToGroup("enable_auto_optimize", "Advanced");
  params.addParamNamesToGroup("disable_fpoptimizer", "Advanced");
  params.addParamNamesToGroup("fail_on_evalerror", "Advanced");

  return params;
}

const char * FunctionParserUtils::_eval_error_msg[] = {
    "Unknown",
    "Division by zero",
    "Square root of a negative value",
    "Logarithm of negative value",
    "Trigonometric error (asin or acos of illegal value)",
    "Maximum recursion level reached"};

FunctionParserUtils::FunctionParserUtils(const InputParameters & parameters)
  : _enable_jit(parameters.isParamValid("enable_jit") && parameters.get<bool>("enable_jit")),
    _enable_ad_cache(parameters.get<bool>("enable_ad_cache")),
    _disable_fpoptimizer(parameters.get<bool>("disable_fpoptimizer")),
    _enable_auto_optimize(parameters.get<bool>("enable_auto_optimize") && !_disable_fpoptimizer),
    _fail_on_evalerror(parameters.get<bool>("fail_on_evalerror")),
    _nan(std::numeric_limits<Real>::quiet_NaN())
{
}

void
FunctionParserUtils::setParserFeatureFlags(ADFunctionPtr & parser)
{
  parser->SetADFlags(ADFunction::ADCacheDerivatives, _enable_ad_cache);
  parser->SetADFlags(ADFunction::ADAutoOptimize, _enable_auto_optimize);
}

Real
FunctionParserUtils::evaluate(ADFunctionPtr & parser)
{
  // null pointer is a shortcut for vanishing derivatives, see functionsOptimize()
  if (parser == NULL)
    return 0.0;

  // evaluate expression
  Real result = parser->Eval(&_func_params[0]);

  // fetch fparser evaluation error
  int error_code = parser->EvalError();

  // no error
  if (error_code == 0)
    return result;

  // hard fail or return not a number
  if (_fail_on_evalerror)
    mooseError("DerivativeParsedMaterial function evaluation encountered an error: ",
               _eval_error_msg[(error_code < 0 || error_code > 5) ? 0 : error_code]);

  return _nan;
}

void
FunctionParserUtils::addFParserConstants(ADFunctionPtr & parser,
                                         const std::vector<std::string> & constant_names,
                                         const std::vector<std::string> & constant_expressions)
{
  // check constant vectors
  unsigned int nconst = constant_expressions.size();
  if (nconst != constant_expressions.size())
    mooseError("The parameter vectors constant_names and constant_values must have equal length.");

  // previously evaluated constant_expressions may be used in following constant_expressions
  std::vector<Real> constant_values(nconst);

  for (unsigned int i = 0; i < nconst; ++i)
  {
    ADFunctionPtr expression = ADFunctionPtr(new ADFunction());

    // set FParser internal feature flags
    setParserFeatureFlags(expression);

    // add previously evaluated constants
    for (unsigned int j = 0; j < i; ++j)
      if (!expression->AddConstant(constant_names[j], constant_values[j]))
        mooseError("Invalid constant name in ParsedMaterialHelper");

    // build the temporary comnstant expression function
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
