//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShaftConnectedPump1PhaseAux.h"
#include "ADShaftConnectedPump1PhaseUserObject.h"

registerMooseObject("ThermalHydraulicsApp", ShaftConnectedPump1PhaseAux);

InputParameters
ShaftConnectedPump1PhaseAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();

  params.addClassDescription("Computes various quantities for a ShaftConnectedPump1Phase.");

  MooseEnum quantity("pump_head hydraulic_torque friction_torque moment_of_inertia");
  params.addRequiredParam<MooseEnum>("quantity", quantity, "Which quantity to compute");
  params.addRequiredParam<UserObjectName>("pump_uo", "Pump user object name");

  return params;
}

ShaftConnectedPump1PhaseAux::ShaftConnectedPump1PhaseAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _quantity(getParam<MooseEnum>("quantity").getEnum<Quantity>()),
    _pump_uo(getUserObject<ADShaftConnectedPump1PhaseUserObject>("pump_uo"))
{
}

Real
ShaftConnectedPump1PhaseAux::computeValue()
{
  switch (_quantity)
  {
    case Quantity::PUMP_HEAD:
      return MetaPhysicL::raw_value(_pump_uo.getPumpHead());
      break;
    case Quantity::HYDRAULIC_TORQUE:
      return MetaPhysicL::raw_value(_pump_uo.getHydraulicTorque());
      break;
    case Quantity::FRICTION_TORQUE:
      return MetaPhysicL::raw_value(_pump_uo.getFrictionTorque());
      break;
    case Quantity::MOMENT_OF_INERTIA:
      return MetaPhysicL::raw_value(_pump_uo.getMomentOfInertia());
      break;
    default:
      mooseError("Invalid 'quantity' parameter.");
  }
}
