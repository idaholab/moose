//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosBoundNodalKernel.h"

/**
 * Class used to enforce a upper bound on a coupled variable
 */
class KokkosUpperBoundNodalKernel final : public KokkosBoundNodalKernel<KokkosUpperBoundNodalKernel>
{
public:
  static InputParameters validParams();

  KokkosUpperBoundNodalKernel(const InputParameters & parameters);

  KOKKOS_FUNCTION Real getResidual(const dof_id_type node) const;
  KOKKOS_FUNCTION Real getJacobian(const dof_id_type node) const;
  KOKKOS_FUNCTION Real getOffDiagJacobian(const unsigned int jvar, const dof_id_type node) const;

private:
  /// The upper bound on the coupled variable
  const Real _upper_bound;
};

KOKKOS_FUNCTION inline Real
KokkosUpperBoundNodalKernel::getResidual(const dof_id_type node) const
{
  return ::Kokkos::min(_u(node), _upper_bound - _v(node));
}

KOKKOS_FUNCTION inline Real
KokkosUpperBoundNodalKernel::getJacobian(const dof_id_type node) const
{
  if (_u(node) <= _upper_bound - _v(node))
    return 1;

  return 0;
}

KOKKOS_FUNCTION inline Real
KokkosUpperBoundNodalKernel::getOffDiagJacobian(const unsigned int jvar,
                                                const dof_id_type node) const
{
  if (jvar == _v_var)
    if (_upper_bound - _v(node) < _u(node))
      return -1;

  return 0;
}
