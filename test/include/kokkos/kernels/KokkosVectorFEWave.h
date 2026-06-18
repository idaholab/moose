//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernelCurl.h"
#include "KokkosParsedFunction.h"

class KokkosVectorFEWave : public Moose::Kokkos::KernelCurl
{
public:
  static InputParameters validParams();

  KokkosVectorFEWave(const InputParameters & parameters);

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
  const bool _has_y_ffn;
  const bool _has_z_ffn;
  Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _x_ffn;
  Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _y_ffn;
  Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _z_ffn;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosVectorFEWave::computeQpResidual(const unsigned int i,
                                      const unsigned int qp,
                                      AssemblyDatum & datum) const
{
  const auto & x_ffn = static_cast<const KokkosParsedFunction &>(_x_ffn);
  const auto & y_ffn = static_cast<const KokkosParsedFunction &>(_y_ffn);
  const auto & z_ffn = static_cast<const KokkosParsedFunction &>(_z_ffn);
  const auto p = datum.q_point(qp);
  const Moose::Kokkos::Real3 forcing(
      x_ffn.value(_t, p), _has_y_ffn ? y_ffn.value(_t, p) : 0, _has_z_ffn ? z_ffn.value(_t, p) : 0);

  return _curl_test(datum, i, qp) * _curl_u(datum, qp) + _test(datum, i, qp) * _u(datum, qp) -
         forcing * _test(datum, i, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosVectorFEWave::computeQpJacobian(const unsigned int i,
                                      const unsigned int j,
                                      const unsigned int qp,
                                      AssemblyDatum & datum) const
{
  return _curl_test(datum, i, qp) * _curl_phi(datum, j, qp) +
         _test(datum, i, qp) * _phi(datum, j, qp);
}
