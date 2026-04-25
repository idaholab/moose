//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosIntegratedBCValue.h"

/**
 * Boundary condition for radiative heat flux where temperature and the
 * temperature of a body in radiative heat transfer are specified.
 */
class KokkosRadiativeHeatFluxBCBase : public Moose::Kokkos::IntegratedBCValue
{
public:
  static InputParameters validParams();
  KokkosRadiativeHeatFluxBCBase(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int j,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;

protected:
  /// Stefan-Boltzmann constant
  const Real _sigma_stefan_boltzmann;

  /// The temperature of the body irhs
  const Real _tinf;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosRadiativeHeatFluxBCBase::computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  auto bc = static_cast<const Derived *>(this);

  Real T = _u(datum, qp);
  Real T4 = T * T * T * T;
  Real T4inf = _tinf * _tinf * _tinf * _tinf;
  return _sigma_stefan_boltzmann * bc->coefficient() * (T4 - T4inf);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosRadiativeHeatFluxBCBase::computeQpJacobian(const unsigned int j,
                                                 const unsigned int qp,
                                                 AssemblyDatum & datum) const
{
  auto bc = static_cast<const Derived *>(this);

  Real T = _u(datum, qp);
  Real T3 = T * T * T;
  return 4 * _sigma_stefan_boltzmann * bc->coefficient() * T3 * _phi(datum, j, qp);
}
