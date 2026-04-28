//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosNeumannBC.h"

/**
 * Implements a Neumann BC where D grad(u) = value * M on the boundary, where
 * value is a constant and M is a material property.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class KokkosMatNeumannBC : public KokkosNeumannBC
{
public:
  static InputParameters validParams();

  KokkosMatNeumannBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;

protected:
  /// Value of material property on the boundary.
  const Moose::Kokkos::MaterialProperty<Real> _boundary_prop;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosMatNeumannBC::computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  return -_value * _boundary_prop(datum, qp);
}
