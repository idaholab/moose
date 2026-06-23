//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include <array>

/**
 * Advanced hourglass correction for QUAD4 elements that computes the correction
 * for a single displacement component (x or y). It uses the current geometry (via
 * _current_elem) to compute an affine displacement field, isolates the hourglass modes,
 * and scales the penalty dynamically.
 */
class HourglassCorrectionQuad4 : public Kernel
{
public:
  static InputParameters validParams();

  HourglassCorrectionQuad4(const InputParameters & parameters);

  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

protected:
  /// Base penalty parameter (supplied by the user)
  const Real _penalty;

  /// Shear modulus for stabilization scaling (default 1.0)
  const Real _mu;

  /// Displacement variable (applied component-wise: x or y)
  const MooseVariable::DoFValue & _v;

  /// Hourglass mode vectors for a QUAD4 element (unnormalized)
  /// Mode 1: [ 1, -1,  1, -1 ]
  const std::array<Real, 4> _g1;
  /// Mode 2: [ 1,  1, -1, -1 ]
  const std::array<Real, 4> _g2;
};
