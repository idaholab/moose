//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

class Function;

/**
 * FunctorMaterial that defines a piecewise enthalpy-temperature relationship for
 * phase-change problems (solidification/melting) and provides the inverse mapping.
 *
 * Adopted formulation (reference: h(T_solidus) = 0):
 *   - Solid (T <= T_solidus):
 *       h = \int_{T_solidus}^{T} cp_s(T') dT'
 *   - Mushy (T_solidus < T < T_liquidus):
 *       h = L * f_l(T), with f_l varying linearly over the interval
 *       (no additional sensible-heat contribution in the mushy region)
 *   - Liquid (T >= T_liquidus):
 *       h = L + \int_{T_liquidus}^{T} cp_l(T') dT'
 *
 * Default behavior (backward compatible): the cp functors are treated as constants
 * over the integration interval, so the sensible contributions reduce to cp*(T-T_ref)
 * in the pure phases.
 *
 * Optional behavior: enable integrate_cp_over_temperature=true and provide
 * cp_solid_T_function / cp_liquid_T_function to integrate cp(T) over temperature.
 *
 * This material provides the following Real-valued functors:
 *   - h_from_p_T
 *   - T_from_p_h
 *   - temperature      (alias of T_from_p_h)
 *   - liquid_fraction
 *   - dTdh             (needed to write k*grad(T) = (k*dTdh)*grad(h))
 */
class INSFVPhaseChangeEnthalpyFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  INSFVPhaseChangeEnthalpyFunctorMaterial(const InputParameters & parameters);

protected:
  /// Solid specific heat capacity (constant-over-interval mode)
  const Moose::Functor<Real> & _cp_s;

  /// Liquid specific heat capacity (constant-over-interval mode)
  const Moose::Functor<Real> & _cp_l;

  /// Latent heat of fusion
  const Moose::Functor<Real> & _L;

  /// Solidus temperature
  const Moose::Functor<Real> & _T_solidus;

  /// Liquidus temperature
  const Moose::Functor<Real> & _T_liquidus;

  /// Temperature functor used to build h_from_p_T (typically for boundary conditions)
  const Moose::Functor<Real> & _temperature_in;

  /// Specific enthalpy functor used to build T_from_p_h, temperature, etc.
  const Moose::Functor<Real> & _enthalpy_in;

  /// Whether to integrate temperature-dependent cp(T) correlations over temperature
  const bool _integrate_cp_over_T;

  /// Optional temperature-dependent cp(T) functions (evaluated with x = temperature)
  const Function * _cp_solid_T_function;
  const Function * _cp_liquid_T_function;

  /// Integration and inversion controls for cp(T)
  const unsigned int _cp_integration_subintervals;
  const unsigned int _cp_inversion_max_its;
  const Real _cp_inversion_rel_tol;
  const Real _cp_inversion_abs_tol;
};
