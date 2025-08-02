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

class KokkosCoupledForceNodalKernel final
  : public Moose::Kokkos::NodalKernel<KokkosCoupledForceNodalKernel>
{
public:
  static InputParameters validParams();

  KokkosCoupledForceNodalKernel(const InputParameters & parameters);

  KOKKOS_FUNCTION inline Real computeQpResidual(const dof_id_type node) const;
  KOKKOS_FUNCTION inline Real computeQpOffDiagJacobian(const unsigned int jvar,
                                                       const dof_id_type node) const;

private:
  /// The number of the coupled variable
  const unsigned int _v_var;

  /// The value of the coupled variable
  const Moose::Kokkos::VariableNodalValue _v;

  /// A multiplicative factor for computing the coupled force
  const Real _coef;
};

KOKKOS_FUNCTION inline Real
KokkosCoupledForceNodalKernel::computeQpResidual(const dof_id_type node) const
{
  return -_coef * _v(node);
}

KOKKOS_FUNCTION inline Real
KokkosCoupledForceNodalKernel::computeQpOffDiagJacobian(const unsigned int jvar,
                                                        const dof_id_type /* node */) const
{
  if (jvar == _v_var)
    return -_coef;
  return 0;
}
