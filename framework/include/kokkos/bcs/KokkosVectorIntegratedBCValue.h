//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosVectorIntegratedBC.h"

namespace Moose::Kokkos
{

/**
 * The base class for Kokkos integrated boundary conditions on vector variables where the residual
 * is of the form $(\dots, \psi_i)$ and the vector test function can be factored out.
 */
class VectorIntegratedBCValue : public VectorIntegratedBC
{
public:
  static InputParameters validParams();

  /// VectorIntegratedBCValue hooks factor out the test function
  static constexpr bool use_precompute_hooks = true;

  /**
   * Constructor
   */
  VectorIntegratedBCValue(const InputParameters & parameters);

  /**
   * Default methods to prevent compile errors even when these methods were not defined in the
   * derived class
   */
  ///@{
  template <typename Derived>
  KOKKOS_FUNCTION Real3 precomputeQpJacobian(const unsigned int /* j */,
                                             const unsigned int /* qp */,
                                             AssemblyDatum & /* datum */) const
  {
    ::Kokkos::abort("Default precomputeQpJacobian() should never be called. Make sure you properly "
                    "redefined this method in your class without typos.");

    return Real3(0);
  }
  template <typename Derived>
  KOKKOS_FUNCTION Real3 precomputeQpOffDiagJacobian(const unsigned int /* j */,
                                                    const unsigned int /* jvar */,
                                                    const unsigned int /* qp */,
                                                    AssemblyDatum & /* datum */) const
  {
    ::Kokkos::abort(
        "Default precomputeQpOffDiagJacobian() should never be called. Make sure you properly "
        "redefined this method in your class without typos.");

    return Real3(0);
  }
  ///@}

  /**
   * Functions used to check if users have overriden the hook methods, whose calculations can be
   * skipped when not overriden
   */
  ///@{
  template <typename Derived>
  static auto defaultJacobian()
  {
    return &VectorIntegratedBCValue::precomputeQpJacobian<Derived>;
  }
  template <typename Derived>
  static auto defaultOffDiagJacobian()
  {
    return &VectorIntegratedBCValue::precomputeQpOffDiagJacobian<Derived>;
  }
  ///@}

  /**
   * Optimized computation bodies that factor out the vector test function
   */
  ///@{
  template <typename Derived>
  KOKKOS_FUNCTION void computeResidualInternal(const Derived & bc, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION void computeJacobianInternal(const Derived & bc, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION void computeOffDiagJacobianInternal(const Derived & bc,
                                                      AssemblyDatum & datum) const;
  ///@}
};

template <typename Derived>
KOKKOS_FUNCTION void
VectorIntegratedBCValue::computeResidualInternal(const Derived & bc, AssemblyDatum & datum) const
{
  ResidualObject::computeResidualInternal(
      datum,
      [&](Real * local_re, const unsigned int ib, const unsigned int ie)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
        {
          Real3 value = datum.JxW(qp) * bc.template precomputeQpResidual<Derived>(qp, datum);

          for (unsigned int i = ib; i < ie; ++i)
            local_re[i] += value * _test(datum, i, qp);
        }
      });
}

template <typename Derived>
KOKKOS_FUNCTION void
VectorIntegratedBCValue::computeJacobianInternal(const Derived & bc, AssemblyDatum & datum) const
{
  ResidualObject::computeJacobianInternal(
      datum,
      [&](Real * local_ke, const unsigned int ib, const unsigned int ie, const unsigned int j)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
        {
          Real3 value = datum.JxW(qp) * bc.template precomputeQpJacobian<Derived>(j, qp, datum);

          for (unsigned int i = ib; i < ie; ++i)
            local_ke[i] += value * _test(datum, i, qp);
        }
      });
}

template <typename Derived>
KOKKOS_FUNCTION void
VectorIntegratedBCValue::computeOffDiagJacobianInternal(const Derived & bc,
                                                        AssemblyDatum & datum) const
{
  ResidualObject::computeJacobianInternal(
      datum,
      [&](Real * local_ke, const unsigned int ib, const unsigned int ie, const unsigned int j)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
        {
          Real3 value = datum.JxW(qp) * bc.template precomputeQpOffDiagJacobian<Derived>(
                                            j, datum.jvar(), qp, datum);

          for (unsigned int i = ib; i < ie; ++i)
            local_ke[i] += value * _test(datum, i, qp);
        }
      });
}

} // namespace Moose::Kokkos
