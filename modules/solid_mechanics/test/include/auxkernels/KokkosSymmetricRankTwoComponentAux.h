//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosAuxKernel.h"

/**
 * Writes one stored Mandel component of a symmetric second-order Kokkos material property into an
 * auxiliary variable, so that a device-resident tensor property can be compared component by
 * component against its non-Kokkos counterpart.
 */
class KokkosSymmetricRankTwoComponentAux : public Moose::Kokkos::AuxKernel
{
public:
  static InputParameters validParams();

  KokkosSymmetricRankTwoComponentAux(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, AssemblyDatum & datum) const;

private:
  /// Property to read
  const Moose::Kokkos::MaterialProperty<Moose::Kokkos::Real6> _prop;

  /// Stored Mandel component index
  const unsigned int _component;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosSymmetricRankTwoComponentAux::computeValue(const unsigned int qp, AssemblyDatum & datum) const
{
  // The stored Mandel component, which is what MaterialSymmetricRankTwoTensorAux reports for a
  // non-Kokkos SymmetricRankTwoTensor, so the two are directly comparable
  const Moose::Kokkos::Real6 & value = _prop(datum, qp);

  return value(_component);
}
