//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JunctionOneToOne.h"

registerMooseObject("ThermalHydraulicsApp", JunctionOneToOne);

InputParameters
JunctionOneToOne::validParams()
{
  InputParameters params = FlowJunction::validParams();

  params.addClassDescription("Junction connecting one flow channel to one other flow channel");

  return params;
}

JunctionOneToOne::JunctionOneToOne(const InputParameters & params) : FlowJunction(params)
{
  logError("Deprecated component. Use JunctionOneToOne1Phase or JunctionOneToOne2Phase instead.");
}
