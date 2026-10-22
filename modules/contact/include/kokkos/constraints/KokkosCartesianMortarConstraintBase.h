//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosMortarConstraint.h"

/**
 * The base of Kokkos mortar contact constraints with a Cartesian Lagrange multiplier: one
 * first-order Lagrange variable per coordinate direction, the variable of the constraint along x
 * and lm_y and lm_z along y and z, which together form the contact traction vector at each
 * secondary node
 */
class KokkosCartesianMortarConstraintBase : public Moose::Kokkos::MortarConstraintBase
{
public:
  static InputParameters validParams();

  KokkosCartesianMortarConstraintBase(const InputParameters & parameters);
  KokkosCartesianMortarConstraintBase(const KokkosCartesianMortarConstraintBase & object);

protected:
  /// The number of components of the multiplier, the mesh dimension
  const unsigned int _dim;
  /// The components of the multiplier
  Moose::Kokkos::Variable _lm[3];
};
