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

class OptimizationTestFunction : public GeneralPostprocessor
{
public:
  static InputParameters validParams();
  OptimizationTestFunction(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void execute() override {}

  PostprocessorValue getValue() const override final;

  /// https://en.wikipedia.org/wiki/Test_functions_for_optimization
  static Real rosen(const std::vector<Real> & x);
  /// https://en.wikipedia.org/wiki/Test_functions_for_optimization
  static Real eggholder(const std::vector<Real> & x);

protected:
  /// Function to evaluate
  const MooseEnum & _function;
  // Input values
  const std::vector<Real> & _x;
};
