//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernelValue.h"
#include "KokkosADQpSeed.h"

namespace Moose::Kokkos
{

/**
 * The base class for a user to derive their own Kokkos kernels whose residual is of the form
 *
 * $r_i = \sum_q JxW_q \, \psi_i f_q$
 *
 * and whose linearization is obtained by automatic differentiation.
 *
 * The user defines the value $f_q$ as a function of the solution value and gradient at the
 * quadrature point, templated on the scalar type so that the same body serves both the residual,
 * evaluated on plain reals, and the linearization, evaluated on seeded dual numbers:
 *
 * @tparam T The scalar type
 * @param u The solution value at the quadrature point
 * @param grad_u The physical solution gradient at the quadrature point
 * @param qp The local quadrature point index
 * @param datum The AssemblyDatum object of the current thread
 * @returns The value
 *
 * template <typename T>
 * KOKKOS_FUNCTION T computeQpValue(const T & u,
 *                                  const Moose::Kokkos::Vector3<T> & grad_u,
 *                                  const unsigned int qp,
 *                                  AssemblyDatum & datum) const;
 *
 * See ADKernelGrad for why the solution arrives through the arguments and what that buys.
 */
class ADKernelValue : public KernelValue
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   */
  ADKernelValue(const InputParameters & parameters);

  virtual unsigned int qpJacobianBlocks() const override
  {
    return QP_JACOBIAN_VALUE_VALUE | QP_JACOBIAN_VALUE_GRADIENT;
  }

  /**
   * The hooks the KernelValue loops call, supplied here in terms of the derived class's value
   */
  ///@{
  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpJacobian(const unsigned int j,
                                            const unsigned int qp,
                                            AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION void computeQpJacobianTensor(QpJacobianBlockAccessor & blocks,
                                               const unsigned int qp,
                                               AssemblyDatum & datum) const;
  ///@}
};

template <typename Derived>
KOKKOS_FUNCTION Real
ADKernelValue::precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  const auto & kernel = *static_cast<const Derived *>(this);

  return kernel.computeQpValue(_u(datum, qp), _grad_u(datum, qp), qp, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
ADKernelValue::computeQpJacobianTensor(QpJacobianBlockAccessor & blocks,
                                       const unsigned int qp,
                                       AssemblyDatum & datum) const
{
  const auto & kernel = *static_cast<const Derived *>(this);

  const auto u = seedQpValue(_u(datum, qp));
  const auto grad_u = seedQpGradient(_grad_u(datum, qp), _dimension);

  harvestQpValue(kernel.computeQpValue(u, grad_u, qp, datum), _dimension, blocks);
}

template <typename Derived>
KOKKOS_FUNCTION Real
ADKernelValue::precomputeQpJacobian(const unsigned int j,
                                    const unsigned int qp,
                                    AssemblyDatum & datum) const
{
  const auto & kernel = *static_cast<const Derived *>(this);

  QpJacobianBlockAccessor blocks;

  kernel.template computeQpJacobianTensor<Derived>(blocks, qp, datum);

  return blocks.valueValue() * _phi(datum, j, qp) +
         blocks.valueGradient() * _grad_phi(datum, j, qp);
}

} // namespace Moose::Kokkos
