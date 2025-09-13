//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernelGrad.h"

using Real3 = Moose::Kokkos::Real3;

class KokkosDiffusionPrecompute final : public Moose::Kokkos::KernelGrad<KokkosDiffusionPrecompute>
{
public:
  static InputParameters validParams();

  KokkosDiffusionPrecompute(const InputParameters & parameters);

  KOKKOS_FUNCTION Real3 precomputeQpResidual(const unsigned int qp, ResidualDatum & datum) const;
  KOKKOS_FUNCTION Real3 precomputeQpJacobian(const unsigned int j,
                                             const unsigned int qp,
                                             ResidualDatum & datum) const;
};

KOKKOS_FUNCTION inline Real3
KokkosDiffusionPrecompute::precomputeQpResidual(const unsigned int qp, ResidualDatum & datum) const
{
  // Note we do not multiply by the gradient of the test function. That is done in the parent
  // class
  return _grad_u(datum, qp);
}

KOKKOS_FUNCTION inline Real3
KokkosDiffusionPrecompute::precomputeQpJacobian(const unsigned int j,
                                                const unsigned int qp,
                                                ResidualDatum & datum) const
{
  // Note we do not multiply by the gradient of the test function. That is done in the parent
  // class
  return _grad_phi(datum, j, qp);
}
