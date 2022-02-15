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
ShaftConnectedComponentPostprocessor::getValue()
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
