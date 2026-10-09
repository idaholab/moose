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
 * Kokkos time derivative term of the heat equation with the specific heat \f$ c_p \f$ and the
 * density \f$ \rho \f$ as arguments: \f$ \rho c_p \frac{\partial T}{\partial t} \f$
 */
class KokkosSpecificHeatConductionTimeDerivative
  : public DerivativeMaterialInterface<KokkosTimeDerivative>
{
public:
  static InputParameters validParams();

  KokkosSpecificHeatConductionTimeDerivative(const InputParameters & parameters);

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
  /// Specific heat
  const Moose::Kokkos::MaterialProperty<Real> _specific_heat;
  /// Derivative of the specific heat with respect to the kernel variable
  const Moose::Kokkos::MaterialProperty<Real> _d_specific_heat_dT;
  /// Density
  const Moose::Kokkos::MaterialProperty<Real> _density;
  /// Derivative of the density with respect to the kernel variable
  const Moose::Kokkos::MaterialProperty<Real> _d_density_dT;
  /// Map from coupled variable numbers to the indices of their derivative properties
  Moose::Kokkos::Map<unsigned int, unsigned int> _jvar_to_index;
  /// Derivatives of the specific heat with respect to the coupled variables
  Moose::Kokkos::Array<Moose::Kokkos::MaterialProperty<Real>> _d_specific_heat_dargs;
  /// Derivatives of the density with respect to the coupled variables
  Moose::Kokkos::Array<Moose::Kokkos::MaterialProperty<Real>> _d_density_dargs;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosSpecificHeatConductionTimeDerivative::precomputeQpResidual(const unsigned int qp,
                                                                 AssemblyDatum & datum) const
{
  return _specific_heat(datum, qp) * _density(datum, qp) *
         KokkosTimeDerivative::precomputeQpResidual<Derived>(qp, datum);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosSpecificHeatConductionTimeDerivative::precomputeQpJacobian(const unsigned int j,
                                                                 const unsigned int qp,
                                                                 AssemblyDatum & datum) const
{
  const Real dT = KokkosTimeDerivative::precomputeQpResidual<Derived>(qp, datum);
  const Real phi = _phi(datum, j, qp);

  // on-diagonal Jacobian with all terms that may depend on the kernel variable
  return _specific_heat(datum, qp) * _density(datum, qp) *
             KokkosTimeDerivative::precomputeQpJacobian<Derived>(j, qp, datum) +
         _d_specific_heat_dT(datum, qp) * phi * _density(datum, qp) * dT +
         _specific_heat(datum, qp) * _d_density_dT(datum, qp) * phi * dT;
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosSpecificHeatConductionTimeDerivative::precomputeQpOffDiagJacobian(const unsigned int j,
                                                                        const unsigned int jvar,
                                                                        const unsigned int qp,
                                                                        AssemblyDatum & datum) const
{
  if (!_jvar_to_index.exists(jvar))
    return 0;

  const auto cvar = _jvar_to_index[jvar];
  const Real dT = KokkosTimeDerivative::precomputeQpResidual<Derived>(qp, datum);
  const Real phi = _phi(datum, j, qp);

  // off-diagonal contribution with terms that depend on coupled variables
  return _d_specific_heat_dargs[cvar](datum, qp) * phi * _density(datum, qp) * dT +
         _specific_heat(datum, qp) * _d_density_dargs[cvar](datum, qp) * phi * dT;
}
