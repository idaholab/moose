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
 * Reporter performing a reduction on a vector
 * currently only implements sum
 */
class VectorSum : public GeneralReporter
{
public:
  static InputParameters validParams();

  VectorSum(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override;

private:
  /// scaling factor for dot product
  const Real & _scale;
  /// Reporter containing vector reduction
  Real & _sum;

  /// Vector being operated on
  const std::vector<Real> & _vector;
};
