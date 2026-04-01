//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosIntegratedBC.h"

/**
 * Implements a flux boundary condition grad(u).n = V.n, where the
 * vector V is specifed by the user. This differs from NeumannBC,
 * where the user instead specifies the _scalar_ value g = grad(u).n.
 */
class KokkosDirectionalNeumannBC : public Moose::Kokkos::IntegratedBC
{
public:
  static InputParameters validParams();

  KokkosDirectionalNeumannBC(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const
  {
    return -_test(datum, i, qp) * (_value * datum.normals(qp));
  }

protected:
  /// Vector to dot with the normal.
  const Moose::Kokkos::Real3 _value;
};
