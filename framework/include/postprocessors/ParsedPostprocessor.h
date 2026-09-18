//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

  virtual void initialize() override final;
  virtual void execute() override final;
  virtual void finalize() override final;
  virtual PostprocessorValue getValue() const override final;

private:
  /// number of postprocessors in parsed expression
  const unsigned int _n_pp;

  /// values of the postprocessors part of the parsed expression
  std::vector<const PostprocessorValue *> _pp_values;

  /// saved pointer to the expression to save on get_param calls
  const std::string * _pexp;

  /// previous expression saved as a string for recognizing controller intervention
  std::string _oldexp;

  /// saved pointer to constant_names to save on get_param calls
  const std::vector<std::string> * _pcnames;

  /// saved pointer to constant_expressions to save on get_param calls
  const std::vector<std::string> * _pcexps;

  /// whether time is part of the parsed expression
  const bool _use_t;

  /// function parser object for the resudual and on-diagonal Jacobian
  SymFunctionPtr _func_F;

  /// This post-processor value
  Real _value;

  /// Postprocessors argument for JIT compile
  std::string _postprocessors;

  using Moose::FunctorBase<Real>::evaluate;
  usingFunctionParserUtilsMembers(false);
};
