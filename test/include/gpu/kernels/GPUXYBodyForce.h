//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUBodyForce.h"

class GPUXYBodyForce final : public GPUBodyForce<GPUXYBodyForce>
{
public:
  static InputParameters validParams();

  GPUXYBodyForce(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         ResidualDatum & datum) const
  {
    return (datum.q_point(qp)(0) + datum.q_point(qp)(1)) *
           GPUBodyForce::computeQpResidual(i, qp, datum);
  }
};
