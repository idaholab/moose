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
 * Reporter containing row sum of a vector of vectors from another Reporter
 */
class ParsedVectorVectorRealReductionReporter : public ParsedReporterBase
{
public:
  static InputParameters validParams();

  ParsedVectorVectorRealReductionReporter(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override;

private:
  // initial value of element-wise reduction
  const Real _initial_value;

  // reporter vector of vector w/ data
  const ReporterName _vec_of_vec_name;

  /// output containing reduction of vector of vector into a vector
  std::vector<Real> & _output_reporter;
};
