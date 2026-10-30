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
 * Source term proportional to the sum of several coupled array variables, componentwise:
 * $(\\vec{u}^\\ast, -\\sigma \\sum_k \\vec{v}_k)$. Exercises the plural Kokkos array coupling API.
 */
class KokkosArrayCoupledArraysForce : public Moose::Kokkos::ArrayKernel
{
public:
  static InputParameters validParams();

  KokkosArrayCoupledArraysForce(const InputParameters & parameters);

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
  /// Coupled array variables
  const Moose::Kokkos::Array<Moose::Kokkos::ArrayVariableValue> _v;
  /// Variable number of the first component of each coupled array variable
  Moose::Kokkos::Array<unsigned int> _v_var;
  /// Multiplier for the coupled force term
  const Real _coef;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosArrayCoupledArraysForce::computeQpResidual(const unsigned int i,
                                                 const unsigned int qp,
                                                 AssemblyDatum & datum) const
{
  Real sum = 0;

  for (unsigned int k = 0; k < _v.size(); ++k)
    sum += _v[k](datum, qp);

  return -_coef * sum * _test(datum, i, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosArrayCoupledArraysForce::computeQpOffDiagJacobian(const unsigned int i,
                                                        const unsigned int j,
                                                        const unsigned int jvar,
                                                        const unsigned int qp,
                                                        AssemblyDatum & datum) const
{
  // Each component only couples to the same component of every coupled array variable
  for (unsigned int k = 0; k < _v_var.size(); ++k)
    if (jvar == _v_var[k] + datum.comp())
      return -_coef * _phi(datum, j, qp) * _test(datum, i, qp);

  return 0;
}
