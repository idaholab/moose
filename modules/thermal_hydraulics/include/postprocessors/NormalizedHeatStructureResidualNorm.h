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
 * Computes a normalized residual norm for a heat structure.
 */
class NormalizedHeatStructureResidualNorm : public DiscreteVariableResidualNorm
{
public:
  static InputParameters validParams();

  NormalizedHeatStructureResidualNorm(const InputParameters & parameters);

  virtual void initialize() override;
  virtual Real getValue() const override;

protected:
  /// Computes (but does not update) the normalization constant
  Real computeNormalization() const;

  /// Reference element size
  const PostprocessorValue & _ref_elem_size;

  /// Normalization constant
  Real _normalization;
  /// Initialized
  bool _initialized;
};
