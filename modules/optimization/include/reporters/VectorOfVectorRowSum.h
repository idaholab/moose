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

/**
 * Reporter containing row sum of a vector of vectors from another Reporter
 */
class VectorOfVectorRowSum : public GeneralReporter
{
public:
  static InputParameters validParams();

  VectorOfVectorRowSum(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override;

private:
  /// Reporter containing vector of vectors row sum
  std::vector<double> & _row_sum;
  /// Reporter name containing vector of vectors
  const ReporterName & _rname;
  /// Reporter data containing vector of vectors
  const std::vector<std::vector<Real>> * _reporter_data;
};
