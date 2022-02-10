//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADVolumeJunction1PhaseBC.h"
#include "ADVolumeJunction1PhaseUserObject.h"
#include "VolumeJunction1Phase.h"
#include "THMIndices3Eqn.h"

registerMooseObject("ThermalHydraulicsApp", ADVolumeJunction1PhaseBC);

InputParameters
ADVolumeJunction1PhaseBC::validParams()
{
  InputParameters params = ADOneDIntegratedBC::validParams();

  params.addRequiredParam<unsigned int>("connection_index", "Index of the connected flow channel");
  params.addRequiredParam<UserObjectName>("volume_junction_uo", "Volume junction user object name");

  params.addRequiredCoupledVar("A_elem", "Cross-sectional area, elemental");
  params.addRequiredCoupledVar("A_linear", "Cross-sectional area, linear");

  params.addRequiredCoupledVar("rhoA", "Flow channel variable: rho*A");
  params.addRequiredCoupledVar("rhouA", "Flow channel variable: rho*u*A");
  params.addRequiredCoupledVar("rhoEA", "Flow channel variable: rho*E*A");

  params.addRequiredCoupledVar("rhoV", "Junction variable: rho*V");
  params.addRequiredCoupledVar("rhouV", "Junction variable: rho*u*V");
  params.addRequiredCoupledVar("rhovV", "Junction variable: rho*v*V");
  params.addRequiredCoupledVar("rhowV", "Junction variable: rho*w*V");
  params.addRequiredCoupledVar("rhoEV", "Junction variable: rho*E*V");

  params.addClassDescription(
      "Adds boundary fluxes for flow channels connected to a 1-phase volume junction");

  return params;
}

ADVolumeJunction1PhaseBC::ADVolumeJunction1PhaseBC(const InputParameters & params)
  : ADOneDIntegratedBC(params),

    _connection_index(getParam<unsigned int>("connection_index")),
    _volume_junction_uo(getUserObject<ADVolumeJunction1PhaseUserObject>("volume_junction_uo")),

    _A_elem(adCoupledValue("A_elem")),
    _A_linear(adCoupledValue("A_linear")),

    _rhoA_jvar(coupled("rhoA")),
    _rhouA_jvar(coupled("rhouA")),
    _rhoEA_jvar(coupled("rhoEA")),

    _rhoV_jvar(coupledScalar("rhoV")),
    _rhouV_jvar(coupledScalar("rhouV")),
    _rhovV_jvar(coupledScalar("rhovV")),
    _rhowV_jvar(coupledScalar("rhowV")),
    _rhoEV_jvar(coupledScalar("rhoEV")),

    _flow_channel_jvar_map(getFlowChannelIndexMapping()),
    _junction_jvar_map(getJunctionIndexMapping()),
    _equation_index(_flow_channel_jvar_map.at(_var.number()))
{
}

ADReal
ADVolumeJunction1PhaseBC::computeQpResidual()
{
  const auto & flux = _volume_junction_uo.getFlux(_connection_index);

  // Note that the ratio A_linear / A_elem is necessary because A_elem is passed
  // to the flux function, but A_linear is to be used on the boundary.
  return flux[_equation_index] * _A_linear[_qp] / _A_elem[_qp] * _normal * _test[_i][_qp];
}

std::map<unsigned int, unsigned int>
ADVolumeJunction1PhaseBC::getFlowChannelIndexMapping() const
{
  std::map<unsigned int, unsigned int> jvar_map;
  jvar_map.insert(std::pair<unsigned int, unsigned int>(_rhoA_jvar, THM3Eqn::EQ_MASS));
  jvar_map.insert(std::pair<unsigned int, unsigned int>(_rhouA_jvar, THM3Eqn::EQ_MOMENTUM));
  jvar_map.insert(std::pair<unsigned int, unsigned int>(_rhoEA_jvar, THM3Eqn::EQ_ENERGY));

  return jvar_map;
}

std::map<unsigned int, unsigned int>
ADVolumeJunction1PhaseBC::getJunctionIndexMapping() const
{
  std::map<unsigned int, unsigned int> jvar_map;
  jvar_map.insert(
      std::pair<unsigned int, unsigned int>(_rhoV_jvar, VolumeJunction1Phase::RHOV_INDEX));
  jvar_map.insert(
      std::pair<unsigned int, unsigned int>(_rhouV_jvar, VolumeJunction1Phase::RHOUV_INDEX));
  jvar_map.insert(
      std::pair<unsigned int, unsigned int>(_rhovV_jvar, VolumeJunction1Phase::RHOVV_INDEX));
  jvar_map.insert(
      std::pair<unsigned int, unsigned int>(_rhowV_jvar, VolumeJunction1Phase::RHOWV_INDEX));
  jvar_map.insert(
      std::pair<unsigned int, unsigned int>(_rhoEV_jvar, VolumeJunction1Phase::RHOEV_INDEX));

  return jvar_map;
}
