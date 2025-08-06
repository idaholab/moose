//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosNodalKernel.h"

/**
 * Dummy class that tests the Jacobian calculation
 */
class KokkosJacobianCheck final : public Moose::Kokkos::NodalKernel<KokkosJacobianCheck>
{
public:
  static InputParameters validParams();

  KokkosJacobianCheck(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const dof_id_type node) const;
  KOKKOS_FUNCTION Real computeQpJacobian(const dof_id_type node) const;
};

KOKKOS_FUNCTION inline Real
KokkosJacobianCheck::computeQpResidual(const dof_id_type node) const
{
  return -5.0 * _u(node);
}

KOKKOS_FUNCTION inline Real
KokkosJacobianCheck::computeQpJacobian(const dof_id_type /* node */) const
{
  return -5.0;
}
