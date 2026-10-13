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
 * Kokkos material computing the thermal compliance sensitivity used by the SIMP topology
 * optimization method
 */
class KokkosThermalSensitivity : public DerivativeMaterialInterface<Moose::Kokkos::Material>
{
public:
  static InputParameters validParams();

  KokkosThermalSensitivity(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const;

private:
  /// Material property base name
  const std::string _base_name;
  /// Thermal compliance sensitivity with respect to the design density
  Moose::Kokkos::MaterialProperty<Real> _sensitivity;
  /// Temperature gradient
  const Moose::Kokkos::VariableGradient _grad_temperature;
  /// Thermal conductivity
  const Moose::Kokkos::MaterialProperty<Real> _thermal_conductivity;
  /// Derivative of the thermal conductivity with respect to the design density
  const Moose::Kokkos::MaterialProperty<Real> _dTdp;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosThermalSensitivity::computeQpProperties(const unsigned int qp, Datum & datum) const
{
  const auto grad_T = _grad_temperature(datum, qp);
  const Real k = _thermal_conductivity(datum, qp);
  const Real thermal_compliance = 0.5 * k * (grad_T * grad_T);
  _sensitivity(datum, qp) = -_dTdp(datum, qp) * thermal_compliance / k;
}
