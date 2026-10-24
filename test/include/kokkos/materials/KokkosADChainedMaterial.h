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
 * Test AD material computing a property from a pointwise AD property p of another material and a
 * coupled variable v:
 *
 *   q = p^2 + c v + g . x
 *
 * The declaration of q optionally lists its inputs, as p alone or as p and v.
 */
class KokkosADChainedMaterial : public Moose::Kokkos::ADMaterial
{
public:
  /// v, and the inputs of p: two coupled field values besides v
  static constexpr unsigned int max_ad_slots = 3;

  static InputParameters validParams();

  KokkosADChainedMaterial(const InputParameters & parameters);

  template <typename Derived, unsigned int slots>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp,
                                           Moose::Kokkos::ADMaterialDatum<slots> & datum) const
  {
    const auto p = _p(datum, qp);
    _q(datum, qp) = p * p + _c * _v(datum, qp) + _g * datum.adQPoint(qp);
  }

private:
  /// The property of another material
  const Moose::Kokkos::ADMaterialPropertyReader _p;
  /// Coupled variable
  const Moose::Kokkos::ADVariableValue _v;
  /// Coefficient of v
  const Real _c;
  /// Coefficient vector of the position
  const Moose::Kokkos::Real3 _g;
  /// The property
  Moose::Kokkos::ADMaterialProperty<Real> _q;

  /**
   * Get the inputs listed in the declaration of the property
   */
  std::vector<Moose::Kokkos::ADPropertyInput> listedInputs() const;
};
