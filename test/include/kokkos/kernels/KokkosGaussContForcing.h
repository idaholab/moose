//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernel.h"

class KokkosGaussContForcing : public Moose::Kokkos::Kernel
{
public:
  static InputParameters validParams();

  KokkosGaussContForcing(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;

protected:
  const Real _amplitude;
  const Real _x_center;
  const Real _y_center;
  const Real _z_center;

  const Real _x_spread;
  const Real _y_spread;
  const Real _z_spread;

  const Real _x_min;
  const Real _x_max;
  const Real _y_min;
  const Real _y_max;
  const Real _z_min;
  const Real _z_max;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosGaussContForcing::computeQpResidual(const unsigned int i,
                                          const unsigned int qp,
                                          AssemblyDatum & datum) const
{
  Real x = datum.q_point(qp)(0);
  Real y = datum.q_point(qp)(1);
  Real z = datum.q_point(qp)(2);

  if (x >= _x_min && x <= _x_max && y >= _y_min && y <= _y_max && z >= _z_min && z <= _z_max)
    return -_test(datum, i, qp) * _amplitude *
           Kokkos::exp(-(((x - _x_center) * (x - _x_center)) / (2.0 * _x_spread * _x_spread) +
                         ((y - _y_center) * (y - _y_center)) / (2.0 * _y_spread * _y_spread) +
                         ((z - _z_center) * (z - _z_center)) / (2.0 * _z_spread * _z_spread)));
  else
    return 0;
}
