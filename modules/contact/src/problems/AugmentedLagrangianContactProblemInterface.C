//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AugmentedLagrangianContactProblemInterface.h"

InputParameters
AugmentedLagrangianContactProblemInterface::validParams()
{
  auto params = emptyInputParameters();
  params.addParam<int>("maximum_lagrangian_update_iterations",
                       100,
                       "Maximum number of update Lagrangian Multiplier iterations per step");
  return params;
}

AugmentedLagrangianContactProblemInterface::AugmentedLagrangianContactProblemInterface(
    const InputParameters & params)
  : _maximum_number_lagrangian_iterations(params.get<int>("maximum_lagrangian_update_iterations"))
{
}
