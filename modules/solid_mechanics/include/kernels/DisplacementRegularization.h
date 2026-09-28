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
  /// Mesh dimension used for the default LuLu factor and the 1D HuHu-LuLu guard.
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

using ADDisplacementRegularization = DisplacementRegularizationTempl<true>;
