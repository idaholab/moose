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
 * Pressure gradient method that uses Rhie-Chow reconstructed gradients when available.
 */
class FVReconstructedPressureGradient : public FVGradientMethod
{
public:
  static InputParameters validParams();

  FVReconstructedPressureGradient(const InputParameters & params);

private:
  void computeGradientWithoutLimiter(
      SystemBase & system,
      GradientContainer & output_gradient,
      GradientContainer & scratch_gradient,
      const std::unordered_set<unsigned int> & variable_numbers) const override;

  const FVGradientMethod & baseGradientMethod(SystemBase & system) const;

  const UserObjectName _rhie_chow_user_object_name;
  const GradientMethodName _base_gradient_method_name;
};
