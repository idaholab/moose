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
 * Constant auxiliary value
 */
class KokkosConstantAux : public Moose::Kokkos::AuxKernel
{
public:
  static InputParameters validParams();

  KokkosConstantAux(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, AssemblyDatum & datum) const;

protected:
  /// The value being set for the current node/element
  Moose::Kokkos::Scalar<const Real> _value;
};

KOKKOS_FUNCTION inline Real
KokkosConstantAux::computeValue(const unsigned int /* qp */, AssemblyDatum & /* datum */) const
{
  return _value;
}
