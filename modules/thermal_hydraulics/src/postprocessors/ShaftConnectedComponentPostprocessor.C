//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShaftConnectedComponentPostprocessor.h"
#include "ADShaftConnectableUserObjectInterface.h"

registerMooseObject("ThermalHydraulicsApp", ShaftConnectedComponentPostprocessor);

InputParameters
ShaftConnectedComponentPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  MooseEnum quantity("torque inertia");
  params.addRequiredParam<MooseEnum>("quantity", quantity, "Quantity to get");
  params.addRequiredParam<UserObjectName>("shaft_connected_component_uo",
                                          "Shaft-connected component user object");
  params.addClassDescription("Gets torque or moment of inertia for a shaft-connected component.");
  return params;
}

ShaftConnectedComponentPostprocessor::ShaftConnectedComponentPostprocessor(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),

    _quantity(getParam<MooseEnum>("quantity").getEnum<Quantity>()),
    _component_uo(
        getUserObject<ADShaftConnectableUserObjectInterface>("shaft_connected_component_uo"))
{
}

void
ShaftConnectedComponentPostprocessor::initialize()
{
}

void
ShaftConnectedComponentPostprocessor::execute()
{
}

Real
ShaftConnectedComponentPostprocessor::getValue() const
{
  switch (_quantity)
  {
    case Quantity::TORQUE:
      return MetaPhysicL::raw_value(_component_uo.getTorque());
      break;
    case Quantity::INERTIA:
      return MetaPhysicL::raw_value(_component_uo.getMomentOfInertia());
      break;
    default:
      mooseError("Invalid 'quantity' parameter.");
  }
}
