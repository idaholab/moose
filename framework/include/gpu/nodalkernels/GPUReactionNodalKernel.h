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

class KokkosReactionNodalKernel final : public Moose::Kokkos::NodalKernel<KokkosReactionNodalKernel>
{
public:
  static InputParameters validParams();

  KokkosReactionNodalKernel(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const dof_id_type node) const { return _coeff * _u(node); }
  KOKKOS_FUNCTION Real computeQpJacobian(const dof_id_type /* node */) const { return _coeff; }

protected:
  const Real _coeff;
};
