//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKineticDisPreRateAux.h"

/**
 * Kokkos concentration of a secondary kinetic species
 */
class KokkosKineticDisPreConcAux : public KokkosKineticDisPreRateAux
{
public:
  static InputParameters validParams();

  KokkosKineticDisPreConcAux(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, AssemblyDatum & datum) const;

private:
  /// Old value of the species concentration
  const Moose::Kokkos::VariableValue _u_old;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosKineticDisPreConcAux::computeValue(const unsigned int qp, AssemblyDatum & datum) const
{
  const Real u_new_aux = _u_old(datum, qp) + kineticRate(qp, datum) * _dt;

  // Bound concentration for the dissolution case
  return u_new_aux < 0.0 ? 0.0 : u_new_aux;
}
