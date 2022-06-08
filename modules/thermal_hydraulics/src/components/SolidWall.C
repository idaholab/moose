//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidWall.h"

registerMooseObject("ThermalHydraulicsApp", SolidWall);

InputParameters
SolidWall::validParams()
{
  InputParameters params = FlowBoundary::validParams();
  return params;
}

SolidWall::SolidWall(const InputParameters & params) : FlowBoundary(params)
{
  logError("Deprecated component. Use SolidWall1Phase or SolidWall2Phase instead.");
}
