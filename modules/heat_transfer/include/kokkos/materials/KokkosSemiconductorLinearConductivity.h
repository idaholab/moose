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
#include "DerivativeMaterialInterface.h"

/**
 * Kokkos material computing the electrical conductivity of a semiconductor from temperature
 * using the Steinhart-Hart equation
 */
class KokkosSemiconductorLinearConductivity
  : public DerivativeMaterialInterface<Moose::Kokkos::Material>
{
public:
  static InputParameters validParams();

  KokkosSemiconductorLinearConductivity(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const;

private:
  const Real _sh_coeff_A;
  const Real _sh_coeff_B;
  /// Coupled temperature
  const Moose::Kokkos::VariableValue _T;
  /// Material property base name
  const std::string _base_name;
  Moose::Kokkos::MaterialProperty<Real> _electric_conductivity;
  /// Derivative of the electrical conductivity, declared only for a non-constant temperature
  Moose::Kokkos::MaterialProperty<Real> _delectric_conductivity_dT;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosSemiconductorLinearConductivity::computeQpProperties(const unsigned int qp,
                                                           Datum & datum) const
{
  const Real T = _T(datum, qp);
  KOKKOS_ASSERT(T > 0);

  const Real sigma = ::Kokkos::exp((_sh_coeff_A - 1 / T) / _sh_coeff_B);
  _electric_conductivity(datum, qp) = sigma;

  if (_delectric_conductivity_dT)
    _delectric_conductivity_dT(datum, qp) = sigma / (_sh_coeff_B * T * T);
}
