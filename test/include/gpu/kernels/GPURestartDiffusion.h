//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUDiffusion.h"

class KokkosRestartDiffusion final : public KokkosDiffusion<KokkosRestartDiffusion>
{
public:
  static InputParameters validParams();

  KokkosRestartDiffusion(const InputParameters & parameters);

  virtual void timestepSetup();

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         ResidualDatum & datum) const
  {
    return _coef(_step) * KokkosDiffusion::computeQpResidual(i, qp, datum);
  }
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int i,
                                         const unsigned int j,
                                         const unsigned int qp,
                                         ResidualDatum & datum) const
  {
    return _coef(_step) * KokkosDiffusion::computeQpJacobian(i, j, qp, datum);
  }

protected:
  Moose::Kokkos::Scalar<unsigned int> _step;
  Moose::Kokkos::ReferenceWrapper<Moose::Kokkos::Array<Real>> _coef;
};
