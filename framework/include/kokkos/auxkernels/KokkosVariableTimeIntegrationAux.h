//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosAuxKernel.h"

/**
 * An AuxKernel that can be used to integrate a field variable in time
 * using a variety of different integration methods.  The result is
 * stored in another field variable.
 */
class KokkosVariableTimeIntegrationAux : public Moose::Kokkos::AuxKernel
{
public:
  static InputParameters validParams();

  KokkosVariableTimeIntegrationAux(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, AssemblyDatum & datum) const;

protected:
  KOKKOS_FUNCTION Real getIntegralValue(const unsigned int qp, AssemblyDatum & datum) const;

  Moose::Kokkos::Array<Moose::Kokkos::VariableValue> _coupled_vars;
  const Real _coef;
  const unsigned int _order;
  Moose::Kokkos::Array<Real> _integration_coef;

  /// The old variable value (zero if order == 3)
  const Moose::Kokkos::VariableValue _u_old;
  /// The older variable value (zero if order != 3)
  const Moose::Kokkos::VariableValue _u_older;
};

KOKKOS_FUNCTION inline Real
KokkosVariableTimeIntegrationAux::computeValue(const unsigned int qp, AssemblyDatum & datum) const
{
  Real integral = getIntegralValue(qp, datum);

  if (_order == 3)
    return _u_older(datum, qp) + _coef * integral;

  return _u_old(datum, qp) + _coef * integral;
}

KOKKOS_FUNCTION inline Real
KokkosVariableTimeIntegrationAux::getIntegralValue(const unsigned int qp,
                                                   AssemblyDatum & datum) const
{
  Real integral_value = 0.0;

  for (unsigned int i = 0; i < _order; ++i)
    integral_value += _integration_coef[i] * _coupled_vars[i](datum, qp) * _dt;

  /**
   * Subsequent timesteps may be unequal, so the standard Simpson rule
   * cannot be used. Use a different set of coefficients here.
   * J. McNAMEE, "A PROGRAM TO INTEGRATE A FUNCTION TABULATED AT
   * UNEQUAL INTERVALS," Internation Journal for Numerical Methods in
   * Engineering, Vol. 17, 217-279. (1981).
   */
  if (_order == 3 && _dt != _dt_old)
  {
    Real x0 = 0.0;
    Real x1 = _dt_old;
    Real x2 = _dt + _dt_old;
    Real y0 = _coupled_vars[2](datum, qp);
    Real y1 = _coupled_vars[1](datum, qp);
    Real y2 = _coupled_vars[0](datum, qp);
    Real term1 = (x2 - x0) * (y0 + (x2 - x0) * (y1 - y0) / (2.0 * (x1 - x0)));
    Real term2 = (2.0 * x2 * x2 - x0 * x2 - x0 * x0 + 3.0 * x0 * x1 - 3.0 * x1 * x2) / 6.0;
    Real term3 = (y2 - y1) / (x2 - x1) - (y1 - y0) / (x1 - x0);
    integral_value = term1 + term2 * term3;
  }

  return integral_value;
}
