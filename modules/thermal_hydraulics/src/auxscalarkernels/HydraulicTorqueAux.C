//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HydraulicTorqueAux.h"
#include "ADShaftConnectedPump1PhaseUserObject.h"

registerMooseObject("ThermalHydraulicsApp", HydraulicTorqueAux);

InputParameters
HydraulicTorqueAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("pump_uo", "Pump user object name");
  params.addClassDescription("Hydraulic torque computed in the 1-phase shaft-connected pump.");
  return params;
}

HydraulicTorqueAux::HydraulicTorqueAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _pump_uo(getUserObject<ADShaftConnectedPump1PhaseUserObject>("pump_uo"))
{
}

Real
HydraulicTorqueAux::computeValue()
{
  return MetaPhysicL::raw_value(_pump_uo.getHydraulicTorque());
}
