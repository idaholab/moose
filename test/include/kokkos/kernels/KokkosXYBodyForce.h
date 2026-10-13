//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosBodyForce.h"

class KokkosXYBodyForce : public KokkosBodyForce
{
public:
  static InputParameters validParams();

  KokkosXYBodyForce(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpResidualValue(const unsigned int qp,
                                                 AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpPositionJacobianValue(const Moose::Kokkos::Real3 & dx,
                                                         const unsigned int qp,
                                                         AssemblyDatum & datum) const;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosXYBodyForce::precomputeQpResidualValue(const unsigned int qp, AssemblyDatum & datum) const
{
  return (datum.q_point(qp)(0) + datum.q_point(qp)(1)) *
         KokkosBodyForce::precomputeQpResidualValue<Derived>(qp, datum);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosXYBodyForce::precomputeQpPositionJacobianValue(const Moose::Kokkos::Real3 & dx,
                                                     const unsigned int qp,
                                                     AssemblyDatum & datum) const
{
  return (dx(0) + dx(1)) * KokkosBodyForce::precomputeQpResidualValue<Derived>(qp, datum);
}
