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

class GPURestartDiffusion final : public GPUDiffusion<GPURestartDiffusion>
{
public:
  static InputParameters validParams();

  GPURestartDiffusion(const InputParameters & parameters);

  virtual void timestepSetup();

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         ResidualDatum & datum) const
  {
    return _coef(_step) * GPUDiffusion::computeQpResidual(i, qp, datum);
  }
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int i,
                                         const unsigned int j,
                                         const unsigned int qp,
                                         ResidualDatum & datum) const
  {
    return _coef(_step) * GPUDiffusion::computeQpJacobian(i, j, qp, datum);
  }

protected:
  GPUScalar<unsigned int> _step;
  GPUReferenceWrapper<GPUArray<Real>> _coef;
};
