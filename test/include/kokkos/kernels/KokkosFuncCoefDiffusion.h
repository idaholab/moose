//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernel.h"
#include "KokkosFunction.h"

class KokkosFuncCoefDiffusion : public Moose::Kokkos::Kernel
{
public:
  static InputParameters validParams();

  KokkosFuncCoefDiffusion(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int i,
                                         const unsigned int j,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;

private:
  const Moose::Kokkos::Function _function;
};

KOKKOS_FUNCTION inline Real
KokkosFuncCoefDiffusion::computeQpResidual(const unsigned int i,
                                           const unsigned int qp,
                                           AssemblyDatum & datum) const
{
  Real k = _function.value(_t, datum.q_point(qp));
  return k * _grad_u(datum, qp) * _grad_test(datum, i, qp);
}

KOKKOS_FUNCTION inline Real
KokkosFuncCoefDiffusion::computeQpJacobian(const unsigned int i,
                                           const unsigned int j,
                                           const unsigned int qp,
                                           AssemblyDatum & datum) const
{
  Real k = _function.value(_t, datum.q_point(qp));
  return k * _grad_phi(datum, j, qp) * _grad_test(datum, i, qp);
}
