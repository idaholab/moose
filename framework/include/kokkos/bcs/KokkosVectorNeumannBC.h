//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosVectorIntegratedBCValue.h"

/**
 * Imposes a constant flux on each component of a vector variable along a boundary
 */
class KokkosVectorNeumannBC : public Moose::Kokkos::VectorIntegratedBCValue
{
public:
  static InputParameters validParams();

  KokkosVectorNeumannBC(const InputParameters & parameters);
  KokkosVectorNeumannBC(const KokkosVectorNeumannBC & object);

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real3 precomputeQpResidual(const unsigned int,
                                                            AssemblyDatum &) const
  {
    return -_values;
  }

protected:
  /// Components of the prescribed flux on the boundary, on the host
  const RealVectorValue & _values_host;
  /// Components of the prescribed flux on the boundary
  const Moose::Kokkos::Real3 _values;
};
