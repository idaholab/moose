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
#include "KokkosParsedFunction.h"

class KokkosParsedFuncCoefDiffusion : public Moose::Kokkos::Kernel
{
public:
  static InputParameters validParams();

  KokkosParsedFuncCoefDiffusion(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int i,
                                         const unsigned int j,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;

private:
  Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _function;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosParsedFuncCoefDiffusion::computeQpResidual(const unsigned int i,
                                                 const unsigned int qp,
                                                 AssemblyDatum & datum) const
{
  const auto & func = static_cast<const KokkosParsedFunction &>(_function);
  Real k = func.value(_t, datum.q_point(qp));
  return k * _grad_u(datum, qp) * _grad_test(datum, i, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosParsedFuncCoefDiffusion::computeQpJacobian(const unsigned int i,
                                                 const unsigned int j,
                                                 const unsigned int qp,
                                                 AssemblyDatum & datum) const
{
  const auto & func = static_cast<const KokkosParsedFunction &>(_function);
  Real k = func.value(_t, datum.q_point(qp));
  return k * _grad_phi(datum, j, qp) * _grad_test(datum, i, qp);
}
