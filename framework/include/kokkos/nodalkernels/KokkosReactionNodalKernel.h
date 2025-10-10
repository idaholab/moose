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

class KokkosReactionNodalKernel : public Moose::Kokkos::NodalKernel
{
public:
  static InputParameters validParams();

  KokkosReactionNodalKernel(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int qp, AssemblyDatum & datum) const;

protected:
  const Real _coeff;
};

KOKKOS_FUNCTION inline Real
KokkosReactionNodalKernel::computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  return _coeff * _u(datum, qp);
}

KOKKOS_FUNCTION inline Real
KokkosReactionNodalKernel::computeQpJacobian(const unsigned int /* qp */,
                                             AssemblyDatum & /* datum */) const
{
  return _coeff;
}
