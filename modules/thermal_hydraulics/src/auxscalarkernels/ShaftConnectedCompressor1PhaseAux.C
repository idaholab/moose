//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShaftConnectedCompressor1PhaseAux.h"
#include "ADShaftConnectedCompressor1PhaseUserObject.h"

registerMooseObject("ThermalHydraulicsApp", ShaftConnectedCompressor1PhaseAux);

InputParameters
ShaftConnectedCompressor1PhaseAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();

  params.addClassDescription("Computes various quantities for a ShaftConnectedCompressor1Phase.");

  MooseEnum quantity(
      "delta_p isentropic_torque dissipation_torque friction_torque moment_of_inertia");
  params.addRequiredParam<MooseEnum>("quantity", quantity, "Which quantity to compute");
  params.addRequiredParam<UserObjectName>("compressor_uo", "Compressor user object name");

  return params;
}

ShaftConnectedCompressor1PhaseAux::ShaftConnectedCompressor1PhaseAux(
    const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _quantity(getParam<MooseEnum>("quantity").getEnum<Quantity>()),
    _compressor_uo(getUserObject<ADShaftConnectedCompressor1PhaseUserObject>("compressor_uo"))
{
}

Real
ShaftConnectedCompressor1PhaseAux::computeValue()
{
  switch (_quantity)
  {
    case Quantity::DELTA_P:
      return MetaPhysicL::raw_value(_compressor_uo.getCompressorDeltaP());
      break;
    case Quantity::ISENTROPIC_TORQUE:
      return MetaPhysicL::raw_value(_compressor_uo.getIsentropicTorque());
      break;
    case Quantity::DISSIPATION_TORQUE:
      return MetaPhysicL::raw_value(_compressor_uo.getDissipationTorque());
      break;
    case Quantity::FRICTION_TORQUE:
      return MetaPhysicL::raw_value(_compressor_uo.getFrictionTorque());
      break;
    case Quantity::MOMENT_OF_INERTIA:
      return MetaPhysicL::raw_value(_compressor_uo.getMomentOfInertia());
      break;
    default:
      mooseError("Invalid 'quantity' parameter.");
  }
}
