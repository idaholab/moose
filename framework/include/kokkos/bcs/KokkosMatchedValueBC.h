//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosNodalBC.h"

/**
 * Implements a simple coupled boundary condition where u=v on the boundary.
 */
class KokkosMatchedValueBC final : public Moose::Kokkos::NodalBC<KokkosMatchedValueBC>
{
public:
  static InputParameters validParams();

  KokkosMatchedValueBC(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const ContiguousNodeID node) const;
  KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int jvar,
                                                const ContiguousNodeID node) const;

protected:
  const Moose::Kokkos::VariableNodalValue _v;

  /// The id of the coupled variable
  const unsigned int _v_num;
  /// Coefficient for primary variable
  const Real _u_coeff;
  /// Coefficient for coupled variable
  const Real _v_coeff;
};

KOKKOS_FUNCTION inline Real
KokkosMatchedValueBC::computeQpResidual(const ContiguousNodeID node) const
{
  return _u_coeff * _u(node) - _v_coeff * _v(node);
}

KOKKOS_FUNCTION inline Real
KokkosMatchedValueBC::computeQpOffDiagJacobian(const unsigned int jvar,
                                               const ContiguousNodeID /* node */) const
{
  if (jvar == _v_num)
    return -_v_coeff;
  else
    return 0;
}
