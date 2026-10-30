//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosArrayKernel.h"

/**
 * Implements a source term proportional to the value of a coupled standard or array variable.
 * Weak form: $(\\vec{u}^\\ast, -\\vec{\\sigma} v)$.
 */
class KokkosArrayCoupledForce : public Moose::Kokkos::ArrayKernel
{
public:
  static InputParameters validParams();

  KokkosArrayCoupledForce(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int i,
                                                const unsigned int j,
                                                const unsigned int jvar,
                                                const unsigned int qp,
                                                AssemblyDatum & datum) const;

private:
  /// Whether the coupled variable is an array variable
  const bool _is_v_array;
  /// Coupled variable number, or the number of the first component of an array variable
  const unsigned int _v_var;
  /// Coupled standard variable
  const Moose::Kokkos::VariableValue _v;
  /// Coupled array variable
  const Moose::Kokkos::ArrayVariableValue _v_array;
  /// Multiplier for each component of the coupled force term
  const Moose::Kokkos::Array<Real> _coef;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosArrayCoupledForce::computeQpResidual(const unsigned int i,
                                           const unsigned int qp,
                                           AssemblyDatum & datum) const
{
  const auto v = _is_v_array ? _v_array(datum, qp) : _v(datum, qp, 0);

  return -_coef[datum.comp()] * v * _test(datum, i, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosArrayCoupledForce::computeQpOffDiagJacobian(const unsigned int i,
                                                  const unsigned int j,
                                                  const unsigned int jvar,
                                                  const unsigned int qp,
                                                  AssemblyDatum & datum) const
{
  // An array variable only couples each component to the same component of v
  const auto v_var = _is_v_array ? _v_var + datum.comp() : _v_var;

  if (jvar != v_var)
    return 0;

  return -_coef[datum.comp()] * _phi(datum, j, qp) * _test(datum, i, qp);
}
