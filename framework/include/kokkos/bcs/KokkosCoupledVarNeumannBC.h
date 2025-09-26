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
 * Implements a Neumann BC where grad(u)=_coupled_var on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class KokkosCoupledVarNeumannBC : public Moose::Kokkos::IntegratedBC
{
public:
  static InputParameters validParams();

  KokkosCoupledVarNeumannBC(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         ResidualDatum & datum) const;
  KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int i,
                                                const unsigned int j,
                                                const unsigned int jvar,
                                                const unsigned int qp,
                                                ResidualDatum & datum) const;

protected:
  /// Variable providing the value of grad(u) on the boundary.
  const Moose::Kokkos::VariableValue _coupled_var;

  /// The identifying number of the coupled variable
  const unsigned int _coupled_num;

  /// A coefficient that is multiplied with the residual contribution
  const Real _coef;

  /// Scale factor
  const Moose::Kokkos::VariableValue _scale_factor;
};

KOKKOS_FUNCTION inline Real
KokkosCoupledVarNeumannBC::computeQpResidual(const unsigned int i,
                                             const unsigned int qp,
                                             ResidualDatum & datum) const
{
  return -_scale_factor(datum, qp) * _coef * _test(datum, i, qp) * _coupled_var(datum, qp);
}

KOKKOS_FUNCTION inline Real
KokkosCoupledVarNeumannBC::computeQpOffDiagJacobian(const unsigned int i,
                                                    const unsigned int j,
                                                    const unsigned int jvar,
                                                    const unsigned int qp,
                                                    ResidualDatum & datum) const
{
  if (jvar == _coupled_num)
    return -_scale_factor(datum, qp) * _coef * _test(datum, i, qp) * _phi(datum, j, qp);
  else
    return 0;
}
