//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVPhaseChangeSource.h"

#include "MooseEnum.h"

#include <algorithm> // std::clamp

registerMooseObject("NavierStokesApp", LinearFVPhaseChangeSource);

InputParameters
LinearFVPhaseChangeSource::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();

  params.addClassDescription(
      "Finite-volume elemental kernel that adds the apparent heat-capacity phase-change source term: "
      "rho * L * (df/dT) * T_dot, with f a smoothstep over [T_solidus, T_liquidus]. "
      "Supports formulation='temperature' (variable is T) and formulation='enthalpy' (variable is enthalpy-like, "
      "provide T(var) and dT/d(var) functors). formulation='auto' selects enthalpy if a 'temperature' functor is "
      "provided, otherwise temperature.");

  params.addRequiredParam<MooseFunctorName>("L", "Latent heat.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The mixture density.");
  params.addRequiredParam<MooseFunctorName>("T_solidus", "The solidus temperature.");
  params.addRequiredParam<MooseFunctorName>("T_liquidus", "The liquidus temperature.");

  MooseEnum formulation("auto temperature enthalpy", "auto");
  params.addParam<MooseEnum>(
      "formulation",
      formulation,
      "Meaning of the solved variable. 'temperature' assumes the variable is temperature. 'enthalpy' assumes the "
      "variable is enthalpy-like and requires 'temperature'=T(var) and 'dT_dvar'=dT/d(var). 'auto' (default) selects "
      "'enthalpy' if 'temperature' is provided, otherwise 'temperature'." );

  params.addParam<MooseFunctorName>(
      "temperature",
      "Temperature functor used when formulation='enthalpy' (or when formulation='auto' and this is provided). "
      "This should be T(var). For example, when solving for enthalpy: T_from_p_h." );

  params.addParam<MooseFunctorName>(
      "dT_dvar",
      "1.0",
      "Derivative of the provided 'temperature' functor with respect to the solved variable, i.e. dT/d(var). "
      "For enthalpy solves this is typically dTdh. Not used when formulation='temperature'." );

  return params;
}

LinearFVPhaseChangeSource::LinearFVPhaseChangeSource(const InputParameters & params)
  : LinearFVElementalKernel(params),
    _L(getFunctor<Real>("L")),
    _rho(getFunctor<Real>(NS::density)),
    _T_solidus(getFunctor<Real>("T_solidus")),
    _T_liquidus(getFunctor<Real>("T_liquidus")),
    _enthalpy_formulation([&]()
                          {
                            const MooseEnum f = getParam<MooseEnum>("formulation");
                            if (f == "enthalpy")
                              return true;
                            if (f == "temperature")
                              return false;
                            // auto
                            return isParamValid("temperature");
                          }()),
    _temperature(_enthalpy_formulation ? &getFunctor<Real>("temperature") : nullptr),
    _dT_dvar(getFunctor<Real>("dT_dvar")),
    _time_integrator(_sys.getTimeIntegrator(_var_num)),
    _factor_history(_time_integrator.numStatesRequired(), 1.0)
{
  if (_enthalpy_formulation)
  {
    if (!isParamValid("temperature"))
      paramError("temperature", "Must be provided when using formulation='enthalpy'." );

    // Safety check: default dT_dvar = 1.0 is almost certainly wrong for enthalpy solves
    const MooseFunctorName dT_name = getParam<MooseFunctorName>("dT_dvar");
    if (dT_name == "1.0")
      paramError(
          "dT_dvar",
          "Must be provided when using formulation='enthalpy' (e.g. dTdh from the enthalpy material). "
          "The default value '1.0' is intended only for temperature solves." );
  }
}

Real
LinearFVPhaseChangeSource::computeMatrixContribution()
{
  const auto state = determineState();
  const auto elem_arg = makeElemArg(_current_elem_info->elem());

  const Real T_sol = _T_solidus(elem_arg, state);
  const Real T_liq = _T_liquidus(elem_arg, state);
  const Real dT_pc = T_liq - T_sol;

  if (dT_pc <= 0.0)
    return 0.0;

  const Real T = _enthalpy_formulation ? (*_temperature)(elem_arg, state)
                                       : _var.getElemValue(*_current_elem_info, state);

  const Real s = std::clamp((T - T_sol) / dT_pc, 0.0, 1.0);
  const Real dfdT = 6.0 * s * (1.0 - s) / dT_pc;

  const Real rhoL = _rho(elem_arg, state) * _L(elem_arg, state);

  const Real var_dot_coeff = _time_integrator.timeDerivativeMatrixContribution(1.0);
  const Real Tdot_coeff = _enthalpy_formulation ? _dT_dvar(elem_arg, state) * var_dot_coeff
                                                : var_dot_coeff;

  return rhoL * dfdT * Tdot_coeff * _current_elem_volume;
}

Real
LinearFVPhaseChangeSource::computeRightHandSideContribution()
{
  const auto state = determineState();
  const auto elem_arg = makeElemArg(_current_elem_info->elem());

  const Real T_sol = _T_solidus(elem_arg, state);
  const Real T_liq = _T_liquidus(elem_arg, state);
  const Real dT_pc = T_liq - T_sol;

  if (dT_pc <= 0.0)
    return 0.0;

  const Real T = _enthalpy_formulation ? (*_temperature)(elem_arg, state)
                                       : _var.getElemValue(*_current_elem_info, state);

  const Real s = std::clamp((T - T_sol) / dT_pc, 0.0, 1.0);
  const Real dfdT = 6.0 * s * (1.0 - s) / dT_pc;

  const Real rhoL = _rho(elem_arg, state) * _L(elem_arg, state);

  // This time integrator helper forms the explicit RHS contribution for the time derivative.
  const Real var_old_dt = _time_integrator.timeDerivativeRHSContribution(_dof_id, _factor_history);
  const Real Told_dt = _enthalpy_formulation ? _dT_dvar(elem_arg, state) * var_old_dt : var_old_dt;

  return rhoL * dfdT * Told_dt * _current_elem_volume;
}

void
LinearFVPhaseChangeSource::setCurrentElemInfo(const ElemInfo * elem_info)
{
  LinearFVElementalKernel::setCurrentElemInfo(elem_info);
  // Keep multiplier history as 1.0 for all required states
  for (const auto i : index_range(_factor_history))
    _factor_history[i] = 1.0;
}
