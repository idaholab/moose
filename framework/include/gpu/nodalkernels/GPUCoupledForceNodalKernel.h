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

class GPUCoupledForceNodalKernel final : public GPUNodalKernel<GPUCoupledForceNodalKernel>
{
public:
  static InputParameters validParams();

  GPUCoupledForceNodalKernel(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const dof_id_type node) const { return -_coef * _v(node); }
  KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int jvar,
                                                const dof_id_type /* node */) const
  {
    if (jvar == _v_var)
      return -_coef;
    return 0;
  }

private:
  /// The number of the coupled variable
  const unsigned int _v_var;

  /// The value of the coupled variable
  GPUVariableNodalValue _v;

  /// A multiplicative factor for computing the coupled force
  const Real _coef;
};
