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

enum class SecondDerivativeRegularizationType
{
  Huhu,
  Lulu,
  HuhuLulu
};

/**
 * Adds regularization terms based on Hessian and Laplacian contractions of a scalar nonlinear
 * variable.
 */
template <bool is_ad>
class SecondDerivativeRegularizationTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();

  SecondDerivativeRegularizationTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;

  /// Selected regularization contraction.
  const SecondDerivativeRegularizationType _regularization_type;
  /// User coefficient multiplying the selected regularization term.
  const Real _coefficient;
  /// Mesh dimension used for the default LuLu factor.
  const unsigned int _dim;
  /// LuLu correction factor used only by HuHu-LuLu.
  const Real _lulu_factor;
  /// Second derivatives of the nonlinear variable.
  const GenericVariableSecond<is_ad> & _second_u;
  /// Second derivatives of test functions.
  const VariableTestSecond & _second_test;
  /// Second derivatives of trial functions used by the hand-coded Jacobian.
  const VariablePhiSecond & _second_phi;

  usingGenericKernelMembers;
};

class SecondDerivativeRegularization : public SecondDerivativeRegularizationTempl<false>
{
public:
  using SecondDerivativeRegularizationTempl<false>::SecondDerivativeRegularizationTempl;

protected:
  virtual Real computeQpJacobian() override;
};

using ADSecondDerivativeRegularization = SecondDerivativeRegularizationTempl<true>;
