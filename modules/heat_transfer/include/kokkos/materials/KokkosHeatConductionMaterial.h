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
class KokkosHeatConductionMaterial final
  : public Moose::Kokkos::Material<KokkosHeatConductionMaterial>
{
public:
  static InputParameters validParams();

  KokkosHeatConductionMaterial(const InputParameters & parameters);

  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const
  {
    _thermal_conductivity(datum, qp) = _my_thermal_conductivity;
    _thermal_conductivity_dT(datum, qp) = 0;
    _specific_heat(datum, qp) = _my_specific_heat;
    _specific_heat_dT(datum, qp) = 0;
  }

private:
  const bool _has_temp;
  const Moose::Kokkos::VariableValue _temperature;

  const Real _my_thermal_conductivity;
  const Real _my_specific_heat;

  Moose::Kokkos::MaterialProperty<Real> _thermal_conductivity;
  Moose::Kokkos::MaterialProperty<Real> _thermal_conductivity_dT;

  Moose::Kokkos::MaterialProperty<Real> _specific_heat;
  Moose::Kokkos::MaterialProperty<Real> _specific_heat_dT;

  /// Minimum temperature, below which temperature is "clipped" before evaluating functions
  const Real _min_T;
};
