//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosParsedFunction.h"

#include "FEProblemBase.h"

registerKokkosFunction("MooseApp", KokkosParsedFunction);

InputParameters
KokkosParsedFunction::validParams()
{
  InputParameters params = FunctionBase::validParams();
  params += MooseParsedFunctionBase::validParams();
  params.addCustomTypeParam<std::string>(
      "expression", "FunctionExpression", "The user defined function.");
  params.addParam<bool>(
      "print_rpn", false, "Print parsed Reverse Polish Notation (RPN) for debugging.");

  params.addClassDescription("Function created by parsing a string");

  return params;
}

KokkosParsedFunction::KokkosParsedFunction(const InputParameters & parameters)
  : FunctionBase(parameters),
    MooseParsedFunctionBase(parameters),
    _expression(verifyFunction(getParam<std::string>("expression"))),
    _builder(std::make_unique<Moose::Kokkos::RPNBuilder>(_expression, &_console)),
    _problem(*parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))
{
  _builder->addDefaultVariables();

  std::vector<std::string> symbol_names;

  if (isParamValid("symbol_names"))
    symbol_names = getParam<std::vector<std::string>>("symbol_names");

  for (const auto & name : symbol_names)
    _builder->addVariable(name);

  _builder->build();

  if (getParam<bool>("print_rpn"))
  {
    _console << std::endl
             << "Reverse Polish Notation (RPN) of KokkosParsedFunction " << name() << ":"
             << std::endl;
    _builder->printRPN(_console);
  }
}

KokkosParsedFunction::KokkosParsedFunction(const KokkosParsedFunction & function)
  : FunctionBase(function),
    MooseParsedFunctionBase(function.parameters()),
    _expression(function._expression),
    _evaluator(function._evaluator),
    _problem(function._problem)
{
}

void
KokkosParsedFunction::initialSetup()
{
  std::vector<std::string> symbol_names;
  std::vector<std::string> symbol_values;

  if (isParamValid("symbol_names"))
    symbol_names = getParam<std::vector<std::string>>("symbol_names");
  if (isParamValid("symbol_values"))
    symbol_values = getParam<std::vector<std::string>>("symbol_values");

  _symbol_values.reserve(symbol_values.size());
  _symbol_functions.reserve(symbol_values.size());

  for (const auto i : make_range(symbol_names.size()))
  {
    if (hasPostprocessorByName(symbol_values[i]))
      _builder->associateScalar(symbol_names[i], &getPostprocessorValueByName(symbol_values[i]));
    else if (_problem.hasKokkosFunction(symbol_values[i]))
    {
      _symbol_functions.push_back(_problem.getKokkosFunction(symbol_values[i]));
      _builder->associateFunction(symbol_names[i], &_symbol_functions.back());
    }
    else if (MooseUtils::parsesToReal(symbol_values[i]))
    {
      _symbol_values.push_back(MooseUtils::convert<Real>(symbol_values[i]));
      _builder->associateScalar(symbol_names[i], &_symbol_values.back());
    }
    else
      paramError("symbol_values", "Symbols cannot be scalar variables for now.");
  }

  _builder->finalize();

  // Should be called after finalization
  _evaluator.init(*_builder);
}
