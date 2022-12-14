//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedOptimizationFunction.h"

// MOOSE includes
#include "MooseUtils.h"

registerMooseObject("OptimizationApp", ParsedOptimizationFunction);

InputParameters
ParsedOptimizationFunction::validParams()
{
  InputParameters params = OptimizationFunction::validParams();
  params.addClassDescription(
      "Function used for optimization that uses a parsed expression with parameter dependence.");

  params.addRequiredCustomTypeParam<std::string>(
      "expression", "FunctionExpression", "The user defined function.");
  params.addRequiredParam<std::vector<std::string>>(
      "param_symbol_names", "Names of parameters in 'expression' being optimized.");
  params.addRequiredParam<ReporterName>(
      "param_vector_name", "Reporter or VectorPostprocessor vector containing parameter values.");
  params.addParam<std::vector<std::string>>(
      "constant_symbol_names",
      std::vector<std::string>(),
      "Variables (excluding t,x,y,z) that are bound to the values provided by the corresponding "
      "items in the symbol_values vector.");
  params.addParam<std::vector<Real>>(
      "constant_symbol_values", std::vector<Real>(), "Constant numeric values for vars.");
  return params;
}

ParsedOptimizationFunction::ParsedOptimizationFunction(const InputParameters & parameters)
  : OptimizationFunction(parameters),
    ReporterInterface(this),
    _expression(getParam<std::string>("expression")),
    _param_names(getParam<std::vector<std::string>>("param_symbol_names")),
    _params(getReporterValue<std::vector<Real>>("param_vector_name")),
    _symbol_names(getParam<std::vector<std::string>>("constant_symbol_names")),
    _symbol_values(getParam<std::vector<Real>>("constant_symbol_values"))
{
  if (_symbol_names.size() != _symbol_values.size())
    paramError("symbol_names", "Number of vars must match the number of vals for a ", type(), "!");

  // Loop through the variables assigned by the user and give an error if x,y,z,t are used
  std::string msg = "The variables \"x, y, z, and t\" in the ParsedFunction are pre-declared for "
                    "use and must not be declared.";
  for (const auto & var : _param_names)
    if (var.find_first_of("xyzt") != std::string::npos && var.size() == 1)
      paramError("param_names", msg);
  for (const auto & var : _symbol_names)
    if (var.find_first_of("xyzt") != std::string::npos && var.size() == 1)
      paramError("symbol_names", msg);

  // Create parser
  _parser = std::make_unique<FunctionParserADBase<Real>>();

  // Add basic and user-defined constants
  _parser->AddConstant("NaN", std::numeric_limits<Real>::quiet_NaN());
  _parser->AddConstant("pi", libMesh::pi);
  _parser->AddConstant("e", std::exp(1.0));
  for (const auto & i : index_range(_symbol_names))
    _parser->AddConstant(_symbol_names[i], _symbol_values[i]);

  // Join xyzt and parameters to give to FParser as variables
  std::vector<std::string> all_vars = {"x", "y", "z", "t"};
  all_vars.insert(all_vars.end(), _param_names.begin(), _param_names.end());

  // Parse expression and error if something goes wrong
  if (_parser->Parse(_expression, MooseUtils::join(all_vars, ",")) != -1)
    paramError("expression", "Unable to parse expression\n", _parser->ErrorMsg());
  _parser->SetADFlags(FunctionParserADBase<Real>::ADAutoOptimize);
  _parser->Optimize();

  // Add parsers for each derivative we are taking, including xyzt
  _derivative_parsers.resize(all_vars.size());
  for (const auto & i : index_range(all_vars))
  {
    _derivative_parsers[i] = std::make_unique<FunctionParserADBase<Real>>(*_parser);
    if (_derivative_parsers[i]->AutoDiff(all_vars[i]) != -1)
      paramError("expression",
                 "Unable to take derivative with respect to ",
                 all_vars[i],
                 "\n",
                 _derivative_parsers[i]->ErrorMsg());
  }
}

Real
ParsedOptimizationFunction::value(Real t, const Point & p) const
{
  return evaluateExpression(*_parser, t, p);
}

RealGradient
ParsedOptimizationFunction::gradient(Real t, const Point & p) const
{
  RealGradient result;
  result(0) = evaluateExpression(*_derivative_parsers[0], t, p, "gradient");
  result(1) = evaluateExpression(*_derivative_parsers[1], t, p, "gradient");
  result(2) = evaluateExpression(*_derivative_parsers[2], t, p, "gradient");
  return result;
}

Real
ParsedOptimizationFunction::timeDerivative(Real t, const Point & p) const
{
  return evaluateExpression(*_derivative_parsers[3], t, p, "time derivative");
}

std::vector<Real>
ParsedOptimizationFunction::parameterGradient(Real t, const Point & p) const
{
  std::vector<Real> result(_param_names.size());
  for (const auto & i : index_range(result))
    result[i] =
        evaluateExpression(*_derivative_parsers[4 + i], t, p, _param_names[i] + " derivative");
  return result;
}

Real
ParsedOptimizationFunction::evaluateExpression(FunctionParserADBase<Real> & parser,
                                               Real t,
                                               const Point & p,
                                               std::string name) const
{
  if (_params.size() != _param_names.size())
    paramError("param_vector_name",
               "Size of vector (",
               _params.size(),
               ") does not match number of specified 'param_names' (",
               _param_names.size(),
               ").");

  std::vector<Real> parser_var_values(4 + _param_names.size());
  parser_var_values[0] = p(0);
  parser_var_values[1] = p(1);
  parser_var_values[2] = p(2);
  parser_var_values[3] = t;
  for (const auto & i : index_range(_param_names))
    parser_var_values[4 + i] = _params[i];

  Real result = parser.Eval(parser_var_values.data());

  auto err = parser.EvalError();
  if (err)
  {
    std::string msg = "Error evaluating function " + name + "\n";
    switch (err)
    {
      case 1:
        msg += "Division by zero";
        break;
      case 2:
        msg += "Square Root error (negative value)";
        break;
      case 3:
        msg += "Log error (negative value)";
        break;
      case 4:
        msg += "Trigonometric error (asin or acos of illegal value)";
        break;
      case 5:
        msg += "Maximum recursion level reached";
        break;
    }

    paramError("expression", msg);
  }

  return result;
}
