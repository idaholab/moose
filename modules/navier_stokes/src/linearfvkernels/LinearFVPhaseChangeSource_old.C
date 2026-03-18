//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVPhaseChangeSource_old.h"

#include <algorithm> // std::clamp

registerMooseObject("NavierStokesApp", LinearFVPhaseChangeSource_old);

InputParameters
LinearFVPhaseChangeSource_old::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();

  params.addClassDescription(
      "Finite-volume elemental kernel that adds the apparent heat-capacity "
      "phase-change source term: rho * L * (df/dT) * T_dot, with f a smoothstep "
      "over [T_solidus, T_liquidus].");

  params.addRequiredParam<MooseFunctorName>("L", "Latent heat.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The mixture density.");
  params.addRequiredParam<MooseFunctorName>("T_solidus", "The solidus temperature.");
  params.addRequiredParam<MooseFunctorName>("T_liquidus", "The liquidus temperature.");

  return params;
}

LinearFVPhaseChangeSource_old::LinearFVPhaseChangeSource_old(const InputParameters & params)
  : LinearFVElementalKernel(params),
    _L(getFunctor<Real>("L")),
    _rho(getFunctor<Real>(NS::density)),
    _T_solidus(getFunctor<Real>("T_solidus")),
    _T_liquidus(getFunctor<Real>("T_liquidus")),
    _time_integrator(_sys.getTimeIntegrator(_var_num)),
    _factor_history(_time_integrator.numStatesRequired(), 0.0),
    _state_args(_time_integrator.numStatesRequired(), determineState())
{
}

Real
LinearFVPhaseChangeSource_old::computeMatrixContribution()
{
  // Element context
  const auto state    = determineState();
  const auto elem_arg = makeElemArg(_current_elem_info->elem());

  const Real T_sol = _T_solidus(elem_arg, state);
  const Real T_liq = _T_liquidus(elem_arg, state);
  const Real dT_pc = T_liq - T_sol;

  // Guard against degenerate or inverted mushy interval
  if (dT_pc <= 0.0)
    return 0.0;

  // Temperature and smoothed liquid fraction in [0,1]
  const Real T      = _var.getElemValue(*_current_elem_info, state);
  const Real s      = std::clamp((T - T_sol) / dT_pc, 0.0, 1.0);

  // df/dT for f(s) = 3s^2 - 2s^3 is 6 s (1 - s) / dT_pc
  const Real dfdT   = 6.0 * s * (1.0 - s) / dT_pc;

  // Apparent heat capacity term rho * L * (df/dT) * T_dot
  const Real rhoL    = _rho(elem_arg, state) * _L(elem_arg, state);
  //const Real Told_dt = _time_integrator.timeDerivativeRHSContribution(_dof_id, _factor_history);
  const Real Tdot    =  _time_integrator.timeDerivativeMatrixContribution(1.0);// - Told_dt;
  if (dfdT > 0)
    _console << "dfdT: " << dfdT << std::endl;
  return rhoL * dfdT * Tdot * _current_elem_volume;
}

Real
LinearFVPhaseChangeSource_old::computeRightHandSideContribution()
{
  // Element context
  const auto state    = determineState();
  const auto elem_arg = makeElemArg(_current_elem_info->elem());

  const Real T_sol = _T_solidus(elem_arg, state);
  const Real T_liq = _T_liquidus(elem_arg, state);
  const Real dT_pc = T_liq - T_sol;

  // Guard against degenerate or inverted mushy interval
  if (dT_pc <= 0.0)
    return 0.0;

  // Temperature and smoothed liquid fraction in [0,1]
  const Real T      = _var.getElemValue(*_current_elem_info, state);
  const Real s      = std::clamp((T - T_sol) / dT_pc, 0.0, 1.0);

  // df/dT for f(s) = 3s^2 - 2s^3 is 6 s (1 - s) / dT_p
  const Real dfdT   = 6.0 * s * (1.0 - s) / dT_pc;

  // Apparent heat capacity term rho * L * (df/dT) * T_dot
  const Real rhoL    = _rho(elem_arg, state) * _L(elem_arg, state);
  const Real Told_dt = _time_integrator.timeDerivativeRHSContribution(_dof_id, _factor_history);
  return rhoL * dfdT * Told_dt * _current_elem_volume;
}

void
LinearFVPhaseChangeSource_old::setCurrentElemInfo(const ElemInfo * elem_info)
{
  LinearFVElementalKernel::setCurrentElemInfo(elem_info);
  for (const auto i : index_range(_factor_history))
    _factor_history[i] = 1.0;
}
