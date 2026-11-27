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

class EigenProblem;

/**
 * Reports the B-norm sqrt(phi^T B phi) of the current solution phi of the nonlinear eigen system,
 * which the EigenProblem computes with the B operator of the solve after each solve and each time
 * it loads an eigenvector. Being homogeneous of degree one in phi, it is usable as the
 * 'normalization' postprocessor of an eigenvalue executioner.
 */
class EigenvectorBNorm : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  EigenvectorBNorm(const InputParameters & parameters);

  void initialize() override {}
  void execute() override {}

  /**
   * @return The B-norm of the current eigenvector, zero before the first solve
   */
  Real getValue() const override;

private:
  /// Eigen problem computing and storing the B-norm of the eigenvector in the solution
  const EigenProblem * const _eigen_problem;
};
