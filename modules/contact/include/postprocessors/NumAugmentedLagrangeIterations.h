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

class AugmentedLagrangianContactProblemInterface;

/**
 * Get the number of extra augmented Lagrange loops around the non-linear solve. This PP will return
 * zero if the current problem is not an augmented Lagrangian problem, and for all time steps that
 * converged on the first iteration.
 */
class NumAugmentedLagrangeIterations : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  NumAugmentedLagrangeIterations(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}

  using Postprocessor::getValue;
  virtual PostprocessorValue getValue() const override;

  // only needed for ElementPostprocessors and NodalPostprocessors
  virtual void threadJoin(const UserObject &) override {}

protected:
  /// Augmented Lagrange problem
  AugmentedLagrangianContactProblemInterface * const _augmented_lagrange_problem;
};
