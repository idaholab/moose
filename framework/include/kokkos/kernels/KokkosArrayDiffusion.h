//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "KokkosArrayKernel.h"

/**
 * The array Laplacian operator with a scalar, diagonal array, or full matrix diffusivity.
 */
class KokkosArrayDiffusion : public Moose::Kokkos::ArrayKernel
{
public:
  static InputParameters validParams();

  KokkosArrayDiffusion(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int i,
                                         const unsigned int j,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int i,
                                                const unsigned int j,
                                                const unsigned int jvar,
                                                const unsigned int qp,
                                                AssemblyDatum & datum) const;

protected:
  const bool _has_d;
  const bool _has_d_1d;
  const bool _has_d_2d;

  const Moose::Kokkos::MaterialProperty<Real> _d;
  const Moose::Kokkos::MaterialProperty<Real, 1> _d_1d;
  const Moose::Kokkos::MaterialProperty<Real, 2> _d_2d;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosArrayDiffusion::computeQpResidual(const unsigned int i,
                                        const unsigned int qp,
                                        AssemblyDatum & datum) const
{
  if (_d)
    return _d(datum, qp) * _grad_u.array(datum, qp) * _grad_test(datum, i, qp);
  else if (_d_1d)
    return _d_1d(datum, qp) * _grad_u.array(datum, qp) * _grad_test(datum, i, qp);
  else
    return _d_2d(datum, qp) * _grad_u.array(datum, qp) * _grad_test(datum, i, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosArrayDiffusion::computeQpJacobian(const unsigned int i,
                                        const unsigned int j,
                                        const unsigned int qp,
                                        AssemblyDatum & datum) const
{
  const auto grad_product = _grad_phi(datum, j, qp) * _grad_test(datum, i, qp);

  if (_d)
    return _d(datum, qp) * grad_product;
  else if (_d_1d)
    return _d_1d.array(datum, qp) * grad_product;
  else
    return _d_2d.array(datum, qp).diagonal() * grad_product;
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosArrayDiffusion::computeQpOffDiagJacobian(const unsigned int i,
                                               const unsigned int j,
                                               const unsigned int jvar,
                                               const unsigned int qp,
                                               AssemblyDatum & datum) const
{
  if (!_d_2d)
    return 0;

  const auto first_var = _kokkos_var.var();
  if (jvar < first_var || jvar - first_var >= _count)
    return 0;

  const auto icomp = datum.comp();
  const auto jcomp = jvar - first_var;
  const auto d = _d_2d(datum, qp);

  return d(icomp, jcomp) * _grad_phi(datum, j, qp) * _grad_test(datum, i, qp);
}
