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
  KOKKOS_FUNCTION Moose::Kokkos::Real3 precomputeQpResidualValue(const unsigned int qp,
                                                                 AssemblyDatum & datum) const;

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real3 precomputeQpJacobianValue(const unsigned int j,
                                                                 const unsigned int qp,
                                                                 AssemblyDatum & datum) const;

protected:
  /**
   * Get the component of a vector tangential to a surface, a - (a . n) n. Its product with a test
   * function equals (n x a) . (n x psi), the integrand of the penalty term.
   * @param a The vector
   * @param normal The unit normal of the surface
   * @returns The tangential component
   */
  KOKKOS_FUNCTION static Moose::Kokkos::Real3 tangential(const Moose::Kokkos::Real3 & a,
                                                         const Moose::Kokkos::Real3 & normal)
  {
    return a - (a * normal) * normal;
  }

  const Moose::Kokkos::Scalar<const Real> _penalty;
  const bool _has_function_y;
  const bool _has_function_z;
  Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _function_x;
  Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _function_y;
  Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _function_z;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosVectorCurlPenaltyDirichletBC::precomputeQpResidualValue(const unsigned int qp,
                                                              AssemblyDatum & datum) const
{
  const auto & function_x = static_cast<const KokkosParsedFunction &>(_function_x);
  const auto & function_y = static_cast<const KokkosParsedFunction &>(_function_y);
  const auto & function_z = static_cast<const KokkosParsedFunction &>(_function_z);
  const auto p = datum.q_point(qp);
  const auto normal = _normals(datum, qp);
  const Moose::Kokkos::Real3 u_exact(function_x.value(_t, p),
                                     _has_function_y ? function_y.value(_t, p) : 0,
                                     _has_function_z ? function_z.value(_t, p) : 0);

  return _penalty * tangential(_u(datum, qp) - u_exact, normal);
}

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosVectorCurlPenaltyDirichletBC::precomputeQpJacobianValue(const unsigned int j,
                                                              const unsigned int qp,
                                                              AssemblyDatum & datum) const
{
  return _penalty * tangential(_phi(datum, j, qp), _normals(datum, qp));
}
