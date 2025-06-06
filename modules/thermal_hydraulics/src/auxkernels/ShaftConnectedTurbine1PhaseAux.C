//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShaftConnectedTurbine1PhaseAux.h"
#include "ADShaftConnectedTurbine1PhaseUserObject.h"

registerMooseObject("ThermalHydraulicsApp", ShaftConnectedTurbine1PhaseAux);

InputParameters
ShaftConnectedTurbine1PhaseAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addClassDescription("Computes various quantities for a ShaftConnectedTurbine1Phase.");

  MooseEnum quantity(
      "delta_p flow_coefficient driving_torque friction_torque moment_of_inertia power");
  params.addRequiredParam<MooseEnum>("quantity", quantity, "Which quantity to compute");
  params.addRequiredParam<UserObjectName>("turbine_uo", "Turbine user object name");

  return params;
}

ShaftConnectedTurbine1PhaseAux::ShaftConnectedTurbine1PhaseAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _quantity(this->template getParam<MooseEnum>("quantity").template getEnum<Quantity>()),
    _turbine_uo(this->template getUserObject<ADShaftConnectedTurbine1PhaseUserObject>("turbine_uo"))
{
}

Real
ShaftConnectedTurbine1PhaseAux::computeValue()
{
  switch (_quantity)
  {
    case Quantity::DELTA_P:
      return MetaPhysicL::raw_value(_turbine_uo.getTurbineDeltaP());
      break;
    case Quantity::FLOW_COEFFICIENT:
      return MetaPhysicL::raw_value(_turbine_uo.getFlowCoefficient());
      break;
    case Quantity::DRIVING_TORQUE:
      return MetaPhysicL::raw_value(_turbine_uo.getDrivingTorque());
      break;
    case Quantity::FRICTION_TORQUE:
      return MetaPhysicL::raw_value(_turbine_uo.getFrictionTorque());
      break;
    case Quantity::MOMENT_OF_INERTIA:
      return MetaPhysicL::raw_value(_turbine_uo.getMomentOfInertia());
      break;
    case Quantity::POWER:
      return MetaPhysicL::raw_value(_turbine_uo.getTurbinePower());
      break;
    default:
      mooseError("Invalid 'quantity' parameter.");
  }
}
