//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"
#include "FERegularization.h"

/**
 * AD version of FERegularization.
 *
 * This class intentionally reuses FERegularizationTools so the AD and manual kernels assemble the
 * same weak form and differ only in how their Jacobian information is obtained.
 */
class ADFERegularization : public ADKernel
{
public:
  static InputParameters validParams();

  ADFERegularization(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// Selected regularization contraction.
  const FERegularizationType _regularization_type;
  /// User coefficient multiplying the selected regularization term.
  const Real _coefficient;
  /// Mesh dimension used in Hessian and Laplacian contractions.
  const unsigned int _dim;
  /// LuLu correction factor used only by HuHu-LuLu.
  const Real _lulu_factor;
  /// AD second derivatives of the nonlinear variable.
  const ADVariableSecond & _second_u;
  /// Non-AD second derivatives of test functions.
  const VariableTestSecond & _second_test;
};
