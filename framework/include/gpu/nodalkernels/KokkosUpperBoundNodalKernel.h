//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUNodalKernel.h"

/**
 * Class used to enforce a upper bound on a coupled variable
 */
class KokkosUpperBoundNodalKernel final
  : public Moose::Kokkos::NodalKernel<KokkosUpperBoundNodalKernel>
{
public:
  static InputParameters validParams();

  KokkosUpperBoundNodalKernel(const InputParameters & parameters);

  KOKKOS_FUNCTION inline Real computeQpResidual(const dof_id_type node) const;
  KOKKOS_FUNCTION inline Real computeQpJacobian(const dof_id_type node) const;
  KOKKOS_FUNCTION inline Real computeQpOffDiagJacobian(const unsigned int jvar,
                                                       const dof_id_type node) const;

private:
  /// The number of the coupled variable
  const unsigned int _v_var;

  /// The value of the coupled variable
  const Moose::Kokkos::VariableNodalValue _v;

  /// Boundaries on which we should not execute this object
  Moose::Kokkos::Array<BoundaryID> _bnd_ids;

  /// The upper bound on the coupled variable
  const Real _upper_bound;
};

KOKKOS_FUNCTION inline Real
KokkosUpperBoundNodalKernel::computeQpResidual(const dof_id_type node) const
{
  for (dof_id_type b = 0; b < _bnd_ids.size(); ++b)
    if (kokkosMesh().isBoundaryNode(node, _bnd_ids[b]))
      return _u(node);

  return ::Kokkos::min(_u(node), _upper_bound - _v(node));
}

KOKKOS_FUNCTION inline Real
KokkosUpperBoundNodalKernel::computeQpJacobian(const dof_id_type node) const
{
  for (dof_id_type b = 0; b < _bnd_ids.size(); ++b)
    if (kokkosMesh().isBoundaryNode(node, _bnd_ids[b]))
      return 1;

  if (_u(node) <= _upper_bound - _v(node))
    return 1;
  return 0;
}

KOKKOS_FUNCTION inline Real
KokkosUpperBoundNodalKernel::computeQpOffDiagJacobian(const unsigned int jvar,
                                                      const dof_id_type node) const
{
  for (dof_id_type b = 0; b < _bnd_ids.size(); ++b)
    if (kokkosMesh().isBoundaryNode(node, _bnd_ids[b]))
      return 0;

  if (jvar == _v_var)
    if (_upper_bound - _v(node) < _u(node))
      return -1;

  return 0;
}
