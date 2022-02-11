//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADShaftComponentTorqueScalarKernel.h"
#include "UserObject.h"
#include "ADShaftConnectableUserObjectInterface.h"

registerMooseObject("ThermalHydraulicsApp", ADShaftComponentTorqueScalarKernel);

InputParameters
ADShaftComponentTorqueScalarKernel::validParams()
{
  InputParameters params = ADScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("shaft_connected_component_uo",
                                          "Shaft connected component user object name");
  params.addClassDescription("Torque contributed by a component connected to a shaft");
  return params;
}

ADShaftComponentTorqueScalarKernel::ADShaftComponentTorqueScalarKernel(
    const InputParameters & parameters)
  : ADScalarKernel(parameters),
    _shaft_connected_component_uo(
        getUserObject<ADShaftConnectableUserObjectInterface>("shaft_connected_component_uo"))
{
}

ADReal
ADShaftComponentTorqueScalarKernel::computeQpResidual()
{
  return -_shaft_connected_component_uo.getTorque();
}
