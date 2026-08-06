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
 * Hourglass correction for underintegrated QUAD4 elements that computes the correction for a
 * single displacement component. It projects the classical hourglass vector out of the affine
 * displacement space and scales the penalty using the current element geometry.
 */
class HourglassCorrectionQuad4 : public Kernel
{
public:
  static InputParameters validParams();

  HourglassCorrectionQuad4(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// Base penalty parameter (supplied by the user)
  const Real _penalty;

  /// Shear modulus for stabilization scaling (default 1.0)
  const Real _mu;

  /// Displacement variable (applied component-wise: x or y)
  const MooseVariable::DofValues & _v;

  /// Classical QUAD4 hourglass vector [1, -1, 1, -1] in libMesh node ordering
  const std::array<Real, 4> _gamma;
};
