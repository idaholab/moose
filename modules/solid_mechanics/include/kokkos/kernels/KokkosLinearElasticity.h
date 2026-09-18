//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosVectorKernelGrad.h"

/**
 * Isotropic linear elasticity for a single vector displacement variable, parameterized by the
 * Lame parameters.
 *
 * Weak form $(\sigma(\vec{u}), \nabla \vec{\psi_i})$, with
 * $\sigma = \lambda \, \mathrm{tr}(\epsilon) \, I + 2 \mu \epsilon$ and
 * $\epsilon = \frac{1}{2} \left( \nabla \vec{u} + \nabla \vec{u}^T \right)$, which is the same
 * operator MFEM's ElasticityIntegrator assembles.
 */
class KokkosLinearElasticity : public Moose::Kokkos::VectorKernelGrad
{
public:
  static InputParameters validParams();

  KokkosLinearElasticity(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real33 precomputeQpResidual(const unsigned int qp,
                                                             AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real33
  precomputeQpJacobian(const unsigned int j, const unsigned int qp, AssemblyDatum & datum) const;

private:
  /**
   * The stress produced by a displacement gradient
   * @param grad Displacement gradient, indexed as (component, spatial direction)
   * @param lambda First Lame parameter
   * @param mu Second Lame parameter
   */
  KOKKOS_INLINE_FUNCTION Moose::Kokkos::Real33 stress(const Moose::Kokkos::Real33 & grad,
                                                      const Real lambda,
                                                      const Real mu) const;

  /// First Lame parameter
  const Moose::Kokkos::MaterialProperty<Real> _lambda;
  /// Second Lame parameter, the shear modulus
  const Moose::Kokkos::MaterialProperty<Real> _mu;
};

KOKKOS_INLINE_FUNCTION Moose::Kokkos::Real33
KokkosLinearElasticity::stress(const Moose::Kokkos::Real33 & grad,
                               const Real lambda,
                               const Real mu) const
{
  Moose::Kokkos::Real33 sigma;

  const Real divergence = grad(0, 0) + grad(1, 1) + grad(2, 2);

  for (unsigned int i = 0; i < 3; ++i)
  {
    // 2 mu epsilon, using the symmetric part of the gradient
    for (unsigned int j = 0; j < 3; ++j)
      sigma(i, j) = mu * (grad(i, j) + grad(j, i));

    sigma(i, i) += lambda * divergence;
  }

  return sigma;
}

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real33
KokkosLinearElasticity::precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  return stress(_grad_u(datum, qp), _lambda(datum, qp), _mu(datum, qp));
}

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real33
KokkosLinearElasticity::precomputeQpJacobian(const unsigned int j,
                                             const unsigned int qp,
                                             AssemblyDatum & datum) const
{
  // The stress depends linearly on the displacement gradient, so its derivative with respect to
  // the jth degree of freedom is the stress evaluated on that shape function's gradient.
  return stress(_grad_phi(datum, j, qp), _lambda(datum, qp), _mu(datum, qp));
}
