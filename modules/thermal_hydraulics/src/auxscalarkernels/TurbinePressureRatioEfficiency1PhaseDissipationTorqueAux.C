//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TurbinePressureRatioEfficiency1PhaseDissipationTorqueAux.h"
#include "ADTurbinePressureRatioEfficiency1PhaseUserObject.h"

registerMooseObject("ThermalHydraulicsApp",
                    TurbinePressureRatioEfficiency1PhaseDissipationTorqueAux);

InputParameters
TurbinePressureRatioEfficiency1PhaseDissipationTorqueAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("turbine_uo", "Turbine user object name");
  params.addClassDescription(
      "Dissipation torque computed in the 1-phase pressure ratio efficiency turbine.");
  return params;
}

TurbinePressureRatioEfficiency1PhaseDissipationTorqueAux::
    TurbinePressureRatioEfficiency1PhaseDissipationTorqueAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _turbine_uo(getUserObject<ADTurbinePressureRatioEfficiency1PhaseUserObject>("turbine_uo"))
{
}

Real
TurbinePressureRatioEfficiency1PhaseDissipationTorqueAux::computeValue()
{
  return MetaPhysicL::raw_value(_turbine_uo.getDissipationTorque());
}
