//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosAuxKernel.h"

/**
 * Kokkos auxiliary kernel computing the heat generated from Joule heating
 */
class KokkosJouleHeatingHeatGeneratedAux : public Moose::Kokkos::AuxKernel
{
public:
  static InputParameters validParams();

  KokkosJouleHeatingHeatGeneratedAux(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, AssemblyDatum & datum) const;

private:
  /// Joule heating material property
  const Moose::Kokkos::MaterialProperty<Real> _heating_residual;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosJouleHeatingHeatGeneratedAux::computeValue(const unsigned int qp, AssemblyDatum & datum) const
{
  return _heating_residual(datum, qp);
}
