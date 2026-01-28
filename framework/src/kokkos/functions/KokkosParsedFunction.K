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

  params.addClassDescription("Function created by parsing a string");

  return params;
}

KokkosParsedFunction::KokkosParsedFunction(const InputParameters & parameters)
  : FunctionBase(parameters),
    MooseParsedFunctionBase(parameters),
    _expression(verifyFunction(getParam<std::string>("expression"))),
    _builder(std::make_unique<Moose::Kokkos::RPNBuilder>(_expression))
{
}

KokkosParsedFunction::KokkosParsedFunction(const KokkosParsedFunction & function)
  : FunctionBase(function),
    MooseParsedFunctionBase(function.parameters()),
    _expression(function._expression)
{
}

void
KokkosParsedFunction::initialSetup()
{
  _builder->build(_builder->getAST());

  _console << std::endl
           << "Abstract Syntax Tree (AST) of KokkosParsedFunction " << name() << ":" << std::endl
           << std::endl;
  _builder->printAST(_console, _builder->getAST());

  _console << std::endl
           << "Reverse Polish Notation (RPN) of KokkosParsedFunction " << name() << ":" << std::endl
           << std::endl;
  _builder->printRPN(_console);
}
