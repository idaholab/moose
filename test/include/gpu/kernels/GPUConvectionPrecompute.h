//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUKernelValue.h"

using Real3 = Moose::Kokkos::Real3;

class KokkosConvectionPrecompute final
  : public Moose::Kokkos::KernelValue<KokkosConvectionPrecompute>
{
public:
  static InputParameters validParams();

  KokkosConvectionPrecompute(const InputParameters & parameters);

  KOKKOS_FUNCTION inline Real precomputeQpResidual(const unsigned int qp,
                                                   ResidualDatum & datum) const;
  KOKKOS_FUNCTION inline Real
  precomputeQpJacobian(const unsigned int j, const unsigned int qp, ResidualDatum & datum) const;

private:
  const Real3 _velocity;
};

KOKKOS_FUNCTION inline Real
KokkosConvectionPrecompute::precomputeQpResidual(const unsigned int qp, ResidualDatum & datum) const
{
  return _velocity * _grad_u(datum, qp);
}

KOKKOS_FUNCTION inline Real
KokkosConvectionPrecompute::precomputeQpJacobian(const unsigned int j,
                                                 const unsigned int qp,
                                                 ResidualDatum & datum) const
{
  return _velocity * _grad_phi(datum, j, qp);
}
