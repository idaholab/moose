//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedConvergence.h"
#include "MooseUtils.h"
#include "Function.h"

registerMooseObject("MooseApp", ParsedConvergence);

InputParameters
ParsedConvergence::validParams()
{
  InputParameters params = Convergence::validParams();
  params += FunctionParserUtils<false>::validParams();

  params.addClassDescription("Evaluates convergence from a parsed expression.");

  params.addRequiredCustomTypeParam<std::string>(
      "convergence_expression", "FunctionExpression", "Expression to parse for convergence");
  params.addCustomTypeParam<std::string>(
      "divergence_expression", "FunctionExpression", "Expression to parse for divergence");
  params.addParam<std::vector<std::string>>(
      "symbol_names", {}, "Symbol names to use in the parsed expressions");
  params.addParam<std::vector<std::string>>(
      "symbol_values",
      {},
      "Values (Convergence names, Postprocessor names, Function names, and constants) "
      "corresponding to each entry in 'symbol_names'");

  return params;
}

ParsedConvergence::ParsedConvergence(const InputParameters & parameters)
  : Convergence(parameters),
    FunctionParserUtils<false>(parameters),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _symbol_names(getParam<std::vector<std::string>>("symbol_names")),
    _symbol_values(getParam<std::vector<std::string>>("symbol_values")),
    _convergence_function_params(_symbol_names.size(), 0.0),
    _divergence_function_params(_symbol_names.size(), 0.0)
{
  if (_symbol_names.size() != _symbol_values.size())
    mooseError("The parameters 'symbol_names' and 'symbol_values' must have the same size.");
}

void
ParsedConvergence::initialSetup()
{
  Convergence::initialSetup();

  initializeSymbols();

  const auto convergence_expression = getParam<std::string>("convergence_expression");
  _convergence_function = makeParsedFunction(convergence_expression);

  const auto divergence_expression = isParamValid("divergence_expression")
                                         ? getParam<std::string>("divergence_expression")
                                         : MooseUtils::join(_convergence_symbol_names, "|");
  _divergence_function = makeParsedFunction(divergence_expression);
}

void
ParsedConvergence::initializeSymbols()
{
  for (const auto i : index_range(_symbol_values))
  {
    ReporterName reporter_name(_symbol_values[i], "value");
    if (_fe_problem.getReporterData().hasReporterValue<PostprocessorValue>(reporter_name))
      initializePostprocessorSymbol(i);
    else if (_fe_problem.hasFunction(_symbol_values[i]))
      initializeFunctionSymbol(i);
    else if (_fe_problem.hasConvergence(_symbol_values[i]))
      initializeConvergenceSymbol(i);
    else
      initializeConstantSymbol(i);
  }
}

void
ParsedConvergence::initializePostprocessorSymbol(unsigned int i)
{
  const PostprocessorValue & pp_value = _fe_problem.getPostprocessorValueByName(_symbol_values[i]);
  _pp_values.push_back(&pp_value);
  _pp_indices.push_back(i);
}

void
ParsedConvergence::initializeFunctionSymbol(unsigned int i)
{
  Function & function = _fe_problem.getFunction(_symbol_values[i], _tid);
  _functions.push_back(&function);
  _function_indices.push_back(i);
}

void
ParsedConvergence::initializeConvergenceSymbol(unsigned int i)
{
  Convergence & convergence = _fe_problem.getConvergence(_symbol_values[i], _tid);
  _convergences.push_back(&convergence);
  _convergence_symbol_names.push_back(_symbol_names[i]);
  _convergence_indices.push_back(i);
}

void
ParsedConvergence::initializeConstantSymbol(unsigned int i)
{
  try
  {
    const Real value = MooseUtils::convert<Real>(_symbol_values[i], true);
    _convergence_function_params[i] = value;
    _divergence_function_params[i] = value;
  }
  catch (const std::invalid_argument & e)
  {
    mooseError(
        "The 'symbol_values' entry '",
        _symbol_values[i],
        "' is not a constant value or the name of a Convergence, Postprocessor, or Function.",
        e.what());
  }
}

ParsedConvergence::SymFunctionPtr
ParsedConvergence::makeParsedFunction(const std::string & expression)
{
  auto sym_function = std::make_shared<SymFunction>();

  setParserFeatureFlags(sym_function);

  // Add constants
  sym_function->AddConstant("pi", std::acos(Real(-1)));
  sym_function->AddConstant("e", std::exp(Real(1)));

  // Parse the expression
  const auto symbols_str = Moose::stringify(_symbol_names);
  if (sym_function->Parse(expression, symbols_str) >= 0)
    mooseError("The expression\n  '",
               expression,
               "'\nwith symbols\n  '",
               symbols_str,
               "'\ncould not be parsed:\n",
               sym_function->ErrorMsg());

  // Optimize the parsed function
  functionsOptimize(sym_function);

  return sym_function;
}

Convergence::MooseConvergenceStatus
ParsedConvergence::checkConvergence(unsigned int iter)
{
  updateSymbolValues(iter);

  const Real converged_real = evaluate(_convergence_function, _convergence_function_params, name());
  const Real diverged_real = evaluate(_divergence_function, _divergence_function_params, name());

  if (convertRealToBool(diverged_real, "divergence_expression"))
    return MooseConvergenceStatus::DIVERGED;
  else if (convertRealToBool(converged_real, "convergence_expression"))
    return MooseConvergenceStatus::CONVERGED;
  else
    return MooseConvergenceStatus::ITERATING;
}

void
ParsedConvergence::updateSymbolValues(unsigned int iter)
{
  updatePostprocessorSymbolValues();
  updateFunctionSymbolValues();
  updateConvergenceSymbolValues(iter);
}

void
ParsedConvergence::updatePostprocessorSymbolValues()
{
  for (const auto i : index_range(_pp_indices))
  {
    _convergence_function_params[_pp_indices[i]] = (*_pp_values[i]);
    _divergence_function_params[_pp_indices[i]] = (*_pp_values[i]);
  }
}

void
ParsedConvergence::updateFunctionSymbolValues()
{
  for (const auto i : index_range(_function_indices))
  {
    const Real function_value = _functions[i]->value(_t, Point(0, 0, 0));
    _convergence_function_params[_function_indices[i]] = function_value;
    _divergence_function_params[_function_indices[i]] = function_value;
  }
}

void
ParsedConvergence::updateConvergenceSymbolValues(unsigned int iter)
{
  for (const auto i : index_range(_convergence_indices))
  {
    const auto status = _convergences[i]->checkConvergence(iter);
    _convergence_function_params[_convergence_indices[i]] =
        status == MooseConvergenceStatus::CONVERGED;
    _divergence_function_params[_convergence_indices[i]] =
        status == MooseConvergenceStatus::DIVERGED;
  }
}

bool
ParsedConvergence::convertRealToBool(Real value, const std::string & param) const
{
  if (MooseUtils::absoluteFuzzyEqual(value, 1.0))
    return true;
  else if (MooseUtils::absoluteFuzzyEqual(value, 0.0))
    return false;
  else
    mooseError("The expression parameter '",
               param,
               "' evaluated to the value ",
               value,
               ", but it must only evaluate to either 0 or 1.");
}
