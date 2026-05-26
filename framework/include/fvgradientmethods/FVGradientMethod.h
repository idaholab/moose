//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "MooseTypes.h"
#include "GradientLimiterType.h"

#include <memory>
#include <unordered_set>
#include <vector>

class SystemBase;

namespace libMesh
{
template <typename T>
class NumericVector;
}

/**
 * Base class for linear finite-volume cell-gradient methods.
 * These methods compute the gradient values that kernels and boundary conditions read from the
 * owning system. Derived classes provide the pre-limiter gradient formula, and this base class
 * applies the selected limiter before the values are published.
 */
class FVGradientMethod : public MooseObject
{
public:
  /// One vector per spatial component of the cell-centered gradient.
  using GradientContainer = std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>>;

  /// Input parameters shared by all the gradient methods.
  static InputParameters validParams();

  /**
   * Constructor
   * @param params Input parameters used to construct the gradient method.
   */
  FVGradientMethod(const InputParameters & params);

  /**
   * Compute the final gradient values for the requested variables.
   *
   * @param system System that owns the variables and gradient storage.
   * @param output_gradient Component vectors where final gradients are written.
   * @param scratch_gradient Temporary component vectors available during the computation.
   * @param variable_numbers Variable numbers whose gradients should be updated.
   */
  void computeGradient(SystemBase & system,
                       GradientContainer & output_gradient,
                       GradientContainer & scratch_gradient,
                       const std::unordered_set<unsigned int> & variable_numbers) const;

  /// Limiter selected for this method.
  Moose::FV::GradientLimiterType limiterType() const { return _limiter_type; }

protected:
  /**
   * Compute the method-specific gradient before this base class applies any limiter.
   *
   * @param system System that owns the variables and gradient storage.
   * @param output_gradient Component vectors where pre-limiter gradients are written.
   * @param scratch_gradient Temporary component vectors available during the computation.
   * @param variable_numbers Variable numbers whose gradients should be updated.
   */
  virtual void computeGradientWithoutLimiter(
      SystemBase & system,
      GradientContainer & output_gradient,
      GradientContainer & scratch_gradient,
      const std::unordered_set<unsigned int> & variable_numbers) const = 0;

private:
  /// Limiter applied after method-specific pre-limiter gradient computation.
  const Moose::FV::GradientLimiterType _limiter_type;
};
