//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosADIntegratedBC.h"

/**
 * Implements a Neumann BC where grad(u)=_coupled_var on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class KokkosADCoupledVarNeumannBC : public Moose::Kokkos::ADIntegratedBC
{
public:
  static InputParameters validParams();

  KokkosADCoupledVarNeumannBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::ADReal
  computeQpResidual(const unsigned int i, const unsigned int qp, AssemblyDatum & datum) const;

protected:
  /// Variable providing the value of grad(u) on the boundary.
  const Moose::Kokkos::ADVariableValue _coupled_var;

  /// A coefficient that is multiplied with the residual contribution
  const Real _coef;

  /// Scale factor
  const Moose::Kokkos::ADVariableValue _scale_factor;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::ADReal
KokkosADCoupledVarNeumannBC::computeQpResidual(const unsigned int i,
                                               const unsigned int qp,
                                               AssemblyDatum & datum) const
{
  return -_test(datum, i, qp) * _scale_factor(datum, qp) * _coef * _coupled_var(datum, qp);
}
