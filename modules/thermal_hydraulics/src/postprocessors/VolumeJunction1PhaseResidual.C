//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumeJunction1PhaseResidual.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", VolumeJunction1PhaseResidual);

InputParameters
VolumeJunction1PhaseResidual::validParams()
{
  InputParameters params = NodeElemVariableResidualNorm::validParams();

  params.addClassDescription("Computes a normalized residual norm for VolumeJunction1Phase.");

  params.addRequiredParam<Real>("p_ref", "Reference pressure [Pa]");
  params.addRequiredParam<Real>("T_ref", "Reference temperature [K]");
  params.addRequiredParam<Real>("vel_ref", "Reference velocity [m/s]");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Single-phase fluid properties object");
  params.addRequiredParam<Real>("volume", "Volume of junction [m^3]");

  return params;
}

VolumeJunction1PhaseResidual::VolumeJunction1PhaseResidual(const InputParameters & parameters)
  : NodeElemVariableResidualNorm(parameters), _initialized(false)
{
}

void
VolumeJunction1PhaseResidual::initialize()
{
  NodeElemVariableResidualNorm::initialize();

  // This cannot be done in constructor or initialSetup() due to some fluid
  // properties not being initialized yet.
  if (!_initialized)
  {
    _normalization = computeNormalization();
    _initialized = true;
  }
}

Real
VolumeJunction1PhaseResidual::computeNormalization() const
{
  const auto V = getParam<Real>("volume");

  const auto p_ref = getParam<Real>("p_ref");
  const auto T_ref = getParam<Real>("T_ref");
  const auto vel_ref = getParam<Real>("vel_ref");

  const auto & fp = getUserObject<SinglePhaseFluidProperties>("fluid_properties");
  const auto rho_ref = fp.rho_from_p_T(p_ref, T_ref);

  const auto variable = getParam<VariableName>("variable");
  if (variable == "rhoV")
    return rho_ref * V;
  else if (variable == "rhouV" || variable == "rhovV" || variable == "rhowV")
    return rho_ref * vel_ref * V;
  else if (variable == "rhoEV")
  {
    const auto e_ref = fp.e_from_p_T(p_ref, T_ref);
    const auto E_ref = e_ref + 0.5 * vel_ref * vel_ref;
    return rho_ref * E_ref * V;
  }
  else
    mooseError("The 'variable' parameter must be one of the following: {rhoV, rhouV, rhovV, rhowV, "
               "rhoEV}.");
}

Real
VolumeJunction1PhaseResidual::getValue() const
{
  return NodeElemVariableResidualNorm::getValue() / _normalization;
}
