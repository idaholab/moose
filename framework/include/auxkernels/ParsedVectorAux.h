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
 * AuxKernel that evaluates a parsed function expression for every component
 */
class ParsedVectorAux : public VectorAuxKernel, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  ParsedVectorAux(const InputParameters & parameters);

protected:
  virtual RealVectorValue computeValue() override;

  /// function expressions
  std::vector<std::string> _function;

  /// coupled variables
  /// NOTE: a potential optimization would be to have different number of arguments per component
  const unsigned int _nargs;
  const unsigned int _n_vector_args;
  const std::vector<const VariableValue *> _args;
  std::vector<const VectorVariableValue *> _vector_args;

  /// import coordinates and time
  const bool _use_xyzt;

  /// function parser object
  std::vector<SymFunctionPtr> _func_F;

  usingFunctionParserUtilsMembers(false);
};
