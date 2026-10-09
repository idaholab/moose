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
  KOKKOS_FUNCTION Moose::Kokkos::Real3 precomputeQpResidualGrad(const unsigned int qp,
                                                                AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real3 precomputeQpJacobianGrad(const unsigned int j,
                                                                const unsigned int qp,
                                                                AssemblyDatum & datum) const;

private:
  Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _function;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosParsedFuncCoefDiffusion::precomputeQpResidualGrad(const unsigned int qp,
                                                        AssemblyDatum & datum) const
{
  const auto & func = static_cast<const KokkosParsedFunction &>(_function);
  const Real k = func.value(_t, datum.q_point(qp));
  return k * _grad_u(datum, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosParsedFuncCoefDiffusion::precomputeQpJacobianGrad(const unsigned int j,
                                                        const unsigned int qp,
                                                        AssemblyDatum & datum) const
{
  const auto & func = static_cast<const KokkosParsedFunction &>(_function);
  const Real k = func.value(_t, datum.q_point(qp));
  return k * _grad_phi(datum, j, qp);
}
