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

class RhieChowMassFlux;

/**
 * Pressure gradient method that uses Rhie-Chow face fluxes to reconstruct a cell-centered pressure
 * gradient for the momentum predictor.
 */
class FVReconstructedPressureGradient : public FVGradientMethod
{
public:
  /// Input parameters for the reconstructed pressure-gradient method.
  static InputParameters validParams();

  /**
   * @param params Input parameters used to construct the reconstructed pressure-gradient method.
   */
  FVReconstructedPressureGradient(const InputParameters & params);

  void setupDependencies(SystemBase & system, unsigned int variable_number) const override;

private:
  /**
   * Compute reconstructed pressure gradients before the base class applies any limiter.
   *
   * @param system Pressure system that owns the pressure variable and gradient storage.
   * @param output_gradient Component vectors where pre-limiter gradients are written.
   * @param scratch_gradient Temporary component vectors available during the computation.
   * @param variable_numbers Pressure variable numbers whose gradients should be updated.
   */
  void computeGradientWithoutLimiter(
      SystemBase & system,
      GradientContainer & output_gradient,
      GradientContainer & scratch_gradient,
      const std::unordered_set<unsigned int> & variable_numbers) const override;

  /**
   * Resolve the method used before reconstructed Rhie-Chow data are available.
   *
   * @param system Pressure system used to access registered gradient methods.
   */
  const FVGradientMethod & resolveBaseGradientMethod(SystemBase & system) const;

  /// Rhie-Chow user object that supplies HbyA, Ainv, and reconstructed cell velocities.
  const UserObjectName _rhie_chow_user_object_name;

  /// Gradient method used before reconstructed Rhie-Chow data are available.
  const GradientMethodName _base_gradient_method_name;

  /// Cached gradient method used before reconstructed Rhie-Chow data are available.
  mutable const FVGradientMethod * _base_gradient_method = nullptr;

  /// Cached Rhie-Chow user object that owns the reconstructed gradients.
  mutable const RhieChowMassFlux * _rhie_chow_user_object = nullptr;
};
