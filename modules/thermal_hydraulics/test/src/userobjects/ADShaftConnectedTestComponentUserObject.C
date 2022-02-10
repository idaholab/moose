//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADShaftConnectedTestComponentUserObject.h"
#include "MooseVariableScalar.h"

registerMooseObject("ThermalHydraulicsTestApp", ADShaftConnectedTestComponentUserObject);

InputParameters
ADShaftConnectedTestComponentUserObject::validParams()
{
  InputParameters params = ADVolumeJunctionBaseUserObject::validParams();
  params += ADShaftConnectableUserObjectInterface::validParams();
  params.addRequiredCoupledVar("rhoA", "rho*A of the connected flow channels");
  params.addRequiredCoupledVar("rhouA", "rhou*A of the connected flow channels");
  params.addRequiredCoupledVar("rhoEA", "rhoE*A of the connected flow channels");
  params.addRequiredCoupledVar("jct_var", "Junction scalar variable");
  params.addRequiredCoupledVar("omega", "Shaft speed scalar variable");
  params.addClassDescription(
      "Test component for showing how to connect a junction-derived object to a shaft");
  return params;
}

ADShaftConnectedTestComponentUserObject::ADShaftConnectedTestComponentUserObject(
    const InputParameters & params)
  : ADVolumeJunctionBaseUserObject(params),
    ADShaftConnectableUserObjectInterface(this),
    _rhoA(adCoupledValue("rhoA")),
    _rhouA(adCoupledValue("rhouA")),
    _rhoEA(adCoupledValue("rhoEA")),

    _jct_var(adCoupledScalarValue("jct_var")),
    _omega(adCoupledScalarValue("omega"))
{
  _flow_variable_names.resize(3);
  _flow_variable_names[0] = "rhoA";
  _flow_variable_names[1] = "rhouA";
  _flow_variable_names[2] = "rhoEA";

  unsigned int n_jct_vars = 1;
  _scalar_variable_names.resize(n_jct_vars);
  _scalar_variable_names[0] = "jct_var";
}

void
ADShaftConnectedTestComponentUserObject::initialize()
{
  ADVolumeJunctionBaseUserObject::initialize();
  ADShaftConnectableUserObjectInterface::initialize();
}

void
ADShaftConnectedTestComponentUserObject::initialSetup()
{
  ADVolumeJunctionBaseUserObject::initialSetup();

  ADShaftConnectableUserObjectInterface::setupConnections(
      ADVolumeJunctionBaseUserObject::_n_connections, ADVolumeJunctionBaseUserObject::_n_flux_eq);
}

void
ADShaftConnectedTestComponentUserObject::computeFluxesAndResiduals(const unsigned int & c)
{
  ADReal tau_c = 0;
  ADReal moi_c = 0;

  if (c == 0)
  {
    tau_c = _jct_var[0] + 2. * _omega[0];
    moi_c = 2. * _jct_var[0] + 3. * _omega[0];

    // residual for the junction variable
    _residual[0] += tau_c + moi_c;

    _torque += tau_c;
    _moment_of_inertia += moi_c;
  }

  tau_c = (c + 1) * _rhoA[0] + (c + 2) * _rhouA[0] + (c + 3) * _rhoEA[0];
  moi_c = (c + 7) * _rhoA[0] + (c + 8) * _rhouA[0] + (c + 9) * _rhoEA[0];

  _residual[0] += tau_c + moi_c;

  _torque += tau_c;
  _moment_of_inertia += moi_c;
}

void
ADShaftConnectedTestComponentUserObject::execute()
{
  ADVolumeJunctionBaseUserObject::storeConnectionData();
  ADShaftConnectableUserObjectInterface::setConnectionData(
      ADVolumeJunctionBaseUserObject::_flow_channel_dofs);

  const unsigned int c = getBoundaryIDIndex();
  computeFluxesAndResiduals(c);
}

void
ADShaftConnectedTestComponentUserObject::finalize()
{
  ADVolumeJunctionBaseUserObject::finalize();

  ADShaftConnectableUserObjectInterface::setupJunctionData(
      ADVolumeJunctionBaseUserObject::_scalar_dofs);
  ADShaftConnectableUserObjectInterface::setOmegaDofs(getScalarVar("omega", 0));
}

void
ADShaftConnectedTestComponentUserObject::threadJoin(const UserObject & uo)
{
  ADVolumeJunctionBaseUserObject::threadJoin(uo);
  ADShaftConnectableUserObjectInterface::threadJoin(uo);
}
