//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"
#include "FunctionParserUtils.h"

/**
 * Reporter performing a reduction on a vector using a parsed function
 */
class VectorSum : public GeneralReporter, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  VectorSum(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override;

private:
  /// whether time is part of the parsed expression
  const bool _use_t;

  // initial value of element-wise reduction
  const Real _initial_value;

  /// Vector being operated on
  const std::vector<Real> & _vector;
  /// output vector
  Real & _output_reporter;
  /// function parser object
  SymFunctionPtr _func_F;

  usingFunctionParserUtilsMembers(false);
};
