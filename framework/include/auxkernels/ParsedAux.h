//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PARSEDAUX_H
#define PARSEDAUX_H

#include "AuxKernel.h"
#include "FunctionParserUtils.h"

// Forward Declarations
class ParsedAux;

template <>
InputParameters validParams<ParsedAux>();

/**
 * AuxKernel that evaluates a parsed function expression
 */
class ParsedAux : public AuxKernel, public FunctionParserUtils
{
public:
  ParsedAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// function expression
  std::string _function;

  /// coupled variables
  unsigned int _nargs;
  std::vector<const VariableValue *> _args;

  /// function parser object for the resudual and on-diagonal Jacobian
  ADFunctionPtr _func_F;
};

#endif /* PARSEDAUX_H */
