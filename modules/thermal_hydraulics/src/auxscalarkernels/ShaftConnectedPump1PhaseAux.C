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
registerMooseObject("ThermalHydraulicsApp", ShaftConnectedPump1PhaseScalarAux);

template <typename T>
InputParameters
ShaftConnectedPump1PhaseAuxTempl<T>::validParams()
{
  InputParameters params = T::validParams();

  params.addClassDescription("Computes various quantities for a ShaftConnectedPump1Phase.");

  MooseEnum quantity("pump_head hydraulic_torque friction_torque moment_of_inertia");
  params.addRequiredParam<MooseEnum>("quantity", quantity, "Which quantity to compute");
  params.addRequiredParam<UserObjectName>("pump_uo", "Pump user object name");

  return params;
}

template <typename T>
ShaftConnectedPump1PhaseAuxTempl<T>::ShaftConnectedPump1PhaseAuxTempl(
    const InputParameters & parameters)
  : T(parameters),
    _quantity(this->template getParam<MooseEnum>("quantity").template getEnum<Quantity>()),
    _pump_uo(this->template getUserObject<ADShaftConnectedPump1PhaseUserObject>("pump_uo"))
{
}

template <typename T>
Real
ShaftConnectedPump1PhaseAuxTempl<T>::computeValue()
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

template class ShaftConnectedPump1PhaseAuxTempl<AuxKernel>;
template class ShaftConnectedPump1PhaseAuxTempl<AuxScalarKernel>;
