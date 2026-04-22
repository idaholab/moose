//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosMaterial.h"

/**
 * Simple material with properties set as constants.
 */
class KokkosHeatConductionMaterial : public Moose::Kokkos::Material
{
public:
  static InputParameters validParams();

  KokkosHeatConductionMaterial(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const;

private:
  const Real _my_thermal_conductivity;
  const Real _my_specific_heat;

  Moose::Kokkos::MaterialProperty<Real> _thermal_conductivity;
  Moose::Kokkos::MaterialProperty<Real> _specific_heat;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosHeatConductionMaterial::computeQpProperties(const unsigned int qp, Datum & datum) const
{
  _thermal_conductivity(datum, qp) = _my_thermal_conductivity;
  _specific_heat(datum, qp) = _my_specific_heat;
}
