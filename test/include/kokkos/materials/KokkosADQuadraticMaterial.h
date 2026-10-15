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
 *   k = k0 + a T^2 + b S + g . x
 *
 * The declaration of k optionally lists its inputs, which may omit S or list it ahead of T.
 */
class KokkosADQuadraticMaterial : public Moose::Kokkos::ADMaterial
{
public:
  /// T and S
  static constexpr unsigned int max_ad_slots = 2;

  static InputParameters validParams();

  KokkosADQuadraticMaterial(const InputParameters & parameters);

  template <typename Derived, unsigned int slots>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp,
                                           Moose::Kokkos::ADMaterialDatum<slots> & datum) const
  {
    const auto T = _T(datum, qp);

    if (_plain_q_point)
    {
      const auto x = datum.q_point(qp);
      _k(datum, qp) = _k0 + _a * T * T + _b * _S(datum, qp) + _g * x;
    }
    else
      _k(datum, qp) = _k0 + _a * T * T + _b * _S(datum, qp) + _g * datum.adQPoint(qp);
  }

private:
  /// Coupled variables
  ///@{
  const Moose::Kokkos::ADVariableValue _T;
  const Moose::Kokkos::ADVariableValue _S;
  ///@}
  /// Constant term
  const Real _k0;
  /// Coefficient of the quadratic term in T
  const Real _a;
  /// Coefficient of S
  const Real _b;
  /// Coefficient vector of the position
  const Moose::Kokkos::Real3 _g;
  /// Whether to read the position as plain reals, which aborts on the current configuration
  const bool _plain_q_point;
  /// The property
  Moose::Kokkos::ADMaterialProperty<Real> _k;

  /**
   * Get the inputs listed in the declaration of the property
   */
  std::vector<Moose::Kokkos::ADPropertyInput> listedInputs() const;
};
