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

class KokkosHeatConductionDerivativeMaterial : public Moose::Kokkos::Material
{
public:
  static InputParameters validParams();

  KokkosHeatConductionDerivativeMaterial(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const;

private:
  Moose::Kokkos::MaterialProperty<Real> _thermal_conductivity;
  Moose::Kokkos::MaterialProperty<Real> _d_thermal_conductivity_dT;
  Moose::Kokkos::MaterialProperty<Real> _specific_heat;
  Moose::Kokkos::MaterialProperty<Real> _d_specific_heat_dT;
  Moose::Kokkos::MaterialProperty<Real> _density;
  Moose::Kokkos::MaterialProperty<Real> _d_density_dT;

  Moose::Kokkos::VariableValue _temp;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosHeatConductionDerivativeMaterial::computeQpProperties(const unsigned int qp,
                                                            Datum & datum) const
{
  Real T = _temp(datum, qp);

  _thermal_conductivity(datum, qp) = Kokkos::exp(T);
  _d_thermal_conductivity_dT(datum, qp) = Kokkos::exp(T);
  _specific_heat(datum, qp) = T * T * T * T;
  _d_specific_heat_dT(datum, qp) = 4 * T * T * T;
  _density(datum, qp) = T * T * T + 2 / T;
  _d_density_dT(datum, qp) = 3 * T * T - 2 / T / T;
}
