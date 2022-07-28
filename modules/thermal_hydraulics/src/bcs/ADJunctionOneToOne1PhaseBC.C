//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADJunctionOneToOne1PhaseBC.h"
#include "ADJunctionOneToOne1PhaseUserObject.h"
#include "THMIndices3Eqn.h"

registerMooseObject("ThermalHydraulicsApp", ADJunctionOneToOne1PhaseBC);

InputParameters
ADJunctionOneToOne1PhaseBC::validParams()
{
  InputParameters params = ADOneDIntegratedBC::validParams();

  params.addRequiredParam<unsigned int>("connection_index", "Index of the connected flow channel");
  params.addRequiredParam<UserObjectName>("junction_uo", "1-phase one-to-one junction user object");

  params.addRequiredCoupledVar("rhoA", "Flow channel variable: rho*A");
  params.addRequiredCoupledVar("rhouA", "Flow channel variable: rho*u*A");
  params.addRequiredCoupledVar("rhoEA", "Flow channel variable: rho*E*A");

  params.addClassDescription(
      "Adds boundary fluxes for flow channels connected to a 1-phase one-to-one junction");

  return params;
}

ADJunctionOneToOne1PhaseBC::ADJunctionOneToOne1PhaseBC(const InputParameters & params)
  : ADOneDIntegratedBC(params),

    _connection_index(getParam<unsigned int>("connection_index")),
    _junction_uo(getUserObject<ADJunctionOneToOne1PhaseUserObject>("junction_uo")),

    _rhoA_jvar(coupled("rhoA")),
    _rhouA_jvar(coupled("rhouA")),
    _rhoEA_jvar(coupled("rhoEA")),

    _jvar_map(getIndexMapping()),
    _equation_index(_jvar_map.at(_var.number()))
{
}

ADReal
ADJunctionOneToOne1PhaseBC::computeQpResidual()
{
  const auto & flux = _junction_uo.getFlux(_connection_index);

  return flux[_equation_index] * _normal * _test[_i][_qp];
}

std::map<unsigned int, unsigned int>
ADJunctionOneToOne1PhaseBC::getIndexMapping() const
{
  std::map<unsigned int, unsigned int> jvar_map;
  jvar_map.insert(std::pair<unsigned int, unsigned int>(_rhoA_jvar, THM3Eqn::EQ_MASS));
  jvar_map.insert(std::pair<unsigned int, unsigned int>(_rhouA_jvar, THM3Eqn::EQ_MOMENTUM));
  jvar_map.insert(std::pair<unsigned int, unsigned int>(_rhoEA_jvar, THM3Eqn::EQ_ENERGY));

  return jvar_map;
}
