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
 * Hourglass correction for underintegrated HEX8 elements: removes the
 * least-squares affine part of the elemental displacement and penalizes the
 * four Flanagan-Belytschko hourglass modes with a rotation-invariant scale.
 * 3D companion of HourglassCorrectionQuad4.
 */
class HourglassCorrectionHex8 : public Kernel
{
public:
  static InputParameters validParams();

  HourglassCorrectionHex8(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// Base penalty parameter
  const Real _penalty;

  /// Shear modulus used in the stabilization scaling
  const Real _mu;

  /// Nodal values of the active displacement component
  const MooseVariable::DofValues & _v;

  /// Flanagan-Belytschko hourglass base vectors in libMesh HEX8 node ordering
  const std::array<std::array<Real, 8>, 4> _gamma;
};
