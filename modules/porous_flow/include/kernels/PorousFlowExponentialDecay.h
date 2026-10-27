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

/**
 * Kernel = _rate * (variable - reference)
 *
 * Templated on is_ad: the false instantiation uses the hand-coded Jacobian;
 * the true instantiation propagates the derivative through the AD residual.
 */
template <bool is_ad>
class PorousFlowExponentialDecayTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowExponentialDecayTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// The decay rate
  const GenericVariableValue<is_ad> & _rate;

  /// The reference
  const GenericVariableValue<is_ad> & _reference;

  usingGenericKernelMembers;
};

typedef PorousFlowExponentialDecayTempl<false> PorousFlowExponentialDecay;
typedef PorousFlowExponentialDecayTempl<true> ADPorousFlowExponentialDecay;
