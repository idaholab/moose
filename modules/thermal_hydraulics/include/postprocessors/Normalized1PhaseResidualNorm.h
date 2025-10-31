//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DiscreteVariableResidualNorm.h"

/**
 * Computes a normalized residual norm for the single-phase flow model.
 */
class Normalized1PhaseResidualNorm : public DiscreteVariableResidualNorm
{
public:
  static InputParameters validParams();

  Normalized1PhaseResidualNorm(const InputParameters & parameters);

  virtual Real getValue() const override;

protected:
  /// Computes the normalization constant
  Real computeNormalization() const;

  /// Normalization constant
  const Real _normalization;
};
