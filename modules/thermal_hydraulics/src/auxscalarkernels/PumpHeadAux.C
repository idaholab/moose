//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PumpHeadAux.h"
#include "ADShaftConnectedPump1PhaseUserObject.h"

registerMooseObject("ThermalHydraulicsApp", PumpHeadAux);

InputParameters
PumpHeadAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("pump_uo", "Pump user object name");
  params.addClassDescription("Head computed in the 1-phase shaft-connected pump.");
  return params;
}

PumpHeadAux::PumpHeadAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _pump_uo(getUserObject<ADShaftConnectedPump1PhaseUserObject>("pump_uo"))
{
}

Real
PumpHeadAux::computeValue()
{
  return MetaPhysicL::raw_value(_pump_uo.getPumpHead());
}
