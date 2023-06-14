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
 * AuxKernel that evaluates a parsed function expression for each component of an array variable
 */
class ArrayParsedAux : public ArrayAuxKernel, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  ArrayParsedAux(const InputParameters & parameters);

protected:
  virtual RealEigenVector computeValue() override;

  /// Function expression
  std::string _function;

  /// Number of coupled regular variables
  const unsigned int _n_vars;
  /// Number of coupled array variables
  const unsigned int _n_array_vars;
  /// Regular variable values
  const std::vector<const VariableValue *> _vars;
  /// Array variable values
  const std::vector<const ArrayVariableValue *> _array_vars;

  /// Import coordinates and time
  const bool _use_xyzt;

  /// Function parser object that is evaluated for setting aux-variable values
  SymFunctionPtr _func_F;

  usingFunctionParserUtilsMembers(false);
};
