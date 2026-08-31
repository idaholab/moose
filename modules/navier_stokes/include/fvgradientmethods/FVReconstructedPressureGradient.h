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

#include "libmesh/numeric_vector.h"

#include <cstdint>

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

  /// Resolve dependencies required by the reconstructed pressure-gradient method.
  void setupDependencies(SystemBase & system, unsigned int variable_number) const;

  /// Name of the gradient method used before reconstructed Rhie-Chow data are available.
  const GradientMethodName & baseGradientMethodName() const { return _base_gradient_method_name; }

  /// Relaxation factor applied to reconstructed gradients.
  Real gradientRelaxation() const { return _gradient_relaxation; }

  /// True once a reconstructed-feedback field has been initialized.
  bool hasFeedback() const { return _feedback_initialized; }

  /// Monotonically increasing feedback generation index.
  std::uint64_t generation() const { return _feedback_generation; }

  /// Update the stored feedback field from a newly reconstructed gradient candidate.
  void updateFeedbackGradient(const GradientContainer & base_gradient,
                              const GradientContainer & reconstructed_candidate) const;

private:
  /**
   * Compute reconstructed pressure gradients before the base class applies any limiter.
   * When a relaxed reconstructed gradient is available, this method copies it into the
   * output gradient container; otherwise it delegates to the configured base method.
   *
   * @param system Pressure system that owns the pressure variable and gradient storage.
   * @param gradient Component vectors where pre-limiter gradients are written.
   * @param variable_numbers Pressure variable numbers whose gradients should be updated.
   */
  virtual void computeGradientWithoutLimiter(
      SystemBase & system,
      GradientContainer & gradient,
      const std::unordered_set<unsigned int> & variable_numbers) const override;

  /**
   * Resolve the method used before reconstructed Rhie-Chow data are available.
   *
   * @param system Pressure system used to access registered gradient methods.
   */
  const FVGradientMethod & resolveBaseGradientMethod(SystemBase & system) const;

  /// Gradient method used before reconstructed Rhie-Chow data are available.
  const GradientMethodName _base_gradient_method_name;

  /// Relaxation factor used when updating reconstructed pressure gradients in the solve.
  const Real _gradient_relaxation;

  /// Cached gradient method used before reconstructed Rhie-Chow data are available.
  mutable const FVGradientMethod * _base_gradient_method = nullptr;

  /// Persistent relaxed feedback field published by this method.
  mutable GradientContainer _feedback;

  /// Whether the feedback field has been initialized.
  mutable bool _feedback_initialized = false;

  /// Feedback generation index incremented on every feedback update.
  mutable std::uint64_t _feedback_generation = 0;
};
