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
#include "THMIndicesVACE.h"

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

    _flow_channel_jvar_map(getFlowChannelIndexMapping()),
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
  jvar_map.insert(std::pair<unsigned int, unsigned int>(_rhoA_jvar, THMVACE1D::MASS));
  jvar_map.insert(std::pair<unsigned int, unsigned int>(_rhouA_jvar, THMVACE1D::MOMENTUM));
  jvar_map.insert(std::pair<unsigned int, unsigned int>(_rhoEA_jvar, THMVACE1D::ENERGY));

  return jvar_map;
}
