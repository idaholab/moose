//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericKernel.h"

enum class DisplacementRegularizationType
{
  Huhu,
  Lulu,
  HuhuLulu
};

namespace DisplacementRegularizationTools
{
MooseEnum regularizationType();
DisplacementRegularizationType regularizationType(const MooseEnum & regularization);
Real defaultLuluFactor(const unsigned int dim);

template <bool is_ad, typename Variable>
const GenericVariableSecond<is_ad> &
secondSln(Variable & var)
{
  if constexpr (is_ad)
    return var.adSecondSln();
  else
    return var.secondSln();
}

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
               const DisplacementRegularizationType type,
               const Real lulu_factor)
{
  switch (type)
  {
    case DisplacementRegularizationType::Huhu:
      return huhu(second_u, second_test, dim);
    case DisplacementRegularizationType::Lulu:
      return lulu(second_u, second_test, dim);
    case DisplacementRegularizationType::HuhuLulu:
      // HuHu-LuLu keeps the Hessian contraction and subtracts a scaled Laplacian contraction.
      return huhu(second_u, second_test, dim) - lulu_factor * lulu(second_u, second_test, dim);
  }

  mooseError("Unknown displacement regularization type.");
}
}

/**
 * Adds displacement regularization terms based on Hessian and Laplacian contractions of one scalar
 * displacement component.
 *
 * Vector-valued mechanics variables are regularized component-wise by adding one kernel per
 * displacement component.
 */
template <bool is_ad>
class DisplacementRegularizationTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();

  DisplacementRegularizationTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;

  /// Selected regularization contraction.
  const DisplacementRegularizationType _regularization_type;
  /// User coefficient multiplying the selected regularization term.
  const Real _coefficient;
  /// Mesh dimension used in Hessian and Laplacian contractions.
  const unsigned int _dim;
  /// LuLu correction factor used only by HuHu-LuLu.
  const Real _lulu_factor;
  /// Second derivatives of the nonlinear variable.
  const GenericVariableSecond<is_ad> & _second_u;
  /// Second derivatives of test functions.
  const VariableTestSecond & _second_test;

  usingGenericKernelMembers;
};

class DisplacementRegularization : public DisplacementRegularizationTempl<false>
{
public:
  static InputParameters validParams();

  DisplacementRegularization(const InputParameters & parameters);

protected:
  virtual Real computeQpJacobian() override;

  /// Second derivatives of trial functions.
  const VariablePhiSecond & _second_phi;
};

typedef DisplacementRegularizationTempl<true> ADDisplacementRegularization;
