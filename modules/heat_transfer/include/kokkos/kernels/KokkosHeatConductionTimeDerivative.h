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

/**
 * A class for defining the time derivative of the heat equation.
 *
 * By default this Kernel computes:
 *   \f$ \rho * c_p * \frac{\partial T}{\partial t}, \f$
 * where \f$ \rho \f$ and \f$ c_p \f$ are material properties with the names "density" and
 * "specific_heat", respectively.
 */
class KokkosHeatConductionTimeDerivative : public KokkosTimeDerivative
{
public:
  /// Contructor for Heat Equation time derivative term.
  static InputParameters validParams();

  KokkosHeatConductionTimeDerivative(const InputParameters & parameters);

  /// Compute the residual of the Heat Equation time derivative.
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;

  /// Compute the jacobian of the Heat Equation time derivative.
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int j,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;

protected:
  Moose::Kokkos::MaterialProperty<Real> _specific_heat;
  Moose::Kokkos::MaterialProperty<Real> _d_specific_heat_dT;
  Moose::Kokkos::MaterialProperty<Real> _density;
  Moose::Kokkos::MaterialProperty<Real> _d_density_dT;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosHeatConductionTimeDerivative::computeQpResidual(const unsigned int qp,
                                                      AssemblyDatum & datum) const
{
  return _specific_heat(datum, qp) * _density(datum, qp) *
         KokkosTimeDerivative::computeQpResidual<Derived>(qp, datum);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosHeatConductionTimeDerivative::computeQpJacobian(const unsigned int j,
                                                      const unsigned int qp,
                                                      AssemblyDatum & datum) const
{
  const Real cp = _specific_heat(datum, qp);
  const Real rho = _density(datum, qp);

  auto jac = cp * rho * KokkosTimeDerivative::computeQpJacobian<Derived>(j, qp, datum);

  if (_d_specific_heat_dT)
    jac += _d_specific_heat_dT(datum, qp) * rho * _phi(datum, j, qp) *
           KokkosTimeDerivative::computeQpResidual<Derived>(qp, datum);
  if (_d_density_dT)
    jac += cp * _d_density_dT(datum, qp) * _phi(datum, j, qp) *
           KokkosTimeDerivative::computeQpResidual<Derived>(qp, datum);

  return jac;
}
