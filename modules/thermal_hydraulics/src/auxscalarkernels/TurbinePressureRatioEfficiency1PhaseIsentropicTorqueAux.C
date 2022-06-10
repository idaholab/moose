//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TurbinePressureRatioEfficiency1PhaseIsentropicTorqueAux.h"
#include "ADTurbinePressureRatioEfficiency1PhaseUserObject.h"

registerMooseObject("ThermalHydraulicsApp",
                    TurbinePressureRatioEfficiency1PhaseIsentropicTorqueAux);

InputParameters
TurbinePressureRatioEfficiency1PhaseIsentropicTorqueAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("turbine_uo", "Compressor user object name");
  params.addClassDescription(
      "Isentropic torque computed in the 1-phase shaft-connected compressor.");
  return params;
}

TurbinePressureRatioEfficiency1PhaseIsentropicTorqueAux::
    TurbinePressureRatioEfficiency1PhaseIsentropicTorqueAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _turbine_uo(getUserObject<ADTurbinePressureRatioEfficiency1PhaseUserObject>("turbine_uo"))
{
}

Real
TurbinePressureRatioEfficiency1PhaseIsentropicTorqueAux::computeValue()
{
  return MetaPhysicL::raw_value(_turbine_uo.getIsentropicTorque());
}
