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
 * Writes one stored Mandel component of a symmetric fourth-order Kokkos material property into an
 * auxiliary variable, so that a device-resident tangent can be compared entry by entry against its
 * non-Kokkos counterpart.
 */
class KokkosSymmetricRankFourComponentAux : public Moose::Kokkos::AuxKernel
{
public:
  static InputParameters validParams();

  KokkosSymmetricRankFourComponentAux(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, AssemblyDatum & datum) const;

private:
  /// Property to read
  const Moose::Kokkos::MaterialProperty<Moose::Kokkos::Real66> _prop;

  ///@{
  /// Stored Mandel row and column indices
  const unsigned int _i;
  const unsigned int _j;
  ///@}
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosSymmetricRankFourComponentAux::computeValue(const unsigned int qp,
                                                 AssemblyDatum & datum) const
{
  const Moose::Kokkos::Real66 & value = _prop(datum, qp);

  return value(_i, _j);
}
