//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

enum class FERegularizationType
{
  Huhu,
  Lulu,
  HuhuLulu
};

namespace FERegularizationTools
{
MooseEnum regularizationType();
FERegularizationType regularizationType(const MooseEnum & regularization);
Real defaultLuluFactor(const unsigned int dim);

/// Hessian-Hessian contraction: \sum_{j,k} u_{,jk} v_{,jk}.
template <typename SolutionSecond, typename TestSecond>
auto
huhu(const SolutionSecond & second_u, const TestSecond & second_test, const unsigned int dim)
{
  decltype(second_u(0, 0) * second_test(0, 0)) value = 0;
  for (const auto j : make_range(dim))
    for (const auto k : make_range(dim))
      value += second_u(j, k) * second_test(j, k);
  return value;
}

/// Laplacian-Laplacian contraction: (\sum_j u_{,jj})(\sum_k v_{,kk}).
template <typename SolutionSecond, typename TestSecond>
auto
lulu(const SolutionSecond & second_u, const TestSecond & second_test, const unsigned int dim)
{
  decltype(second_u(0, 0) * second_test(0, 0)) laplacian_u = 0;
  Real laplacian_test = 0;
  for (const auto j : make_range(dim))
  {
    laplacian_u += second_u(j, j);
    laplacian_test += second_test(j, j);
  }
  return laplacian_u * laplacian_test;
}

/// Shared weak-form contraction used by both the manual and AD kernels.
template <typename SolutionSecond, typename TestSecond>
auto
regularization(const SolutionSecond & second_u,
               const TestSecond & second_test,
               const unsigned int dim,
               const FERegularizationType type,
               const Real lulu_factor)
{
  switch (type)
  {
    case FERegularizationType::Huhu:
      return huhu(second_u, second_test, dim);
    case FERegularizationType::Lulu:
      return lulu(second_u, second_test, dim);
    case FERegularizationType::HuhuLulu:
      // HuHu-LuLu keeps the Hessian contraction and subtracts a scaled Laplacian contraction.
      return huhu(second_u, second_test, dim) - lulu_factor * lulu(second_u, second_test, dim);
  }

  mooseError("Unknown FE regularization type.");
}
}

/**
 * Adds FE regularization terms based on Hessian and Laplacian contractions of one scalar field.
 *
 * Vector-valued mechanics variables are regularized component-wise by adding one kernel per
 * displacement component.
 */
class FERegularization : public Kernel
{
public:
  static InputParameters validParams();

  FERegularization(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// Selected regularization contraction.
  const FERegularizationType _regularization_type;
  /// User coefficient multiplying the selected regularization term.
  const Real _coefficient;
  /// Mesh dimension used in Hessian and Laplacian contractions.
  const unsigned int _dim;
  /// LuLu correction factor used only by HuHu-LuLu.
  const Real _lulu_factor;
  /// Second derivatives of the nonlinear variable.
  const VariableSecond & _second_u;
  /// Second derivatives of trial functions.
  const VariablePhiSecond & _second_phi;
  /// Second derivatives of test functions.
  const VariableTestSecond & _second_test;
};
