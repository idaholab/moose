// //* This file is part of the MOOSE framework
// //* https://www.mooseframework.org
// //*
// //* All rights reserved, see COPYRIGHT for full restrictions
// //* https://github.com/idaholab/moose/blob/master/COPYRIGHT
// //*
// //* Licensed under LGPL 2.1, please see LICENSE for details
// //* https://www.gnu.org/licenses/lgpl-2.1.html

// #pragma once

// #include "LinearFVElementalKernel.h"
// #include "NS.h"
// #include "MooseFunctor.h"
// #include "TimeIntegrator.h"

// /**
//  * Finite-volume elemental kernel that adds the apparent heat-capacity
//  * phase-change source term: rho * L * (df/dT) * T_dot.
//  *
//  * Uses a smoothstep f(s) = 3 s^2 - 2 s^3 with s = clamp((T - T_solidus)/(T_liquidus - T_solidus), 0, 1),
//  * so df/dT = 6 s (1 - s) / (T_liquidus - T_solidus).
//  */
// class LinearFVPhaseChangeSource : public LinearFVElementalKernel
// {
// public:
//   static InputParameters validParams();

//   /// Constructor
//   LinearFVPhaseChangeSource(const InputParameters & params);

//   Real computeMatrixContribution() override;
//   Real computeRightHandSideContribution() override;

//   virtual void setCurrentElemInfo(const ElemInfo * elem_info) override;

// protected:

//   /// Latent heat
//   const Moose::Functor<Real> & _L;

//   /// Density
//   const Moose::Functor<Real> & _rho;

//   /// Solidus Temperature
//   const Moose::Functor<Real> & _T_solidus;

//   /// Liquidus Temperature
//   const Moose::Functor<Real> & _T_liquidus;

//   /// The time integrator to use in this kernel, will provide information
//   /// on how many states are required in the history.
//   const TimeIntegrator & _time_integrator;

// private:
//   /// Current and older values of the material property multiplier.
//   std::vector<Real> _factor_history;

//   /// State args, the args which will help us fetch the different states of
//   /// the material property multiplier. 0th is the current, 1st is old
//   /// 2nd is the older. Might not need all, depends on the time integrator.
//   std::vector<Moose::StateArg> _state_args;
// };

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
 * The liquid fraction f is defined over the mushy interval
 * [T_solidus, T_liquidus] with s = clamp((T - T_solidus)/(T_liquidus - T_solidus), 0, 1):
 *  - smoothing='smooth' (default): f(s) = 3 s^2 - 2 s^3, so df/dT = 6 s (1 - s) / dT_pc
 *  - smoothing='sharp': f(s) = s (linear), so df/dT = 1 / dT_pc inside the interval, 0 outside
 */
class LinearFVPhaseChangeSource : public LinearFVElementalKernel
{
public:
  static InputParameters validParams();

  /// Constructor
  LinearFVPhaseChangeSource(const InputParameters & params);

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

  /// Whether to use the smoothstep liquid fraction (true, default) or the
  /// linear ("sharp") liquid fraction (false)
  const bool _smooth;

  /// The time integrator to use in this kernel, will provide information
  /// on how many states are required in the history.
  const TimeIntegrator & _time_integrator;

private:
  /// Compute df/dT at temperature T for the selected liquid-fraction shape
  Real computeDfDT(const Real T, const Real T_sol, const Real dT_pc) const;

  /// Current and older values of the material property multiplier.
  std::vector<Real> _factor_history;

  /// State args, the args which will help us fetch the different states of
  /// the material property multiplier. 0th is the current, 1st is old
  /// 2nd is the older. Might not need all, depends on the time integrator.
  std::vector<Moose::StateArg> _state_args;
};
