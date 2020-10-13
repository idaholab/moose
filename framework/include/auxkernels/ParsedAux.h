//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "FunctionParserUtils.h"

/**
 * AuxKernel that evaluates a parsed function expression
 */
class ParsedAux : public AuxKernel, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  ParsedAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// function expression
  std::string _function;

  /// coupled variables
  const unsigned int _nargs;
  const std::vector<const VariableValue *> _args;

  /// import coordinates and time
  const bool _use_xyzt;

  /// function parser object for the resudual and on-diagonal Jacobian
  SymFunctionPtr _func_F;

  usingFunctionParserUtilsMembers(false);
};
