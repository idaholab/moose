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
 * Gamma-gradient kernel for the first and second set of grains
 */
class GammaModelGammaGradientKernel : public ADKernel
{
public:
  static InputParameters validParams();

  GammaModelGammaGradientKernel(const InputParameters & parameters);

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

  /// Positive derivatives of gamma
  const ADMaterialProperty<RealGradient> & _dgamma_plus;

  /// Negative derivatives of gamma
  const ADMaterialProperty<RealGradient> & _dgamma_minus;

  /// Material property L
  const ADMaterialProperty<Real> & _L;

  /// Material property m
  const ADMaterialProperty<Real> & _mu;

  /// Number of coupled order parameter variables
  const unsigned int _op_num;

  /// Coupled order parameter variables values
  const std::vector<const ADVariableValue *> _vals;
};
