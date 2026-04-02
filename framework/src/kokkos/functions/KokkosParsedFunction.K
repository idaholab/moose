//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosParsedFunction.h"

registerKokkosFunction("MooseApp", KokkosParsedFunction);

InputParameters
KokkosParsedFunction::validParams()
{
  InputParameters params = FunctionBase::validParams();
  params += MooseParsedFunctionBase::validParams();
  params.addCustomTypeParam<std::string>(
      "expression", "FunctionExpression", "The user defined function.");
  params.addParam<bool>("print_rpn", "Print parsed Reverse Polish Notation (RPN) for debugging.");

  params.addClassDescription("Function created by parsing a string");

  return params;
}

KokkosParsedFunction::KokkosParsedFunction(const InputParameters & parameters)
  : FunctionBase(parameters),
    MooseParsedFunctionBase(parameters),
    _expression(verifyFunction(getParam<std::string>("expression"))),
    _builder(std::make_unique<Moose::Kokkos::RPNBuilder>(_expression, &_console))
{
  _builder->addDefaultVariables();

  const auto & symbol_names = getParam<std::vector<std::string>>("symbol_names");

  for (const auto i : make_range(symbol_names.size()))
    _builder->addVariable(symbol_names[i]);

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
    _evaluator(function._evaluator)
{
}

void
KokkosParsedFunction::initialSetup()
{
  const auto & symbol_names = getParam<std::vector<std::string>>("symbol_names");
  const auto & symbol_values = getParam<std::vector<std::string>>("symbol_values");

  for (const auto i : make_range(symbol_names.size()))
  {
    if (hasPostprocessorByName(symbol_values[i]))
      _builder->associateValue(symbol_names[i], &getPostprocessorValueByName(symbol_values[i]));
    else if (MooseUtils::parsesToReal(symbol_values[i]))
    {
      _symbol_values.push_back(MooseUtils::convert<Real>(symbol_values[i]));
      _builder->associateValue(symbol_names[i], &_symbol_values.back());
    }
    else
      paramError("symbol_values", "Symbols can only be constants or postprocessors for now.");
  }

  _builder->finalize();

  // Should be called after finalization
  _evaluator.init(*_builder);

  // Dry-run for checking whether pre-allocated stack size is sufficient
  _evaluator.eval(0, Real3(0));
}
