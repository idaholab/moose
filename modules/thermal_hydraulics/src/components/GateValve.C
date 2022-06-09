//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GateValve.h"

registerMooseObject("ThermalHydraulicsApp", GateValve);

InputParameters
GateValve::validParams()
{
  InputParameters params = FlowJunction::validParams();

  params.addRequiredParam<Real>("open_area_fraction", "Fraction of flow area that is open [-]");

  params.addClassDescription("Gate valve component");

  return params;
}

GateValve::GateValve(const InputParameters & params) : FlowJunction(params)
{
  logError("Deprecated component. Use GateValve1Phase or GateValve2Phase instead.");
}
