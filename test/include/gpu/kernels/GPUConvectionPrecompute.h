//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUKernelValue.h"

class GPUConvectionPrecompute final : public GPUKernelValue<GPUConvectionPrecompute>
{
public:
  static InputParameters validParams();

  GPUConvectionPrecompute(const InputParameters & parameters);

  KOKKOS_FUNCTION Real precomputeQpResidual(const unsigned int qp, ResidualDatum & datum) const
  {
    return _velocity * _grad_u(datum, qp);
  }
  KOKKOS_FUNCTION Real precomputeQpJacobian(const unsigned int j,
                                            const unsigned int qp,
                                            ResidualDatum & datum) const
  {
    return _velocity * _grad_phi(datum, j, qp);
  }

private:
  Real3 _velocity;
};
