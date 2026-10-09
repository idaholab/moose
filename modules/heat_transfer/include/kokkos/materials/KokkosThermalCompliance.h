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
 * Kokkos material computing the thermal compliance used by the SIMP topology optimization method
 */
class KokkosThermalCompliance : public Moose::Kokkos::Material
{
public:
  static InputParameters validParams();

  KokkosThermalCompliance(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const;

private:
  /// Temperature gradient
  const Moose::Kokkos::VariableGradient _grad_temperature;
  /// Thermal conductivity
  const Moose::Kokkos::MaterialProperty<Real> _thermal_conductivity;
  /// Thermal compliance
  Moose::Kokkos::MaterialProperty<Real> _thermal_compliance;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosThermalCompliance::computeQpProperties(const unsigned int qp, Datum & datum) const
{
  const auto grad_T = _grad_temperature(datum, qp);
  _thermal_compliance(datum, qp) = 0.5 * _thermal_conductivity(datum, qp) * (grad_T * grad_T);
}
