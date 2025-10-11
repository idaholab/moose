//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosBodyForce.h"

class KokkosXYBodyForce : public KokkosBodyForce
{
public:
  static InputParameters validParams();

  KokkosXYBodyForce(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;
};

KOKKOS_FUNCTION inline Real
KokkosXYBodyForce::computeQpResidual(const unsigned int i,
                                     const unsigned int qp,
                                     AssemblyDatum & datum) const
{
  return (datum.q_point(qp)(0) + datum.q_point(qp)(1)) *
         KokkosBodyForce::computeQpResidual(i, qp, datum);
}
