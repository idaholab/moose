//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosDiffusion.h"

class KokkosRestartDiffusion : public KokkosDiffusion
{
public:
  static InputParameters validParams();

  KokkosRestartDiffusion(const InputParameters & parameters);

  virtual void timestepSetup() override;

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real3 precomputeQpResidualGrad(const unsigned int qp,
                                                                AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real3 precomputeQpJacobianGrad(const unsigned int j,
                                                                const unsigned int qp,
                                                                AssemblyDatum & datum) const;

protected:
  Moose::Kokkos::Scalar<unsigned int> _step;
  Moose::Kokkos::ReferenceWrapper<Moose::Kokkos::Array<Real>> _coef;
  int & _last_t_step;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosRestartDiffusion::precomputeQpResidualGrad(const unsigned int qp, AssemblyDatum & datum) const
{
  return _coef(_step) * KokkosDiffusion::precomputeQpResidualGrad<Derived>(qp, datum);
}

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosRestartDiffusion::precomputeQpJacobianGrad(const unsigned int j,
                                                 const unsigned int qp,
                                                 AssemblyDatum & datum) const
{
  return _coef(_step) * KokkosDiffusion::precomputeQpJacobianGrad<Derived>(j, qp, datum);
}
