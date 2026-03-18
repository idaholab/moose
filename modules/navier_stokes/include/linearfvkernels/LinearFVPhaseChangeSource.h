//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVElementalKernel.h"
#include "MooseFunctor.h"
#include "NS.h"
#include "TimeIntegrator.h"

/**
 * Finite-volume elemental kernel that adds the apparent heat-capacity
 * phase-change source term:
 *
 *   rho * L * (df/dT) * T_dot
 *
 * with f a smoothstep over [T_solidus, T_liquidus].
 *
 * This kernel supports two common "energy variable" choices:
 *
 *  - formulation = temperature:
 *      The solved variable is temperature (T). The source term is formed
 *      directly using T and T_dot.
 *
 *  - formulation = enthalpy:
 *      The solved variable is an enthalpy-like quantity (h) and you provide
 *        - temperature : a functor T(var, ...)
 *        - dT_dvar     : a functor dT/d(var)
 *      so that T_dot = (dT/dvar) * var_dot.
 *
 * For backward compatibility, formulation = auto will behave like:
 *  - if 'temperature' functor is provided -> enthalpy formulation
 *  - otherwise                             -> temperature formulation
 *
 * Uses a smoothstep f(s) = 3 s^2 - 2 s^3 with
 *   s = clamp((T - T_solidus)/(T_liquidus - T_solidus), 0, 1)
 * so
 *   df/dT = 6 s (1 - s) / (T_liquidus - T_solidus).
 */
class LinearFVPhaseChangeSource : public LinearFVElementalKernel
{
public:
  static InputParameters validParams();

  LinearFVPhaseChangeSource(const InputParameters & params);

  Real computeMatrixContribution() override;
  Real computeRightHandSideContribution() override;

  void setCurrentElemInfo(const ElemInfo * elem_info) override;

protected:
  const Moose::Functor<Real> & _L;
  const Moose::Functor<Real> & _rho;
  const Moose::Functor<Real> & _T_solidus;
  const Moose::Functor<Real> & _T_liquidus;

  /// True when the solved variable is enthalpy-like and we compute temperature via functors
  const bool _enthalpy_formulation;

  /// Temperature functor used when _enthalpy_formulation == true
  const Moose::Functor<Real> * const _temperature;

  /// dT/d(var) functor used when _enthalpy_formulation == true
  const Moose::Functor<Real> & _dT_dvar;

  /// Time integrator for the variable associated with this kernel
  const TimeIntegrator & _time_integrator;

  /// Multiplier history for time integrator (we keep it as 1.0 for all states)
  std::vector<Real> _factor_history;
};
