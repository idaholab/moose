//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PumpHydraulicTorqueAux.h"
#include "ADShaftConnectedPump1PhaseUserObject.h"

registerMooseObject("ThermalHydraulicsApp", PumpHydraulicTorqueAux);
registerMooseObjectRenamed("ThermalHydraulicsApp",
                           HydraulicTorqueAux,
                           "02/01/2024 00:00",
                           PumpHydraulicTorqueAux);

InputParameters
PumpHydraulicTorqueAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("pump_uo", "Pump user object name");
  params.addClassDescription(
      "Returns the hydraulic torque computed in the 1-phase shaft-connected pump.");
  return params;
}

PumpHydraulicTorqueAux::PumpHydraulicTorqueAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _pump_uo(getUserObject<ADShaftConnectedPump1PhaseUserObject>("pump_uo"))
{
}

Real
PumpHydraulicTorqueAux::computeValue()
{
  return MetaPhysicL::raw_value(_pump_uo.getHydraulicTorque());
}
