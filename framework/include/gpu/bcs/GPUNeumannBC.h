//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUIntegratedBC.h"

class GPUNeumannBC final : public GPUIntegratedBC<GPUNeumannBC>
{
public:
  static InputParameters validParams();

  GPUNeumannBC(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         ResidualDatum & datum) const
  {
    return -_test(datum, i, qp) * _value;
  }

private:
  /// Value of grad(u) on the boundary.
  const Real _value;
};
