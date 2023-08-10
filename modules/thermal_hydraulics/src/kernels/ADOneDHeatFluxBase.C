//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADOneDHeatFluxBase.h"
#include "ADHeatFluxFromHeatStructureBaseUserObject.h"

InputParameters
ADOneDHeatFluxBase::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addRequiredParam<UserObjectName>(
      "q_uo", "The name of the user object that computed the heat flux");
  params.addClassDescription("Base class for a heat flux in the energy equation for 1-phase flow");
  return params;
}

ADOneDHeatFluxBase::ADOneDHeatFluxBase(const InputParameters & parameters)
  : ADKernel(parameters), _q_uo(getUserObject<ADHeatFluxFromHeatStructureBaseUserObject>("q_uo"))
{
}
