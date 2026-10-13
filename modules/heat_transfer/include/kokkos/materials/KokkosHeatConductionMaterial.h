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
#include "KokkosFunction.h"

/**
 * General-purpose material model for heat conduction with constant properties or properties
 * given as functions of temperature
 */
class KokkosHeatConductionMaterial : public Moose::Kokkos::Material
{
public:
  static InputParameters validParams();

  KokkosHeatConductionMaterial(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const;

private:
  /// Whether temperature is coupled
  const bool _has_temp;
  /// Coupled temperature
  const Moose::Kokkos::VariableValue _temperature;

  const Real _my_thermal_conductivity;
  const Real _my_specific_heat;

  Moose::Kokkos::MaterialProperty<Real> _thermal_conductivity;
  Moose::Kokkos::MaterialProperty<Real> _thermal_conductivity_dT;
  /// Thermal conductivity as a function of temperature
  const Moose::Kokkos::Function _thermal_conductivity_temperature_function;

  Moose::Kokkos::MaterialProperty<Real> _specific_heat;
  /// Specific heat as a function of temperature
  const Moose::Kokkos::Function _specific_heat_temperature_function;
  Moose::Kokkos::MaterialProperty<Real> _specific_heat_dT;

  /// Whether a minimum temperature for evaluating the property functions is specified
  const bool _has_min_T;
  /// Minimum temperature for evaluating the property functions
  const Real _min_T;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosHeatConductionMaterial::computeQpProperties(const unsigned int qp, Datum & datum) const
{
  Real qp_temperature = _has_temp ? _temperature(datum, qp) : 0;

  if (_has_temp && _has_min_T && qp_temperature < _min_T)
    qp_temperature = _min_T;

  const Moose::Kokkos::Real3 origin(0);

  if (_thermal_conductivity_temperature_function)
  {
    _thermal_conductivity(datum, qp) =
        _thermal_conductivity_temperature_function.value(qp_temperature, origin);
    _thermal_conductivity_dT(datum, qp) =
        _thermal_conductivity_temperature_function.timeDerivative(qp_temperature, origin);
  }
  else
  {
    _thermal_conductivity(datum, qp) = _my_thermal_conductivity;
    _thermal_conductivity_dT(datum, qp) = 0;
  }

  if (_specific_heat_temperature_function)
  {
    _specific_heat(datum, qp) = _specific_heat_temperature_function.value(qp_temperature, origin);
    _specific_heat_dT(datum, qp) =
        _specific_heat_temperature_function.timeDerivative(qp_temperature, origin);
  }
  else
  {
    _specific_heat(datum, qp) = _my_specific_heat;
    _specific_heat_dT(datum, qp) = 0;
  }
}
