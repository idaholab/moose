//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernelGrad.h"

using Real3 = Moose::Kokkos::Real3;

class KokkosHeatConduction : public Moose::Kokkos::KernelGrad
{
public:
  static InputParameters validParams();

  KokkosHeatConduction(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real3 computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real3 computeQpJacobian(const unsigned int j,
                                          const unsigned int qp,
                                          AssemblyDatum & datum) const;

private:
  Moose::Kokkos::MaterialProperty<Real> _diffusion_coefficient;
  Moose::Kokkos::MaterialProperty<Real> _diffusion_coefficient_dT;
};

template <typename Derived>
KOKKOS_FUNCTION Real3
KokkosHeatConduction::computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  return _diffusion_coefficient(datum, qp) * _grad_u(datum, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Real3
KokkosHeatConduction::computeQpJacobian(const unsigned int j,
                                        const unsigned int qp,
                                        AssemblyDatum & datum) const
{
  Real3 jac = _diffusion_coefficient(datum, qp) * _grad_phi(datum, j, qp);
  if (_diffusion_coefficient_dT)
    jac += _diffusion_coefficient_dT(datum, qp) * _phi(datum, j, qp) * _grad_u(datum, qp);
  return jac;
}
