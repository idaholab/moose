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

class KokkosConstantRate final : public Moose::Kokkos::NodalKernel<KokkosConstantRate>
{
public:
  static InputParameters validParams();

  KokkosConstantRate(const InputParameters & parameters);

  KOKKOS_FUNCTION inline Real computeQpResidual(const dof_id_type node) const;

protected:
  const Moose::Kokkos::Scalar<const Real> _rate;
};

KOKKOS_FUNCTION inline Real
KokkosConstantRate::computeQpResidual(const dof_id_type /* node */) const
{
  return -_rate;
}
