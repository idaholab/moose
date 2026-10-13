//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosADKernel.h"

/**
 * Test kernel exercising pointwise AD with nonlinear functions of the values and gradients of the
 * kernel variable u and a coupled variable v, contributing both a test function value term and a
 * test function gradient term:
 *
 *   (grad psi_i, (1 + u^2) grad u + v grad v) + (psi_i, u v^2 + |grad u|^2)
 */
class KokkosADNonlinearCoupledTest : public Moose::Kokkos::ADKernel
{
public:
  /// u, grad u, v, and grad v
  static constexpr unsigned int max_ad_slots = 8;

  static InputParameters validParams();

  KokkosADNonlinearCoupledTest(const InputParameters & parameters);

  template <typename Derived, unsigned int slots>
  KOKKOS_FUNCTION Moose::Kokkos::DenseADReal3<slots>
  precomputeQpResidualGrad(const unsigned int qp,
                           Moose::Kokkos::ADAssemblyDatum<slots> & datum) const
  {
    const auto u = _u(datum, qp);
    return (1 + u * u) * _grad_u(datum, qp) + _v(datum, qp) * _grad_v(datum, qp);
  }

  template <typename Derived, unsigned int slots>
  KOKKOS_FUNCTION Moose::Kokkos::DenseADReal<slots>
  precomputeQpResidualValue(const unsigned int qp,
                            Moose::Kokkos::ADAssemblyDatum<slots> & datum) const
  {
    const auto v = _v(datum, qp);
    const auto grad_u = _grad_u(datum, qp);
    return _u(datum, qp) * v * v + grad_u * grad_u;
  }

private:
  /// Kernel variable and its gradient
  ///@{
  const Moose::Kokkos::ADVariableValue _u;
  const Moose::Kokkos::ADVariableGradient _grad_u;
  ///@}
  /// Coupled variable and its gradient
  ///@{
  const Moose::Kokkos::ADVariableValue _v;
  const Moose::Kokkos::ADVariableGradient _grad_v;
  ///@}
};
