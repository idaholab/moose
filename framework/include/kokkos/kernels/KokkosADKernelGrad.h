//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernelGrad.h"
#include "KokkosADQpSeed.h"

namespace Moose::Kokkos
{

/**
 * The base class for a user to derive their own Kokkos kernels whose residual is of the form
 *
 * $r_i = \sum_q JxW_q \, \nabla\psi_i \cdot F_q$
 *
 * and whose linearization is obtained by automatic differentiation.
 *
 * The user defines the flux $F_q$ as a function of the solution value and gradient at the
 * quadrature point, templated on the scalar type so that the same body serves both the residual,
 * evaluated on plain reals, and the linearization, evaluated on seeded dual numbers:
 *
 * @tparam T The scalar type
 * @param u The solution value at the quadrature point
 * @param grad_u The physical solution gradient at the quadrature point
 * @param qp The local quadrature point index
 * @param datum The AssemblyDatum object of the current thread
 * @returns The flux
 *
 * template <typename T>
 * KOKKOS_FUNCTION Moose::Kokkos::Vector3<T> computeQpFlux(const T & u,
 *                                                         const Moose::Kokkos::Vector3<T> & grad_u,
 *                                                         const unsigned int qp,
 *                                                         AssemblyDatum & datum) const;
 *
 * Taking the solution through the arguments, and not through the variable accessors, is what allows
 * the derivative seeds to be placed on the quadrature-point pair: the derivative width is then set
 * by the spatial dimension, so linearizing a quadrature point costs the same at every polynomial
 * order. It also makes the body a pure function of the solution at one point, which is the property
 * the quadrature-point Jacobian cache needs in order to supply the operator action and diagonal of
 * a whole p-multigrid hierarchy from a single fill. Any solution dependence reaching the body by
 * another route is invisible to the seeds and so absent from the linearization.
 */
class ADKernelGrad : public KernelGrad
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   */
  ADKernelGrad(const InputParameters & parameters);

  virtual unsigned int qpJacobianBlocks() const override
  {
    return QP_JACOBIAN_GRADIENT_VALUE | QP_JACOBIAN_GRADIENT_GRADIENT;
  }

  /**
   * The hooks the KernelGrad loops call, supplied here in terms of the derived class's flux
   */
  ///@{
  template <typename Derived>
  KOKKOS_FUNCTION Real3 precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real3 precomputeQpJacobian(const unsigned int j,
                                             const unsigned int qp,
                                             AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION void computeQpJacobianTensor(QpJacobianBlockAccessor & blocks,
                                               const unsigned int qp,
                                               AssemblyDatum & datum) const;
  ///@}
};

template <typename Derived>
KOKKOS_FUNCTION Real3
ADKernelGrad::precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  const auto & kernel = *static_cast<const Derived *>(this);

  return kernel.computeQpFlux(_u(datum, qp), _grad_u(datum, qp), qp, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
ADKernelGrad::computeQpJacobianTensor(QpJacobianBlockAccessor & blocks,
                                      const unsigned int qp,
                                      AssemblyDatum & datum) const
{
  const auto & kernel = *static_cast<const Derived *>(this);

  const auto u = seedQpValue(_u(datum, qp));
  const auto grad_u = seedQpGradient(_grad_u(datum, qp), _dimension);

  harvestQpFlux(kernel.computeQpFlux(u, grad_u, qp, datum), _dimension, blocks);
}

template <typename Derived>
KOKKOS_FUNCTION Real3
ADKernelGrad::precomputeQpJacobian(const unsigned int j,
                                   const unsigned int qp,
                                   AssemblyDatum & datum) const
{
  const auto & kernel = *static_cast<const Derived *>(this);

  // Assembling the element Jacobian evaluates the linearization once per trial function, since the
  // trial index is the outer loop of the assembly. The quadrature-point Jacobian cache evaluates it
  // once per quadrature point and contracts the result against every basis function it needs.
  QpJacobianBlockAccessor blocks;

  kernel.template computeQpJacobianTensor<Derived>(blocks, qp, datum);

  return blocks.fluxValue() * _phi(datum, j, qp) + blocks.fluxGradient() * _grad_phi(datum, j, qp);
}

} // namespace Moose::Kokkos
