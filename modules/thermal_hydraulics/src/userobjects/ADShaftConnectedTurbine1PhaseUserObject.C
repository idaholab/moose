//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADShaftConnectedTurbine1PhaseUserObject.h"
#include "ShaftConnectedTurbine1Phase.h"
#include "SinglePhaseFluidProperties.h"
#include "VolumeJunction1Phase.h"
#include "MooseVariableScalar.h"
#include "THMIndices3Eqn.h"
#include "Function.h"
#include "Numerics.h"
#include "metaphysicl/parallel_numberarray.h"
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/utility.h"

registerMooseObject("ThermalHydraulicsApp", ADShaftConnectedTurbine1PhaseUserObject);

InputParameters
ADShaftConnectedTurbine1PhaseUserObject::validParams()
{
  InputParameters params = ADVolumeJunction1PhaseUserObject::validParams();
  params += ADShaftConnectableUserObjectInterface::validParams();

  params.addParam<BoundaryName>("inlet", "Turbine inlet");
  params.addParam<BoundaryName>("outlet", "Turbine outlet");
  params.addRequiredParam<Point>("di_out", "Direction of connected outlet");
  params.addRequiredParam<Real>("omega_rated", "Rated turbine speed [rad/s]");
  params.addRequiredParam<Real>("D_wheel", "Diameter of turbine wheel [m]");
  params.addRequiredParam<Real>("speed_cr_fr", "Turbine speed threshold for friction [-]");
  params.addRequiredParam<Real>("tau_fr_const", "Turbine friction constant [N-m]");
  params.addRequiredParam<std::vector<Real>>("tau_fr_coeff", "Friction coefficients [N-m]");
  params.addRequiredParam<Real>("speed_cr_I", "Turbine speed threshold for inertia [-]");
  params.addRequiredParam<Real>("inertia_const", "Turbine inertia constant [kg-m^2]");
  params.addRequiredParam<std::vector<Real>>("inertia_coeff",
                                             "Turbine inertia coefficients [kg-m^2]");
  params.addRequiredParam<FunctionName>("head_coefficient",
                                        "Function to compute data for turbine head [-]");
  params.addRequiredParam<FunctionName>("power_coefficient",
                                        "Function to compute data for turbine power [-]");
  params.addRequiredParam<std::string>("turbine_name",
                                       "Name of the instance of this turbine component");
  params.addRequiredCoupledVar("omega", "Shaft rotational speed [rad/s]");

  params.addClassDescription("Computes and caches flux and residual vectors for a 1-phase "
                             "turbine. Also computes turbine torque "
                             "and delta_p which is passed to the connected shaft");

  return params;
}

ADShaftConnectedTurbine1PhaseUserObject::ADShaftConnectedTurbine1PhaseUserObject(
    const InputParameters & params)
  : ADVolumeJunction1PhaseUserObject(params),
    ADShaftConnectableUserObjectInterface(this),

    _di_out(getParam<Point>("di_out")),
    _omega_rated(getParam<Real>("omega_rated")),
    _D_wheel(getParam<Real>("D_wheel")),
    _speed_cr_fr(getParam<Real>("speed_cr_fr")),
    _tau_fr_const(getParam<Real>("tau_fr_const")),
    _tau_fr_coeff(getParam<std::vector<Real>>("tau_fr_coeff")),
    _speed_cr_I(getParam<Real>("speed_cr_I")),
    _inertia_const(getParam<Real>("inertia_const")),
    _inertia_coeff(getParam<std::vector<Real>>("inertia_coeff")),
    _head_coefficient(getFunction("head_coefficient")),
    _power_coefficient(getFunction("power_coefficient")),
    _turbine_name(getParam<std::string>("turbine_name")),

    _omega(adCoupledScalarValue("omega"))
{
}

void
ADShaftConnectedTurbine1PhaseUserObject::initialSetup()
{
  ADVolumeJunction1PhaseUserObject::initialSetup();

  ADShaftConnectableUserObjectInterface::setupConnections(
      ADVolumeJunctionBaseUserObject::_n_connections, ADVolumeJunctionBaseUserObject::_n_flux_eq);
}

void
ADShaftConnectedTurbine1PhaseUserObject::initialize()
{
  ADVolumeJunction1PhaseUserObject::initialize();
  ADShaftConnectableUserObjectInterface::initialize();

  _driving_torque = 0;
  _friction_torque = 0;
  _flow_coeff = 0;
  _delta_p = 0;
  _power = 0;
}

void
ADShaftConnectedTurbine1PhaseUserObject::execute()
{
  ADVolumeJunctionBaseUserObject::storeConnectionData();
  ADShaftConnectableUserObjectInterface::setConnectionData(
      ADVolumeJunctionBaseUserObject::_flow_channel_dofs);

  const unsigned int c = getBoundaryIDIndex();

  computeFluxesAndResiduals(c);
}

void
ADShaftConnectedTurbine1PhaseUserObject::computeFluxesAndResiduals(const unsigned int & c)
{
  ADVolumeJunction1PhaseUserObject::computeFluxesAndResiduals(c);

  // inlet c=0 established in component
  if (c == 0)
  {
    const ADReal Q_in = (_rhouA[0] / _rhoA[0]) * _A[0];

    _flow_coeff = Q_in / (_omega[0] * _D_wheel * _D_wheel * _D_wheel);

    const auto head_coeff = _head_coefficient.value(_flow_coeff, ADPoint());

    const ADReal gH = head_coeff * _D_wheel * _D_wheel * _omega[0] * _omega[0];

    const auto power_coeff = _power_coefficient.value(_flow_coeff, ADPoint());

    // friction torque
    ADReal alpha = _omega[0] / _omega_rated;
    Real sign;
    if (_omega[0] >= 0)
      sign = -1;
    else
      sign = 1;
    if (alpha < _speed_cr_fr)
    {
      _friction_torque = sign * _tau_fr_const;
    }
    else
    {
      _friction_torque = sign * (_tau_fr_coeff[0] + _tau_fr_coeff[1] * std::abs(alpha) +
                                 _tau_fr_coeff[2] * alpha * alpha +
                                 _tau_fr_coeff[3] * std::abs(alpha * alpha * alpha));
    }

    _driving_torque =
        power_coeff * (_rhoV[0] / _volume) * _omega[0] * _omega[0] * Utility::pow<5>(_D_wheel);

    _torque += _driving_torque + _friction_torque;

    if (alpha < _speed_cr_I)
    {
      _moment_of_inertia += _inertia_const;
    }
    else
    {
      _moment_of_inertia += _inertia_coeff[0] + _inertia_coeff[1] * std::abs(alpha) +
                            _inertia_coeff[2] * alpha * alpha +
                            _inertia_coeff[3] * std::abs(alpha * alpha * alpha);
    }

    // compute momentum and energy source terms
    // a positive torque value results in a negative S_energy
    _power = _torque * _omega[0];
    const ADReal S_energy = -_power;

    _delta_p = (_rhoV[0] / _volume) * gH;

    const ADRealVectorValue S_momentum = -_delta_p * _A_ref * _di_out;

    _residual[VolumeJunction1Phase::RHOEV_INDEX] -= S_energy;

    _residual[VolumeJunction1Phase::RHOUV_INDEX] -= S_momentum(0);
    _residual[VolumeJunction1Phase::RHOVV_INDEX] -= S_momentum(1);
    _residual[VolumeJunction1Phase::RHOWV_INDEX] -= S_momentum(2);
  }
}

ADReal
ADShaftConnectedTurbine1PhaseUserObject::getDrivingTorque() const
{
  return _driving_torque;
}

ADReal
ADShaftConnectedTurbine1PhaseUserObject::getFrictionTorque() const
{
  return _friction_torque;
}

ADReal
ADShaftConnectedTurbine1PhaseUserObject::getFlowCoefficient() const
{
  return _flow_coeff;
}

ADReal
ADShaftConnectedTurbine1PhaseUserObject::getTurbineDeltaP() const
{
  return _delta_p;
}

ADReal
ADShaftConnectedTurbine1PhaseUserObject::getTurbinePower() const
{
  return _power;
}

void
ADShaftConnectedTurbine1PhaseUserObject::finalize()
{
  ADVolumeJunction1PhaseUserObject::finalize();
  ADShaftConnectableUserObjectInterface::finalize();

  ADShaftConnectableUserObjectInterface::setupJunctionData(
      ADVolumeJunctionBaseUserObject::_scalar_dofs);
  ADShaftConnectableUserObjectInterface::setOmegaDofs(getScalarVar("omega", 0));

  comm().sum(_driving_torque);
  comm().sum(_friction_torque);
  comm().sum(_flow_coeff);
  comm().sum(_delta_p);
  comm().sum(_power);
}

void
ADShaftConnectedTurbine1PhaseUserObject::threadJoin(const UserObject & uo)
{
  ADVolumeJunction1PhaseUserObject::threadJoin(uo);
  ADShaftConnectableUserObjectInterface::threadJoin(uo);

  const ADShaftConnectedTurbine1PhaseUserObject & scpuo =
      dynamic_cast<const ADShaftConnectedTurbine1PhaseUserObject &>(uo);
  _driving_torque += scpuo._driving_torque;
  _friction_torque += scpuo._friction_torque;
  _flow_coeff += scpuo._flow_coeff;
  _delta_p += scpuo._delta_p;
  _power += scpuo._power;
}
