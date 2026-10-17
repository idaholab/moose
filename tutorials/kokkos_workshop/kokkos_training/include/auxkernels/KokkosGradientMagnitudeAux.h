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
 * Magnitude of the gradient of a coupled variable, for elemental auxiliary variables
 */
class KokkosGradientMagnitudeAux : public Moose::Kokkos::AuxKernel
{
public:
  static InputParameters validParams();

  KokkosGradientMagnitudeAux(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, AssemblyDatum & datum) const;

private:
  /// Gradient of the coupled variable
  const Moose::Kokkos::VariableGradient _grad_v;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosGradientMagnitudeAux::computeValue(const unsigned int qp, AssemblyDatum & datum) const
{
  return _grad_v(datum, qp).norm();
}
