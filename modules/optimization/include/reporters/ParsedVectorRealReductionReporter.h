//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ParsedReporterBase.h"

/**
 * Reporter performing a reduction on a vector using a parsed function
 */
class ParsedVectorRealReductionReporter : public ParsedReporterBase
{
public:
  static InputParameters validParams();

  ParsedVectorRealReductionReporter(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override;

private:
  // initial value of element-wise reduction
  const Real _initial_value;

  /// Vector being operated on
  const std::vector<Real> & _reporter_data;

  /// output containing reduction of vector into a scalar
  Real & _output_reporter;
};
