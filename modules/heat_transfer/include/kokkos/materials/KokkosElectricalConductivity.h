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
 * Kokkos material computing the electrical conductivity as a function of temperature, using
 * copper for parameter defaults
 */
class KokkosElectricalConductivity : public Moose::Kokkos::Material
{
public:
  static InputParameters validParams();

  KokkosElectricalConductivity(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const;

private:
  const Real _ref_resis;
  const Real _temp_coeff;
  const Real _ref_temp;
  /// Coupled temperature
  const Moose::Kokkos::VariableValue _T;
  /// Material property base name
  const std::string _base_name;
  Moose::Kokkos::MaterialProperty<Real> _electric_conductivity;
  Moose::Kokkos::MaterialProperty<Real> _delectric_conductivity_dT;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosElectricalConductivity::computeQpProperties(const unsigned int qp, Datum & datum) const
{
  const Real resistivity = _ref_resis * (1.0 + _temp_coeff * (_T(datum, qp) - _ref_temp));
  const Real dresistivity_dT = _ref_resis * _temp_coeff;
  _electric_conductivity(datum, qp) = 1.0 / resistivity;
  _delectric_conductivity_dT(datum, qp) = -1.0 / (resistivity * resistivity) * dresistivity_dT;
}
