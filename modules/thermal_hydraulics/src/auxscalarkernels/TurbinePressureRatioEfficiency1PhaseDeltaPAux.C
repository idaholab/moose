//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TurbinePressureRatioEfficiency1PhaseDeltaPAux.h"
#include "ADTurbinePressureRatioEfficiency1PhaseUserObject.h"

registerMooseObject("ThermalHydraulicsApp", TurbinePressureRatioEfficiency1PhaseDeltaPAux);

InputParameters
TurbinePressureRatioEfficiency1PhaseDeltaPAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("turbine_uo", "Turbine user object name");
  params.addClassDescription(
      "Change in pressure computed in the 1-phase pressure ratio and efficiency turbine.");
  return params;
}

TurbinePressureRatioEfficiency1PhaseDeltaPAux::TurbinePressureRatioEfficiency1PhaseDeltaPAux(
    const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _turbine_uo(getUserObject<ADTurbinePressureRatioEfficiency1PhaseUserObject>("turbine_uo"))
{
}

Real
TurbinePressureRatioEfficiency1PhaseDeltaPAux::computeValue()
{
  return MetaPhysicL::raw_value(_turbine_uo.getTurbineDeltaP());
}
