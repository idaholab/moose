//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosEquilibriumReactionFlux.h"
#include "KokkosKernelValue.h"

/**
 * Kokkos convection of an equilibrium species by the Darcy velocity
 */
class KokkosCoupledConvectionReactionSub
  : public KokkosEquilibriumReactionFlux<Moose::Kokkos::KernelValue>
{
  using Real3 = Moose::Kokkos::Real3;

public:
  static InputParameters validParams();

  KokkosCoupledConvectionReactionSub(const InputParameters & parameters);

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
  KOKKOS_FUNCTION Real3 darcyVelocity(const unsigned int qp, AssemblyDatum & datum) const;

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

KOKKOS_FUNCTION inline Moose::Kokkos::Real3
KokkosCoupledConvectionReactionSub::darcyVelocity(const unsigned int qp,
                                                  AssemblyDatum & datum) const
{
  return -_cond(datum, qp) * (_grad_p(datum, qp) - _density(datum, qp) * _gravity);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosCoupledConvectionReactionSub::precomputeQpResidual(const unsigned int qp,
                                                         AssemblyDatum & datum) const
{
  return factor(qp, datum) * (darcyVelocity(qp, datum) * flux(qp, datum));
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosCoupledConvectionReactionSub::precomputeQpJacobian(const unsigned int j,
                                                         const unsigned int qp,
                                                         AssemblyDatum & datum) const
{
  return factor(qp, datum) * (darcyVelocity(qp, datum) * fluxJacobian(j, qp, datum));
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosCoupledConvectionReactionSub::precomputeQpOffDiagJacobian(const unsigned int j,
                                                                const unsigned int jvar,
                                                                const unsigned int qp,
                                                                AssemblyDatum & datum) const
{
  if (jvar == _pvar)
    return factor(qp, datum) * ((-_cond(datum, qp) * _grad_phi(datum, j, qp)) * flux(qp, datum));

  // If jvar is not one of the coupled species, return 0
  if (!_var_to_index.exists(jvar))
    return 0;

  return factor(qp, datum) *
         (darcyVelocity(qp, datum) * fluxOffDiagJacobian(j, _var_to_index[jvar], qp, datum));
}
