//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernelGrad.h"

/**
 * Kokkos Darcy flux -cond * (grad P - rho * g) of the fluid pressure
 */
class KokkosDarcyFluxPressure : public Moose::Kokkos::KernelGrad
{
  using Real3 = Moose::Kokkos::Real3;

public:
  static InputParameters validParams();

  KokkosDarcyFluxPressure(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real3 precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real3 precomputeQpJacobian(const unsigned int j,
                                             const unsigned int qp,
                                             AssemblyDatum & datum) const;

private:
  /// Hydraulic conductivity
  const Moose::Kokkos::MaterialProperty<Real> _cond;
  /// Gravity
  const Real3 _gravity;
  /// Fluid density
  const Moose::Kokkos::MaterialProperty<Real> _density;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosDarcyFluxPressure::precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  return _cond(datum, qp) * (_grad_u(datum, qp) - _density(datum, qp) * _gravity);
}

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosDarcyFluxPressure::precomputeQpJacobian(const unsigned int j,
                                              const unsigned int qp,
                                              AssemblyDatum & datum) const
{
  return _cond(datum, qp) * _grad_phi(datum, j, qp);
}
