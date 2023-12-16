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
 * Reporter containing operation between vectors from another Reporter
 */
class ParsedVectors : public GeneralReporter, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  ParsedVectors(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override;

private:
  /// whether time is part of the parsed expression
  const bool _use_t;

  /// output vector
  std::vector<double> & _output_reporter;

  /// function parser object
  SymFunctionPtr _func_F;

  std::vector<const std::vector<Real> *> _reporter_data;

  usingFunctionParserUtilsMembers(false);
};
