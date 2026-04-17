//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernel.h"

class KokkosBlockPropertyDiffusion : public Moose::Kokkos::Kernel
{
public:
  static InputParameters validParams();

  KokkosBlockPropertyDiffusion(const InputParameters & parameters);

  virtual void initialSetup() override;

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int i,
                                         const unsigned int j,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;

private:
  std::set<SubdomainID> _blocks;

  Moose::Kokkos::Array<bool> _has_prop;
  Moose::Kokkos::MaterialProperty<Real> _prop;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosBlockPropertyDiffusion::computeQpResidual(const unsigned int i,
                                                const unsigned int qp,
                                                AssemblyDatum & datum) const
{
  Real k = _has_prop[datum.subdomain()] ? _prop(datum, qp) : 1.0;
  return k * _grad_u(datum, qp) * _grad_test(datum, i, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosBlockPropertyDiffusion::computeQpJacobian(const unsigned int i,
                                                const unsigned int j,
                                                const unsigned int qp,
                                                AssemblyDatum & datum) const
{
  Real k = _has_prop[datum.subdomain()] ? _prop(datum, qp) : 1.0;
  return k * _grad_phi(datum, j, qp) * _grad_test(datum, i, qp);
}
