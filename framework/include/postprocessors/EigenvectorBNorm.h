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

#include "libmesh/numeric_vector.h"

#include <memory>

class NonlinearEigenSystem;

/**
 * Computes the B-norm sqrt(phi^T B phi) of the current solution phi of the nonlinear eigen system,
 * where B is the operator the eigen solver solved with. Being homogeneous of degree one in phi, it
 * is usable as the 'normalization' postprocessor of an eigenvalue executioner.
 */
class EigenvectorBNorm : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  EigenvectorBNorm(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;

  /**
   * @return The B-norm of the current eigenvector
   */
  virtual Real getValue() const override;

protected:
  /// Nonlinear eigen system holding the eigenvector and the operators of the eigen solver
  NonlinearEigenSystem * const _nl_eigen;

  /// The quadratic form phi^T B phi of the current eigenvector, computed by execute()
  Real _b_norm_squared;

  /// Work vector holding B * phi, allocated on first use and reused by every execution
  std::unique_ptr<NumericVector<Number>> _b_phi;
};
