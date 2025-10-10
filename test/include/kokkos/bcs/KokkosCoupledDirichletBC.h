//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosDirichletBC.h"

/**
 * Implements the Dirichlet boundary condition
 * c*u + u^2 + v^2 = _value
 * where "u" is the current variable, and "v" is a coupled variable.
 * Note: without the constant term, a zero initial guess gives you a
 * zero row in the Jacobian, which is a bad thing.
 */
class KokkosCoupledDirichletBC final : public KokkosDirichletBC<KokkosCoupledDirichletBC>
{
public:
  static InputParameters validParams();

  KokkosCoupledDirichletBC(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int qp, AssemblyDatum & datum) const;
  KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int jvar,
                                                const unsigned int qp,
                                                AssemblyDatum & datum) const;

protected:
  // The coupled variable
  const Moose::Kokkos::VariableValue _v;

  /// The id of the coupled variable
  const unsigned int _v_num;

  // The constant (not user-selectable for now)
  const Real _c;
};

KOKKOS_FUNCTION inline Real
KokkosCoupledDirichletBC::computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  return _c * _u(datum, qp) + _u(datum, qp) * _u(datum, qp) + _v(datum, qp) * _v(datum, qp) -
         _value;
}

KOKKOS_FUNCTION inline Real
KokkosCoupledDirichletBC::computeQpJacobian(const unsigned int qp, AssemblyDatum & datum) const
{
  return _c + 2 * _u(datum, qp);
}

KOKKOS_FUNCTION inline Real
KokkosCoupledDirichletBC::computeQpOffDiagJacobian(const unsigned int jvar,
                                                   const unsigned int qp,
                                                   AssemblyDatum & datum) const
{
  if (jvar == _v_num)
    return 2 * _v(datum, qp);
  else
    return 0;
}
