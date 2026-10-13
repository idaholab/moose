//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosVectorKernel.h"

/**
 * Test kernel for the curl-curl operator with a reaction term, (curl psi_i, curl u) + (psi_i, u),
 * which reads no position so that it is usable on the current configuration
 */
class KokkosVectorCurlCurl : public Moose::Kokkos::VectorKernel
{
public:
  static InputParameters validParams();

  KokkosVectorCurlCurl(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real3 precomputeQpResidualCurl(const unsigned int qp,
                                                                AssemblyDatum & datum) const
  {
    return _curl_u(datum, qp);
  }
  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real3 precomputeQpResidualValue(const unsigned int qp,
                                                                 AssemblyDatum & datum) const
  {
    return _u(datum, qp);
  }
  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real3
  precomputeQpJacobianCurl(const unsigned int j, const unsigned int qp, AssemblyDatum & datum) const
  {
    return _curl_phi(datum, j, qp);
  }
  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real3 precomputeQpJacobianValue(const unsigned int j,
                                                                 const unsigned int qp,
                                                                 AssemblyDatum & datum) const
  {
    return _phi(datum, j, qp);
  }
};
