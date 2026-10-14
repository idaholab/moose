//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosADMaterial.h"

/**
 * Test AD material with a property depending on a coupled variable and on the position on the
 * configuration of the material:
 *
 *   k = k0 + a T^2 + g . x
 */
class KokkosADQuadraticMaterial : public Moose::Kokkos::ADMaterial
{
public:
  /// T
  static constexpr unsigned int max_ad_slots = 1;

  static InputParameters validParams();

  KokkosADQuadraticMaterial(const InputParameters & parameters);

  template <typename Derived, unsigned int slots>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp,
                                           Moose::Kokkos::ADMaterialDatum<slots> & datum) const
  {
    const auto T = _T(datum, qp);
    const auto k = _k0 + _a * T * T + _g * datum.adQPoint(qp);
    _k(datum, qp) = Moose::Kokkos::denseADValue(k);
  }

private:
  /// Coupled variable
  const Moose::Kokkos::ADVariableValue _T;
  /// Constant term
  const Real _k0;
  /// Coefficient of the quadratic term in T
  const Real _a;
  /// Coefficient vector of the position
  const Moose::Kokkos::Real3 _g;
  /// The property
  Moose::Kokkos::MaterialProperty<Real> _k;
};
