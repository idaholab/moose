//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "KokkosArrayNodalBC.h"
#include "KokkosReferenceWrapper.h"

/**
 * Imposes constant values on the components of a Kokkos array variable.
 */
class KokkosArrayDirichletBC : public Moose::Kokkos::ArrayNodalBC
{
public:
  static InputParameters validParams();

  KokkosArrayDirichletBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;

protected:
  /// Controllable host values and their device-compatible copy
  const Moose::Kokkos::DualReferenceWrapper<const RealEigenVector, Moose::Kokkos::Array<Real>>
      _values;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosArrayDirichletBC::computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  return _u.array(datum, qp) - _values.device();
}
