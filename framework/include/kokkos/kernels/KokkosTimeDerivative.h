//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosTimeKernel.h"

class KokkosTimeDerivative : public Moose::Kokkos::TimeKernel
{
public:
  static InputParameters validParams();

  KokkosTimeDerivative(const InputParameters & parameters);

  /**
   * Compute the diagonal Jacobian. With lumping, each entry adds to the diagonal of its row.
   */
  template <typename Derived, typename Team>
  KOKKOS_FUNCTION void computeJacobianInternal(const Derived & kernel,
                                               const Team & team,
                                               const Moose::Kokkos::TeamGroup & group,
                                               bool active,
                                               AssemblyDatum & datum) const;

  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpResidualValue(const unsigned int qp,
                                                 AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpJacobianValue(const unsigned int j,
                                                 const unsigned int qp,
                                                 AssemblyDatum & datum) const;

protected:
  const bool _lumping;
};

template <typename Derived, typename Team>
KOKKOS_FUNCTION void
KokkosTimeDerivative::computeJacobianInternal(const Derived & kernel,
                                              const Team & team,
                                              const Moose::Kokkos::TeamGroup & group,
                                              const bool active,
                                              AssemblyDatum & datum) const
{
  using namespace Moose::Kokkos;

  const auto scaling_factor =
      kokkosSystem(_kokkos_var.sys()).getVariableScalingFactor(_kokkos_var.var());

  assemblePrecomputedTerms<PrecomputeFamily::Jacobian, false>(
      kernel,
      team,
      group,
      _precompute_layout,
      active,
      datum,
      [&](const Real local_ke, const unsigned int i, const unsigned int j)
      {
        accumulateTaggedElementalMatrix(
            local_ke * scaling_factor, datum.elem().id, i, _lumping ? i : j, datum.jvar());
      });
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosTimeDerivative::precomputeQpResidualValue(const unsigned int qp, AssemblyDatum & datum) const
{
  return _u_dot(datum, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosTimeDerivative::precomputeQpJacobianValue(const unsigned int j,
                                                const unsigned int qp,
                                                AssemblyDatum & datum) const
{
  return _phi(datum, j, qp) * _du_dot_du;
}
