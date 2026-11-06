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

/**
 * Nonlinear diffusion -div(k grad(u)) with the conductivity k and, optionally, its derivative with
 * respect to u provided by material properties
 */
class KokkosMatNonlinearDiffusion : public Moose::Kokkos::KernelGrad
{
  using Real3 = Moose::Kokkos::Real3;

public:
  static InputParameters validParams();

  KokkosMatNonlinearDiffusion(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real3 precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real3 precomputeQpJacobian(const unsigned int j,
                                             const unsigned int qp,
                                             AssemblyDatum & datum) const;

private:
  /// Conductivity
  Moose::Kokkos::MaterialProperty<Real> _k;
  /// Derivative of the conductivity with respect to u; uninitialized if not provided
  Moose::Kokkos::MaterialProperty<Real> _dk_du;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosMatNonlinearDiffusion::precomputeQpResidual(const unsigned int qp,
                                                  AssemblyDatum & datum) const
{
  return _k(datum, qp) * _grad_u(datum, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosMatNonlinearDiffusion::precomputeQpJacobian(const unsigned int j,
                                                  const unsigned int qp,
                                                  AssemblyDatum & datum) const
{
  Real3 jac = _k(datum, qp) * _grad_phi(datum, j, qp);
  if (_dk_du)
    jac += _dk_du(datum, qp) * _phi(datum, j, qp) * _grad_u(datum, qp);
  return jac;
}
