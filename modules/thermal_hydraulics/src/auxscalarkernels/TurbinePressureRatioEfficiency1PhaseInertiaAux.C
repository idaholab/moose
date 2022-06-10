//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TurbinePressureRatioEfficiency1PhaseInertiaAux.h"
#include "ADTurbinePressureRatioEfficiency1PhaseUserObject.h"

registerMooseObject("ThermalHydraulicsApp", TurbinePressureRatioEfficiency1PhaseInertiaAux);

InputParameters
TurbinePressureRatioEfficiency1PhaseInertiaAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("turbine_uo", "Turbine user object name");
  params.addClassDescription(
      "Moment of inertia computed in the 1-phase pressure ratio efficiency turbine.");
  return params;
}

TurbinePressureRatioEfficiency1PhaseInertiaAux::TurbinePressureRatioEfficiency1PhaseInertiaAux(
    const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _turbine_uo(getUserObject<ADTurbinePressureRatioEfficiency1PhaseUserObject>("turbine_uo"))
{
}

Real
TurbinePressureRatioEfficiency1PhaseInertiaAux::computeValue()
{
  return MetaPhysicL::raw_value(_turbine_uo.getMomentOfInertia());
}
