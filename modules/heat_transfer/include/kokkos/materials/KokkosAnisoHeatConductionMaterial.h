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
#include "DerivativeMaterialInterface.h"

/**
 * General-purpose Kokkos material model for anisotropic heat conduction
 */
class KokkosAnisoHeatConductionMaterial
  : public DerivativeMaterialInterface<Moose::Kokkos::Material>
{
  using Real33 = Moose::Kokkos::Real33;

public:
  static InputParameters validParams();

  KokkosAnisoHeatConductionMaterial(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void initQpStatefulProperties(const unsigned int qp, Datum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const;

private:
  /// Reference temperature for the thermal conductivity
  const Real _ref_temp;
  /// Coupled temperature
  const Moose::Kokkos::VariableValue _T;
  /// Material property base name
  const std::string _base_name;
  /// User-provided thermal conductivity tensor
  const Real33 _user_provided_thermal_conductivity;
  Moose::Kokkos::MaterialProperty<Real33> _thermal_conductivity;
  Moose::Kokkos::MaterialProperty<Real33> _dthermal_conductivity_dT;
  /// Temperature coefficient of the thermal conductivity as a function of temperature
  const Moose::Kokkos::Function _thermal_conductivity_temperature_coefficient_function;
  Moose::Kokkos::MaterialProperty<Real> _specific_heat;
  Moose::Kokkos::MaterialProperty<Real> _dspecific_heat_dT;
  /// Specific heat as a function of temperature
  const Moose::Kokkos::Function _specific_heat_function;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosAnisoHeatConductionMaterial::computeQpProperties(const unsigned int qp, Datum & datum) const
{
  const Real temp_qp = _T(datum, qp);
  const auto p = datum.q_point(qp);

  if (_thermal_conductivity_temperature_coefficient_function)
  {
    _thermal_conductivity(datum, qp) =
        _user_provided_thermal_conductivity *
        (1.0 + _thermal_conductivity_temperature_coefficient_function.value(temp_qp, p) *
                   (temp_qp - _ref_temp));
    _dthermal_conductivity_dT(datum, qp) =
        _user_provided_thermal_conductivity *
        _thermal_conductivity_temperature_coefficient_function.timeDerivative(temp_qp, p) *
        (temp_qp - _ref_temp);
  }
  else
  {
    _thermal_conductivity(datum, qp) = _user_provided_thermal_conductivity;
    _dthermal_conductivity_dT(datum, qp) = 0;
  }

  _specific_heat(datum, qp) = _specific_heat_function.value(temp_qp, p);
  _dspecific_heat_dT(datum, qp) = _specific_heat_function.timeDerivative(temp_qp, p);
}

template <typename Derived>
KOKKOS_FUNCTION void
KokkosAnisoHeatConductionMaterial::initQpStatefulProperties(const unsigned int qp,
                                                            Datum & datum) const
{
  _thermal_conductivity(datum, qp) = _user_provided_thermal_conductivity;
}
