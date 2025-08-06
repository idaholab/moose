//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosTimeNodalKernel.h"

class KokkosTimeDerivativeNodalKernel final
  : public Moose::Kokkos::TimeNodalKernel<KokkosTimeDerivativeNodalKernel>
{
  usingKokkosTimeNodalKernelMembers(KokkosTimeDerivativeNodalKernel);

public:
  static InputParameters validParams();

  KokkosTimeDerivativeNodalKernel(const InputParameters & parameters);

  KOKKOS_FUNCTION inline Real computeQpResidual(const dof_id_type node) const;
  KOKKOS_FUNCTION inline Real computeQpJacobian(const dof_id_type node) const;
};

KOKKOS_FUNCTION inline Real
KokkosTimeDerivativeNodalKernel::computeQpResidual(const dof_id_type node) const
{
  return _u_dot(node);
}

KOKKOS_FUNCTION inline Real
KokkosTimeDerivativeNodalKernel::computeQpJacobian(const dof_id_type /* node */) const
{
  return _du_dot_du;
}
