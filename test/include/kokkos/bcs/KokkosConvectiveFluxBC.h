//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosIntegratedBC.h"

class KokkosConvectiveFluxBC final : public Moose::Kokkos::IntegratedBC<KokkosConvectiveFluxBC>
{
public:
  static InputParameters validParams();

  KokkosConvectiveFluxBC(const InputParameters & parameters);

  KOKKOS_FUNCTION inline Real
  computeQpResidual(const unsigned int i, const unsigned int qp, ResidualDatum & datum) const;
  KOKKOS_FUNCTION inline Real computeQpJacobian(const unsigned int i,
                                                const unsigned int j,
                                                const unsigned int qp,
                                                ResidualDatum & datum) const;

private:
  const Real _initial;
  const Real _final;
  const Real _rate;
  const Real _duration;
};

KOKKOS_FUNCTION inline Real
KokkosConvectiveFluxBC::computeQpResidual(const unsigned int i,
                                          const unsigned int qp,
                                          ResidualDatum & datum) const
{
  Real value;

  if (_t < _duration)
    value = _initial + (_final - _initial) * ::Kokkos::sin((0.5 / _duration) * libMesh::pi * _t);
  else
    value = _final;

  return -(_test(datum, i, qp) * _rate * (value - _u(datum, qp)));
}

KOKKOS_FUNCTION inline Real
KokkosConvectiveFluxBC::computeQpJacobian(const unsigned int i,
                                          const unsigned int j,
                                          const unsigned int qp,
                                          ResidualDatum & datum) const
{
  return -(_test(datum, i, qp) * _rate * (-_phi(datum, j, qp)));
}
