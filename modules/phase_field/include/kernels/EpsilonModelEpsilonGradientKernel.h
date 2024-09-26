//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

/**
 * Epsilon-gradient kernel for the first and second set of grains
 */

class EpsilonModelEpsilonGradientKernel : public ADKernel
{
public:
  static InputParameters validParams();

  EpsilonModelEpsilonGradientKernel(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  /// Type of sets
  enum class SetsType
  {
    FIRST,
    SECOND
  };

  /// Choose between the first and second set of grain
  const SetsType _grains_set;

  /// Positive derivatives of epsilon
  const ADMaterialProperty<RealGradient> & _depsilon_plus;

  /// Negative derivatives of epsilon
  const ADMaterialProperty<RealGradient> & _depsilon_minus;

  /// Material property L
  const ADMaterialProperty<Real> & _L;

  /// Number of coupled order parameter variables
  const unsigned int _op_num;

  /// Coupled order parameter variables gradients
  const std::vector<const ADVariableGradient *> _grad_vals;
};
