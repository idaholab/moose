//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernel.h"

namespace Moose::Kokkos
{

/**
 * The base class for Kokkos time-derivative kernels
 */
class TimeKernel : public Kernel
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   */
  TimeKernel(const InputParameters & parameters);

  /**
   * Copy constructor for parallel dispatch
   */
  TimeKernel(const TimeKernel & object);

  /**
   * Hook for additional computation for residual after the standard calls
   * @param ib The beginning element-local DOF index
   * @param ie The end element-local DOF index
   * @param datum The AssemblyDatum object of the current thread
   * @param local_re The temporary storage storing the residual contribution of each DOF
   */
  KOKKOS_FUNCTION void computeResidualAdditional(const unsigned int /* ib */,
                                                 const unsigned int /* ie */,
                                                 AssemblyDatum & /* datum */,
                                                 Real * /* local_re */) const
  {
  }

  /**
   * The parallel computation body that hides the base class method to allow additional computation
   * for residual through computeResidualAdditional()
   */
  template <typename Derived>
  KOKKOS_FUNCTION void computeResidualInternal(const Derived & kernel, AssemblyDatum & datum) const;

protected:
  /**
   * Time derivative of the current solution at quadrature points
   */
  const VariableValue _u_dot;
  /**
   * Derivative of u_dot with respect to u
   */
  Array<Real> _du_dot_du;
};

template <typename Derived>
KOKKOS_FUNCTION void
TimeKernel::computeResidualInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  ResidualObject::computeResidualInternal(
      datum,
      [&](Real * local_re, const unsigned int ib, const unsigned int ie)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
        {
          datum.reinit();

          for (unsigned int i = ib; i < ie; ++i)
            local_re[i] += datum.JxW(qp) * kernel.computeQpResidualShim(kernel, i, qp, datum);
        }

        kernel.computeResidualAdditional(ib, ie, datum, local_re);
      });
}

} // namespace Moose::Kokkos
