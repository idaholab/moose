//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUKernel.h"

namespace Moose
{
namespace Kokkos
{

/**
 * The base class for a user to derive his own Kokkos kernels where the residual is of the form
 *
 * $(\dots, \psi_i)$
 *
 * i.e. the test function $(\psi_i)$ can be factored out for optimization.
 *
 * The user is expected to define precomputeQpResidual() and precomputeQpJacobian() instead of
 * computeQpResidual() and computeQpJacobian(). The signature of precomputeQpResidual() expected to
 * be defined in the derived class is as follows:
 *
 * @param qp The local quadrature point index
 * @param datum The ResidualDatum object of the current thread
 * @returns The component of the residual contribution that will be multiplied by the test function
 *
 * KOKKOS_FUNCTION Real precomputeQpResidual(const unsigned int qp,
 *                                           ResidualDatum & datum) const;
 *
 * The signature of precomputeQpJacobian() can be found in the code below. The definition of
 * computeQpOffDiagJacobian() is still the same with the original Kokkos kernel.
 */
template <typename Derived>
class KernelValue : public Kernel<Derived>
{
  usingKokkosKernelMembers(Derived);

public:
  static InputParameters validParams()
  {
    InputParameters params = Kernel<Derived>::validParams();
    return params;
  }

  /**
   * Constructor
   */
  KernelValue(const InputParameters & parameters) : Kernel<Derived>(parameters) {}

  /**
   * Default methods to prevent compile errors even when these methods were not defined in the
   * derived class
   */
  ///@{
  /**
   * Compute diagonal Jacobian contribution on a quadrature point
   * @param j The trial function DOF index
   * @param qp The local quadrature point index
   * @param datum The ResidualDatum object of the current thread
   * @returns The component of the Jacobian contribution that will be multiplied by the test
   * function
   */
  KOKKOS_FUNCTION Real precomputeQpJacobian(const unsigned int j,
                                            const unsigned int qp,
                                            ResidualDatum & /* datum */) const
  {
    return 0;
  }
  ///@}

  /**
   * The parallel computation bodies that hide the base class methods to optimize for factoring
   * out the test function
   */
  ///@{
  KOKKOS_FUNCTION void computeResidualInternal(const Derived * kernel, ResidualDatum & datum) const;
  KOKKOS_FUNCTION void computeJacobianInternal(const Derived * kernel, ResidualDatum & datum) const;
  ///@}

protected:
  /**
   * Get whether precomputeQpJacobian() was not defined in the derived class
   * @returns Whether precomputeQpJacobian() was not defined in the derived class
   */
  virtual bool defaultJacobian() const override
  {
    return &Derived::precomputeQpJacobian == &KernelValue::precomputeQpJacobian;
  }
};

template <typename Derived>
KOKKOS_FUNCTION void
KernelValue<Derived>::computeResidualInternal(const Derived * kernel, ResidualDatum & datum) const
{
  Real local_re[MAX_DOF];

  unsigned int num_batches = datum.n_dofs() / MAX_DOF;

  if (datum.n_dofs() % MAX_DOF)
    ++num_batches;

  for (unsigned int batch = 0; batch < num_batches; ++batch)
  {
    unsigned int ib = batch * MAX_DOF;
    unsigned int ie = ::Kokkos::min(ib + MAX_DOF, datum.n_dofs());

    for (unsigned int i = ib; i < ie; ++i)
      local_re[i - ib] = 0;

    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    {
      datum.reinit();

      Real value = datum.JxW(qp) * kernel->precomputeQpResidual(qp, datum);

      for (unsigned int i = ib; i < ie; ++i)
        local_re[i - ib] += value * _test(datum, i, qp);
    }

    for (unsigned int i = ib; i < ie; ++i)
      accumulateTaggedElementalResidual(local_re[i - ib], datum.elem().id, i);
  }
}

template <typename Derived>
KOKKOS_FUNCTION void
KernelValue<Derived>::computeJacobianInternal(const Derived * kernel, ResidualDatum & datum) const
{
  Real local_ke[MAX_DOF];
  Real value;

  unsigned int num_batches = datum.n_idofs() * datum.n_jdofs() / MAX_DOF;

  if ((datum.n_idofs() * datum.n_jdofs()) % MAX_DOF)
    ++num_batches;

  for (unsigned int batch = 0; batch < num_batches; ++batch)
  {
    unsigned int ijb = batch * MAX_DOF;
    unsigned int ije = ::Kokkos::min(ijb + MAX_DOF, datum.n_idofs() * datum.n_jdofs());

    for (unsigned int ij = ijb; ij < ije; ++ij)
      local_ke[ij - ijb] = 0;

    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    {
      datum.reinit();

      unsigned int j_old = libMesh::invalid_uint;

      for (unsigned int ij = ijb; ij < ije; ++ij)
      {
        unsigned int i = ij % datum.n_jdofs();
        unsigned int j = ij / datum.n_jdofs();

        if (j != j_old)
        {
          value = datum.JxW(qp) * kernel->precomputeQpJacobian(j, qp, datum);
          j_old = j;
        }

        local_ke[ij - ijb] += value * _test(datum, i, qp);
      }
    }

    for (unsigned int ij = ijb; ij < ije; ++ij)
    {
      unsigned int i = ij % datum.n_jdofs();
      unsigned int j = ij / datum.n_jdofs();

      accumulateTaggedElementalMatrix(local_ke[ij - ijb], datum.elem().id, i, j, datum.jvar());
    }
  }
}

} // namespace Kokkos
} // namespace Moose

#define usingKokkosKernelValueMembers(T) usingKokkosKernelMembers(T)
