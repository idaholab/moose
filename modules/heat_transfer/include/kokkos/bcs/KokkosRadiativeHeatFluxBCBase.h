//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosIntegratedBCValue.h"
#include "KokkosFunction.h"

/**
 * Base class for Kokkos boundary conditions for radiative heat exchange with a body of
 * temperature \f$ T_\infty \f$. The derived class defines the hook
 *
 * template <typename Derived>
 * KOKKOS_FUNCTION Real coefficient(const unsigned int qp, AssemblyDatum & datum) const;
 *
 * returning the effective emissivity of the radiative exchange.
 */
class KokkosRadiativeHeatFluxBCBase : public Moose::Kokkos::IntegratedBCValue
{
public:
  static InputParameters validParams();

  KokkosRadiativeHeatFluxBCBase(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpJacobian(const unsigned int j,
                                            const unsigned int qp,
                                            AssemblyDatum & datum) const;

protected:
  /// Stefan-Boltzmann constant
  const Real _sigma_stefan_boltzmann;
  /// Temperature of the body in radiative heat transfer
  const Moose::Kokkos::Function _tinf;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosRadiativeHeatFluxBCBase::precomputeQpResidual(const unsigned int qp,
                                                    AssemblyDatum & datum) const
{
  const auto & bc = *static_cast<const Derived *>(this);
  const Real T = _u(datum, qp);
  const Real Tinf = _tinf.value(_t, datum.q_point(qp));
  const Real T4 = T * T * T * T;
  const Real T4inf = Tinf * Tinf * Tinf * Tinf;

  return _sigma_stefan_boltzmann * bc.template coefficient<Derived>(qp, datum) * (T4 - T4inf);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosRadiativeHeatFluxBCBase::precomputeQpJacobian(const unsigned int j,
                                                    const unsigned int qp,
                                                    AssemblyDatum & datum) const
{
  const auto & bc = *static_cast<const Derived *>(this);
  const Real T = _u(datum, qp);
  const Real T3 = T * T * T;

  return 4 * _sigma_stefan_boltzmann * bc.template coefficient<Derived>(qp, datum) * T3 *
         _phi(datum, j, qp);
}
