//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUKernel.h"

class KokkosCoefDiffusion final : public Moose::Kokkos::Kernel<KokkosCoefDiffusion>
{
public:
  static InputParameters validParams();

  KokkosCoefDiffusion(const InputParameters & parameters);

  KOKKOS_FUNCTION inline Real
  computeQpResidual(const unsigned int i, const unsigned int qp, ResidualDatum & datum) const;
  KOKKOS_FUNCTION inline Real computeQpJacobian(const unsigned int i,
                                                const unsigned int j,
                                                const unsigned int qp,
                                                ResidualDatum & datum) const;

protected:
  const Moose::Kokkos::Scalar<const Real> _coef;
};

KOKKOS_FUNCTION inline Real
KokkosCoefDiffusion::computeQpResidual(const unsigned int i,
                                       const unsigned int qp,
                                       ResidualDatum & datum) const
{
  return _coef * _grad_test(datum, i, qp) * _grad_u(datum, qp);
}

KOKKOS_FUNCTION inline Real
KokkosCoefDiffusion::computeQpJacobian(const unsigned int i,
                                       const unsigned int j,
                                       const unsigned int qp,
                                       ResidualDatum & datum) const
{
  return _coef * _grad_test(datum, i, qp) * _grad_phi(datum, j, qp);
}
