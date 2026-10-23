//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosVectorNodalBC.h"
#include "KokkosFunction.h"

/**
 * Imposes the essential boundary condition u = g(t, x) on a vector variable, where the components
 * of g are calculated with Kokkos functions.
 */
class KokkosVectorFunctionDirichletBC : public Moose::Kokkos::VectorNodalBC
{
public:
  static InputParameters validParams();

  KokkosVectorFunctionDirichletBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real3 computeQpResidual(const unsigned int qp,
                                                         AssemblyDatum & datum) const;

protected:
  /// Optional vector function
  const Moose::Kokkos::Function _function;
  /// Component functions, used if the vector function is not provided
  ///@{
  const Moose::Kokkos::Function _function_x;
  const Moose::Kokkos::Function _function_y;
  const Moose::Kokkos::Function _function_z;
  ///@}
  /// Whether the vector function is provided
  const bool _has_function;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosVectorFunctionDirichletBC::computeQpResidual(const unsigned int qp,
                                                   AssemblyDatum & datum) const
{
  const auto p = datum.q_point(qp);

  Moose::Kokkos::Real3 values;
  if (_has_function)
    values = _function.vectorValue(_t, p);
  else
    values = Moose::Kokkos::Real3(
        _function_x.value(_t, p), _function_y.value(_t, p), _function_z.value(_t, p));

  return _u(datum, qp) - values;
}
