//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosVectorIntegratedBC.h"
#include "KokkosParsedFunction.h"

class KokkosVectorCurlPenaltyDirichletBC : public Moose::Kokkos::VectorIntegratedBC
{
public:
  static InputParameters validParams();

  KokkosVectorCurlPenaltyDirichletBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int i,
                                         const unsigned int j,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;

protected:
  const Moose::Kokkos::Scalar<const Real> _penalty;
  const bool _has_function_y;
  const bool _has_function_z;
  Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _function_x;
  Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _function_y;
  Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _function_z;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosVectorCurlPenaltyDirichletBC::computeQpResidual(const unsigned int i,
                                                      const unsigned int qp,
                                                      AssemblyDatum & datum) const
{
  const auto & function_x = static_cast<const KokkosParsedFunction &>(_function_x);
  const auto & function_y = static_cast<const KokkosParsedFunction &>(_function_y);
  const auto & function_z = static_cast<const KokkosParsedFunction &>(_function_z);
  const auto p = datum.q_point(qp);
  const auto normal = datum.normals(qp);
  const Moose::Kokkos::Real3 u_exact(function_x.value(_t, p),
                                     _has_function_y ? function_y.value(_t, p) : 0,
                                     _has_function_z ? function_z.value(_t, p) : 0);

  const auto ncu = (_u(datum, qp) - u_exact).cross_product(normal);
  return _penalty * (ncu * _test(datum, i, qp).cross_product(normal));
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosVectorCurlPenaltyDirichletBC::computeQpJacobian(const unsigned int i,
                                                      const unsigned int j,
                                                      const unsigned int qp,
                                                      AssemblyDatum & datum) const
{
  const auto normal = datum.normals(qp);
  return _penalty *
         (_phi(datum, j, qp).cross_product(normal) * _test(datum, i, qp).cross_product(normal));
}
