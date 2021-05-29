//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"
#include "FunctionParserUtils.h"

/**
 * Postprocessor that evaluates a parsed function expression
 */
class ParsedPostprocessor : public GeneralPostprocessor, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  ParsedPostprocessor(const InputParameters & parameters);

  void initialize() override final;
  void execute() override final;
  PostprocessorValue getValue() override final;

private:
  /// number of postprocessors in parsed expression
  const unsigned int _n_pp;

  /// values of the postprocessors part of the parsed expression
  std::vector<const PostprocessorValue *> _pp_values;

  /// whether time is part of the parsed expression
  const bool _use_t;

  /// function parser object for the resudual and on-diagonal Jacobian
  SymFunctionPtr _func_F;

  usingFunctionParserUtilsMembers(false);
};
