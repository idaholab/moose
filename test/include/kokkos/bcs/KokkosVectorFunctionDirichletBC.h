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
#include "KokkosParsedFunction.h"

class KokkosVectorFunctionDirichletBC : public Moose::Kokkos::VectorNodalBC
{
public:
  static InputParameters validParams();

  KokkosVectorFunctionDirichletBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real3 computeQpResidual(const unsigned int qp,
                                                         AssemblyDatum & datum) const;

protected:
  const bool _has_function_y;
  const bool _has_function_z;
  Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _function_x;
  Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _function_y;
  Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _function_z;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosVectorFunctionDirichletBC::computeQpResidual(const unsigned int qp,
                                                   AssemblyDatum & datum) const
{
  const auto & function_x = static_cast<const KokkosParsedFunction &>(_function_x);
  const auto & function_y = static_cast<const KokkosParsedFunction &>(_function_y);
  const auto & function_z = static_cast<const KokkosParsedFunction &>(_function_z);
  const auto p = datum.q_point(qp);

  const Moose::Kokkos::Real3 values(function_x.value(_t, p),
                                    _has_function_y ? function_y.value(_t, p) : 0,
                                    _has_function_z ? function_z.value(_t, p) : 0);

  return _u(datum, qp) - values;
}
