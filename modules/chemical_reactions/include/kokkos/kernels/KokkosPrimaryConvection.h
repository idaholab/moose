//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernelValue.h"

/**
 * Kokkos convection of the primary species by the Darcy velocity
 */
class KokkosPrimaryConvection : public Moose::Kokkos::KernelValue
{
  using Real3 = Moose::Kokkos::Real3;

public:
  static InputParameters validParams();

  KokkosPrimaryConvection(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpJacobian(const unsigned int j,
                                            const unsigned int qp,
                                            AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpOffDiagJacobian(const unsigned int j,
                                                   const unsigned int jvar,
                                                   const unsigned int qp,
                                                   AssemblyDatum & datum) const;

private:
  /// Darcy velocity
  KOKKOS_FUNCTION Real3 darcyVelocity(const unsigned int qp, AssemblyDatum & datum) const
  {
    return -_cond(datum, qp) * (_grad_p(datum, qp) - _density(datum, qp) * _gravity);
  }

  /// Hydraulic conductivity
  const Moose::Kokkos::MaterialProperty<Real> _cond;
  /// Gravity
  const Real3 _gravity;
  /// Fluid density
  const Moose::Kokkos::MaterialProperty<Real> _density;
  /// Pressure gradient
  const Moose::Kokkos::VariableGradient _grad_p;
  /// Pressure variable number
  const unsigned int _pvar;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosPrimaryConvection::precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  return darcyVelocity(qp, datum) * _grad_u(datum, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosPrimaryConvection::precomputeQpJacobian(const unsigned int j,
                                              const unsigned int qp,
                                              AssemblyDatum & datum) const
{
  return darcyVelocity(qp, datum) * _grad_phi(datum, j, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosPrimaryConvection::precomputeQpOffDiagJacobian(const unsigned int j,
                                                     const unsigned int jvar,
                                                     const unsigned int qp,
                                                     AssemblyDatum & datum) const
{
  if (jvar != _pvar)
    return 0;

  return -_cond(datum, qp) * (_grad_phi(datum, j, qp) * _grad_u(datum, qp));
}
