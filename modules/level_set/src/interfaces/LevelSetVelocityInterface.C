//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "LevelSetVelocityInterface.h"

template <>
InputParameters
validParams<LevelSetVelocityInterface<>>()
{
  InputParameters parameters = emptyInputParameters();
  parameters.addCoupledVar(
      "velocity_x", 0, "The variable containing the x-component of the velocity front.");
  parameters.addCoupledVar(
      "velocity_y", 0, "The variable containing the y-component of the velocity front.");
  parameters.addCoupledVar(
      "velocity_z", 0, "The variable containing the z-component of the velocity front.");
  return parameters;
}
