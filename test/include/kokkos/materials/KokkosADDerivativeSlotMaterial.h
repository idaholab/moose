//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosMaterial.h"

/**
 * Test material copying one stored derivative of a pointwise AD property into a property
 */
class KokkosADDerivativeSlotMaterial : public Moose::Kokkos::Material
{
public:
  static InputParameters validParams();

  KokkosADDerivativeSlotMaterial(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const
  {
    _derivative(datum, qp) = _derivatives(datum, qp)(_slot);
  }

private:
  /// Stored derivatives of the AD property
  const Moose::Kokkos::MaterialProperty<Real, 1> _derivatives;
  /// Slot of the copied derivative
  const unsigned int _slot;
  /// The copied derivative
  Moose::Kokkos::MaterialProperty<Real> _derivative;
};
