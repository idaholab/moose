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
 * M-gradient kernel for the first and second set of grains
 */
class EpsilonModelMGradientKernel : public ADKernel
{
public:
  static InputParameters validParams();

  EpsilonModelMGradientKernel(const InputParameters & parameters);

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

  /// Positive derivatives of m
  const ADMaterialProperty<RealGradient> & _dm_plus;

  /// Negative derivatives of m
  const ADMaterialProperty<RealGradient> & _dm_minus;

  /// Material property L
  const ADMaterialProperty<Real> & _L;

  /// Free energy function
  const ADMaterialProperty<Real> & _F;
};
