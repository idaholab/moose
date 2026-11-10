//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosNodalBC.h"

/**
 * A spatially-constant, time-varying NodalBC whose imposed value g(t)
 * varies sinusoidally with time according to the formula:
 *
 * g(t) = { g0 + (gT - g0) * sin ((pi*t) / (2*T)), 0 < t < T
 *        { gT                                   , t > T
 * where:
 * g0 = value at time 0
 * gT = value at time T
 *  T = duration over which the value is changing.
 */
class KokkosSinDirichletBC : public Moose::Kokkos::NodalBC
{
public:
  static InputParameters validParams();

  KokkosSinDirichletBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;

private:
  const Real _initial;
  const Real _final;
  const Real _duration;
  /// Angular frequency pi / (2 T), precomputed on host
  const Real _omega;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosSinDirichletBC::computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  Real value;
  if (_t < _duration)
    value = _initial + (_final - _initial) * ::Kokkos::sin(_omega * _t);
  else
    value = _final;

  return _u(datum, qp) - value;
}
