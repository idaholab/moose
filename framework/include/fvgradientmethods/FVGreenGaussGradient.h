//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVGradientMethod.h"

/**
 * Green-Gauss cell-centered gradient method for linear finite-volume variables.
 */
class FVGreenGaussGradient : public FVGradientMethod
{
public:
  /// Input parameters for the Green-Gauss gradient method.
  static InputParameters validParams();

  /**
   * @param params Input parameters used to construct the Green-Gauss gradient method.
   */
  FVGreenGaussGradient(const InputParameters & params);

protected:
  /**
   * Compute Green-Gauss gradients before the base class applies any limiter.
   *
   * @param system System that owns the variables and gradient storage.
   * @param output_gradient Component vectors where pre-limiter gradients are written.
   * @param scratch_gradient Temporary component vectors available during the computation.
   * @param variable_numbers Variable numbers whose gradients should be updated.
   */
  void computeGradientWithoutLimiter(
      SystemBase & system,
      GradientContainer & output_gradient,
      GradientContainer & scratch_gradient,
      const std::unordered_set<unsigned int> & variable_numbers) const override;
};
