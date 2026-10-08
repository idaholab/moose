//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosTimeDerivative.h"
#include "KokkosMap.h"
#include "DerivativeMaterialInterface.h"

/**
 * Kokkos time derivative term of the heat equation with the heat capacity \f$ C_p \f$ as an
 * argument: \f$ C_p \frac{\partial T}{\partial t} \f$
 */
class KokkosHeatCapacityConductionTimeDerivative
  : public DerivativeMaterialInterface<KokkosTimeDerivative>
{
public:
  static InputParameters validParams();

  KokkosHeatCapacityConductionTimeDerivative(const InputParameters & parameters);

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
  /// Heat capacity
  const Moose::Kokkos::MaterialProperty<Real> _heat_capacity;
  /// Derivative of the heat capacity with respect to the kernel variable
  const Moose::Kokkos::MaterialProperty<Real> _d_heat_capacity_dT;
  /// Map from coupled variable numbers to the indices of their derivative properties
  Moose::Kokkos::Map<unsigned int, unsigned int> _jvar_to_index;
  /// Derivatives of the heat capacity with respect to the coupled variables
  Moose::Kokkos::Array<Moose::Kokkos::MaterialProperty<Real>> _d_heat_capacity_dargs;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosHeatCapacityConductionTimeDerivative::precomputeQpResidual(const unsigned int qp,
                                                                 AssemblyDatum & datum) const
{
  return _heat_capacity(datum, qp) * KokkosTimeDerivative::precomputeQpResidual<Derived>(qp, datum);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosHeatCapacityConductionTimeDerivative::precomputeQpJacobian(const unsigned int j,
                                                                 const unsigned int qp,
                                                                 AssemblyDatum & datum) const
{
  // on-diagonal Jacobian with all terms that may depend on the kernel variable
  return _heat_capacity(datum, qp) *
             KokkosTimeDerivative::precomputeQpJacobian<Derived>(j, qp, datum) +
         _d_heat_capacity_dT(datum, qp) * _phi(datum, j, qp) *
             KokkosTimeDerivative::precomputeQpResidual<Derived>(qp, datum);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosHeatCapacityConductionTimeDerivative::precomputeQpOffDiagJacobian(const unsigned int j,
                                                                        const unsigned int jvar,
                                                                        const unsigned int qp,
                                                                        AssemblyDatum & datum) const
{
  if (!_jvar_to_index.exists(jvar))
    return 0;

  // off-diagonal contribution with terms that depend on coupled variables
  return _d_heat_capacity_dargs[_jvar_to_index[jvar]](datum, qp) * _phi(datum, j, qp) *
         KokkosTimeDerivative::precomputeQpResidual<Derived>(qp, datum);
}
