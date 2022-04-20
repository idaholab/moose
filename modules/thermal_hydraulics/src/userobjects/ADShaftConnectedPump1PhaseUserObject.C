//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADShaftConnectedPump1PhaseUserObject.h"
#include "VolumeJunction1Phase.h"
#include "MooseVariableScalar.h"
#include "THMIndices3Eqn.h"
#include "Function.h"
#include "metaphysicl/parallel_numberarray.h"
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"
#include "libmesh/parallel_algebra.h"

registerMooseObject("ThermalHydraulicsApp", ADShaftConnectedPump1PhaseUserObject);

InputParameters
ADShaftConnectedPump1PhaseUserObject::validParams()
{
  InputParameters params = ADVolumeJunction1PhaseUserObject::validParams();
  params += ADShaftConnectableUserObjectInterface::validParams();

  params.addParam<BoundaryName>("inlet", "Pump inlet");
  params.addParam<BoundaryName>("outlet", "Pump outlet");
  params.addRequiredParam<Point>("di_out", "Direction of connected outlet");
  params.addRequiredParam<Real>("gravity_magnitude", "Gravity constant, [m/s^2]");
  params.addRequiredParam<Real>("omega_rated", "Rated pump speed [rad/s]");
  params.addRequiredParam<Real>("volumetric_rated", "Rated pump volumetric flow rate [m^3/s]");
  params.addRequiredParam<Real>("head_rated", "Rated pump head [m]");
  params.addRequiredParam<Real>("torque_rated", "Rated pump torque [N-m]");
  params.addRequiredParam<Real>("density_rated", "Rated pump fluid density [kg/m^3]");
  params.addRequiredParam<Real>("speed_cr_fr", "Pump speed threshold for friction [-]");
  params.addRequiredParam<Real>("tau_fr_const", "Pump friction constant [N-m]");
  params.addRequiredParam<std::vector<Real>>("tau_fr_coeff", "Friction coefficients [N-m]");
  params.addRequiredParam<Real>("speed_cr_I", "Pump speed threshold for inertia [-]");
  params.addRequiredParam<Real>("inertia_const", "Pump inertia constant [kg-m^2]");
  params.addRequiredParam<std::vector<Real>>("inertia_coeff", "Pump inertia coefficients [kg-m^2]");
  params.addRequiredParam<FunctionName>("head", "Function to compute data for pump head [-]");
  params.addRequiredParam<FunctionName>("torque_hydraulic",
                                        "Function to compute data for pump torque [-]");
  params.addRequiredParam<std::string>("pump_name", "Name of the instance of this pump component");
  params.addParam<Real>(
      "transition_width",
      1e-3,
      "Transition width for sign of the frictional torque at 0 speed over rated speed ratio.");
  params.addRequiredCoupledVar("omega", "Shaft rotational speed [rad/s]");

  params.addClassDescription(
      "Computes and caches flux and residual vectors for a 1-phase pump. Also computes pump torque "
      "and head which is passed to the connected shaft");

  return params;
}

ADShaftConnectedPump1PhaseUserObject::ADShaftConnectedPump1PhaseUserObject(
    const InputParameters & params)
  : ADVolumeJunction1PhaseUserObject(params),
    ADShaftConnectableUserObjectInterface(this),

    _di_out(getParam<Point>("di_out")),
    _g(getParam<Real>("gravity_magnitude")),
    _omega_rated(getParam<Real>("omega_rated")),
    _volumetric_rated(getParam<Real>("volumetric_rated")),
    _head_rated(getParam<Real>("head_rated")),
    _torque_rated(getParam<Real>("torque_rated")),
    _density_rated(getParam<Real>("density_rated")),
    _speed_cr_fr(getParam<Real>("speed_cr_fr")),
    _tau_fr_const(getParam<Real>("tau_fr_const")),
    _tau_fr_coeff(getParam<std::vector<Real>>("tau_fr_coeff")),
    _speed_cr_I(getParam<Real>("speed_cr_I")),
    _inertia_const(getParam<Real>("inertia_const")),
    _inertia_coeff(getParam<std::vector<Real>>("inertia_coeff")),
    _head(getFunction("head")),
    _torque_hydraulic(getFunction("torque_hydraulic")),
    _pump_name(getParam<std::string>("pump_name")),
    _omega(adCoupledScalarValue("omega")),
    _transition_width(getParam<Real>("transition_width")),
    _transition_friction(0, _transition_width)
{
}

void
ADShaftConnectedPump1PhaseUserObject::initialSetup()
{
  ADVolumeJunction1PhaseUserObject::initialSetup();

  ADShaftConnectableUserObjectInterface::setupConnections(
      ADVolumeJunctionBaseUserObject::_n_connections, ADVolumeJunctionBaseUserObject::_n_flux_eq);
}

void
ADShaftConnectedPump1PhaseUserObject::initialize()
{
  ADVolumeJunction1PhaseUserObject::initialize();
  ADShaftConnectableUserObjectInterface::initialize();

  _hydraulic_torque = 0;
  _friction_torque = 0;
  _pump_head = 0;
}

void
ADShaftConnectedPump1PhaseUserObject::execute()
{
  ADVolumeJunctionBaseUserObject::storeConnectionData();
  ADShaftConnectableUserObjectInterface::setConnectionData(
      ADVolumeJunctionBaseUserObject::_flow_channel_dofs);

  const unsigned int c = getBoundaryIDIndex();

  computeFluxesAndResiduals(c);
}

void
ADShaftConnectedPump1PhaseUserObject::computeFluxesAndResiduals(const unsigned int & c)
{
  ADVolumeJunction1PhaseUserObject::computeFluxesAndResiduals(c);

  // inlet c=0 established in component
  if (c == 0)
  {
    ADReal alpha = _omega[0] / _omega_rated;

    ADReal Q_in = (_rhouA[0] / _rhoA[0]) * _A[0];

    ADReal nu = Q_in / _volumetric_rated;

    // Head and torque
    ADReal x_p = std::atan2(alpha, nu);
    const auto wt = _torque_hydraulic.value(x_p, ADPoint());
    const auto wh = _head.value(x_p, ADPoint());

    const ADReal y = alpha * alpha + nu * nu;

    const auto zt = wt * _torque_rated;

    // Real homologous_torque = -(alpha * alpha + nu * nu) * wt * _torque_rated;
    const ADReal homologous_torque = -y * zt;
    _hydraulic_torque = homologous_torque * ((_rhoA[0] / _A[0]) / _density_rated);

    const auto zh = wh * _head_rated;

    // _pump_head = (alpha * alpha + nu * nu) * wh * _head_rated;
    _pump_head = y * zh;

    // MoI
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

    // friction torque
    ADReal sign;
    if (alpha > _transition_friction.rightEnd())
      sign = -1;
    else if (alpha < _transition_friction.leftEnd())
      sign = 1;
    else
      sign = _transition_friction.value(alpha, 1, -1);

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

    // compute momentum and energy source terms
    // a negative torque value results in a positive S_energy
    const ADReal S_energy = -_hydraulic_torque * _omega[0];

    // a positive head value results in a positive S_momentum
    const ADRealVectorValue S_momentum = (_rhoA[0] / _A[0]) * _g * _pump_head * _A_ref * _di_out;
    //

    _residual[VolumeJunction1Phase::RHOEV_INDEX] -= S_energy;

    _residual[VolumeJunction1Phase::RHOUV_INDEX] -= S_momentum(0);
    _residual[VolumeJunction1Phase::RHOVV_INDEX] -= S_momentum(1);
    _residual[VolumeJunction1Phase::RHOWV_INDEX] -= S_momentum(2);

    _torque += _hydraulic_torque + _friction_torque;
  }
}

ADReal
ADShaftConnectedPump1PhaseUserObject::getHydraulicTorque() const
{
  return _hydraulic_torque;
}

ADReal
ADShaftConnectedPump1PhaseUserObject::getFrictionTorque() const
{
  return _friction_torque;
}

ADReal
ADShaftConnectedPump1PhaseUserObject::getPumpHead() const
{
  return _pump_head;
}

void
ADShaftConnectedPump1PhaseUserObject::finalize()
{
  ADVolumeJunction1PhaseUserObject::finalize();
  ADShaftConnectableUserObjectInterface::finalize();

  ADShaftConnectableUserObjectInterface::setupJunctionData(
      ADVolumeJunctionBaseUserObject::_scalar_dofs);
  ADShaftConnectableUserObjectInterface::setOmegaDofs(getScalarVar("omega", 0));

  comm().sum(_hydraulic_torque);
  comm().sum(_friction_torque);
  comm().sum(_pump_head);
}

void
ADShaftConnectedPump1PhaseUserObject::threadJoin(const UserObject & uo)
{
  ADVolumeJunction1PhaseUserObject::threadJoin(uo);
  ADShaftConnectableUserObjectInterface::threadJoin(uo);

  const ADShaftConnectedPump1PhaseUserObject & scpuo =
      dynamic_cast<const ADShaftConnectedPump1PhaseUserObject &>(uo);
  _hydraulic_torque += scpuo._hydraulic_torque;
  _friction_torque += scpuo._friction_torque;
  _pump_head += scpuo._pump_head;
}
