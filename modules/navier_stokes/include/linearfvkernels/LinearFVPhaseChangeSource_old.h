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
#include "NS.h"
#include "MooseFunctor.h"
#include "TimeIntegrator.h"

/**
 * Finite-volume elemental kernel that adds the apparent heat-capacity
 * phase-change source term: rho * L * (df/dT) * T_dot.
 *
 * Uses a smoothstep f(s) = 3 s^2 - 2 s^3 with s = clamp((T - T_solidus)/(T_liquidus - T_solidus), 0, 1),
 * so df/dT = 6 s (1 - s) / (T_liquidus - T_solidus).
 */
class LinearFVPhaseChangeSource_old : public LinearFVElementalKernel
{
public:
  static InputParameters validParams();

  /// Constructor
  LinearFVPhaseChangeSource_old(const InputParameters & params);

  Real computeMatrixContribution() override;
  Real computeRightHandSideContribution() override;

  virtual void setCurrentElemInfo(const ElemInfo * elem_info) override;

protected:

  /// Latent heat
  const Moose::Functor<Real> & _L;

  /// Density
  const Moose::Functor<Real> & _rho;

  /// Solidus Temperature
  const Moose::Functor<Real> & _T_solidus;

  /// Liquidus Temperature
  const Moose::Functor<Real> & _T_liquidus;

  /// The time integrator to use in this kernel, will provide information
  /// on how many states are required in the history.
  const TimeIntegrator & _time_integrator;

private:
  /// Current and older values of the material property multiplier.
  std::vector<Real> _factor_history;

  /// State args, the args which will help us fetch the different states of
  /// the material property multiplier. 0th is the current, 1st is old
  /// 2nd is the older. Might not need all, depends on the time integrator.
  std::vector<Moose::StateArg> _state_args;
};
