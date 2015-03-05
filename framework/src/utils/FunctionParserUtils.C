#include "FunctionParserUtils.h"

template<>
InputParameters validParams<FunctionParserUtils>()
{
  InputParameters params = emptyInputParameters();

#ifdef LIBMESH_HAVE_FPARSER_JIT
  params.addParam<bool>("enable_jit", true, "enable just-in-time compilation of function expressions for faster evaluation");
#endif
  params.addParam<bool>( "disable_fpoptimizer", false, "Disable the function parser algebraic optimizer");
  params.addParam<bool>( "fail_on_evalerror", false, "Fail fatally if a function evaluation returns an error code (otherwise just pass on NaN)");

  return params;
}

const char * FunctionParserUtils::_eval_error_msg[] = {
  "Unknown",
  "Division by zero",
  "Square root of a negative value",
  "Logarithm of negative value",
  "Trigonometric error (asin or acos of illegal value)",
  "Maximum recursion level reached"
};

FunctionParserUtils::FunctionParserUtils(const std::string & /* name */,
                                          InputParameters parameters) :
    _enable_jit(parameters.isParamValid("enable_jit") &&
                parameters.get<bool>("enable_jit")),
    _disable_fpoptimizer(parameters.get<bool>("disable_fpoptimizer")),
    _fail_on_evalerror(parameters.get<bool>("fail_on_evalerror")),
    _nan(std::numeric_limits<Real>::quiet_NaN())
{
}

Real
FunctionParserUtils::evaluate(ADFunction * parser)
{
  // null pointer is a shortcut for vanishing derivatives, see functionsOptimize()
  if (parser == NULL) return 0.0;

  // evaluate expression
  Real result = parser->Eval(_func_params);

  // fetch fparser evaluation error
  int error_code = parser->EvalError();

  // no error
  if (error_code == 0)
    return result;

  // hard fail or return not a number
  if (_fail_on_evalerror)
    mooseError("DerivativeParsedMaterial function evaluation encountered an error: "
               << _eval_error_msg[(error_code < 0 || error_code > 5) ? 0 : error_code]);

  return _nan;
}
