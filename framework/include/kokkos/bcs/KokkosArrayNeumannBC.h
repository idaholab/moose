//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "KokkosArrayIntegratedBC.h"
#include "KokkosReferenceWrapper.h"

/**
 * Imposes constant normal gradients on the components of a Kokkos array variable.
 */
class KokkosArrayNeumannBC : public Moose::Kokkos::ArrayIntegratedBC
{
public:
  static InputParameters validParams();

  KokkosArrayNeumannBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;

protected:
  /// Host component values retained so controlled values can be copied to the device
  const RealEigenVector _value_host;
  /// Host component values and their device-compatible copy
  const Moose::Kokkos::DualReferenceWrapper<const RealEigenVector, Moose::Kokkos::Array<Real>>
      _value;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosArrayNeumannBC::computeQpResidual(const unsigned int i,
                                        const unsigned int qp,
                                        AssemblyDatum & datum) const
{
  return -_test(datum, i, qp) * _value.device()[datum.comp()];
}
