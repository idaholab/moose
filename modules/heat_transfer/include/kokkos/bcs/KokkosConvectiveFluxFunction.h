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
 * Kokkos convective heat flux boundary condition with the far-field temperature and heat transfer
 * coefficient given by functions
 */
class KokkosConvectiveFluxFunction : public Moose::Kokkos::IntegratedBCValue
{
public:
  static InputParameters validParams();

  KokkosConvectiveFluxFunction(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpJacobian(const unsigned int j,
                                            const unsigned int qp,
                                            AssemblyDatum & datum) const;

private:
  /// Far-field temperature function
  const Moose::Kokkos::Function _T_infinity;
  /// Heat transfer coefficient function
  const Moose::Kokkos::Function _coefficient;
  /// Whether the heat transfer coefficient is a function of temperature instead of time and position
  const bool _coef_of_temperature;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosConvectiveFluxFunction::precomputeQpResidual(const unsigned int qp,
                                                   AssemblyDatum & datum) const
{
  const auto u = _u(datum, qp);
  const auto p = datum.q_point(qp);
  const Real coef = _coef_of_temperature ? _coefficient.value(u, Moose::Kokkos::Real3(0))
                                         : _coefficient.value(_t, p);

  return coef * (u - _T_infinity.value(_t, p));
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosConvectiveFluxFunction::precomputeQpJacobian(const unsigned int j,
                                                   const unsigned int qp,
                                                   AssemblyDatum & datum) const
{
  const auto p = datum.q_point(qp);

  if (!_coef_of_temperature)
    return _coefficient.value(_t, p) * _phi(datum, j, qp);

  const auto u = _u(datum, qp);
  const Moose::Kokkos::Real3 origin(0);
  const Real coef = _coefficient.value(u, origin);
  const Real dcoef_dT = _coefficient.timeDerivative(u, origin);

  return (coef + (u - _T_infinity.value(_t, p)) * dcoef_dT) * _phi(datum, j, qp);
}
