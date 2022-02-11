//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Turbine1PhaseFlowCoefficientAux.h"
#include "ADShaftConnectedTurbine1PhaseUserObject.h"

registerMooseObject("ThermalHydraulicsApp", Turbine1PhaseFlowCoefficientAux);

InputParameters
Turbine1PhaseFlowCoefficientAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("turbine_uo", "Turbine user object name");
  params.addClassDescription("Flow coefficient computed in the 1-phase shaft-connected turbine.");
  return params;
}

Turbine1PhaseFlowCoefficientAux::Turbine1PhaseFlowCoefficientAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _turbine_uo(getUserObject<ADShaftConnectedTurbine1PhaseUserObject>("turbine_uo"))
{
}

Real
Turbine1PhaseFlowCoefficientAux::computeValue()
{
  return MetaPhysicL::raw_value(_turbine_uo.getFlowCoefficient());
}
