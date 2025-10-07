//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Normalized1PhaseResidualNorm.h"
#include "SinglePhaseFluidProperties.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", Normalized1PhaseResidualNorm);

InputParameters
Normalized1PhaseResidualNorm::validParams()
{
  InputParameters params = DiscreteVariableResidualNorm::validParams();

  params.addClassDescription(
      "Computes a normalized residual norm for the single-phase flow model.");

  params.addRequiredParam<Real>("p_ref", "Reference pressure [Pa]");
  params.addRequiredParam<Real>("T_ref", "Reference temperature [K]");
  params.addRequiredParam<Real>("vel_ref", "Reference velocity [m/s]");
  params.addRequiredParam<FunctionName>("A", "Cross-sectional area function [m^2]");
  params.addRequiredParam<Point>("point",
                                 "Point at which to evaluate cross-sectional area function [m]");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Single-phase fluid properties object");
  params.addRequiredParam<Real>("min_elem_size", "Minimum element size on block [m]");

  return params;
}

Normalized1PhaseResidualNorm::Normalized1PhaseResidualNorm(const InputParameters & parameters)
  : DiscreteVariableResidualNorm(parameters), _initialized(false)
{
}

void
Normalized1PhaseResidualNorm::initialize()
{
  DiscreteVariableResidualNorm::initialize();

  // This cannot be done in constructor or initialSetup() due to some fluid
  // properties not being initialized yet.
  if (!_initialized)
  {
    _normalization = computeNormalization();
    _initialized = true;
  }
}

Real
Normalized1PhaseResidualNorm::computeNormalization() const
{
  const auto h_min = getParam<Real>("min_elem_size");

  const auto & A_fn = getFunction("A");
  const auto & point = getParam<Point>("point");
  const auto A_ref = A_fn.value(0.0, point);

  const auto p_ref = getParam<Real>("p_ref");
  const auto T_ref = getParam<Real>("T_ref");
  const auto vel_ref = getParam<Real>("vel_ref");

  const auto & fp = getUserObject<SinglePhaseFluidProperties>("fluid_properties");
  const auto rho_ref = fp.rho_from_p_T(p_ref, T_ref);

  const auto variable = getParam<VariableName>("variable");
  if (variable == "rhoA")
    return rho_ref * A_ref * h_min;
  else if (variable == "rhouA")
    return rho_ref * vel_ref * A_ref * h_min;
  else if (variable == "rhoEA")
  {
    const auto e_ref = fp.e_from_p_T(p_ref, T_ref);
    const auto E_ref = e_ref + 0.5 * vel_ref * vel_ref;
    return rho_ref * E_ref * A_ref * h_min;
  }
  else
    mooseError("The 'variable' parameter must be one of the following: {rhoA, rhouA, rhoEA}.");
}

Real
Normalized1PhaseResidualNorm::getValue() const
{
  return DiscreteVariableResidualNorm::getValue() / _normalization;
}
