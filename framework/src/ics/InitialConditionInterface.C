//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InitialConditionInterface.h"
#include "InputParameters.h"
#include "MooseEnum.h"

InputParameters
InitialConditionInterface::validParams()
{
  InputParameters params = emptyInputParameters();

  MooseEnum stateEnum("CURRENT=0 OLD=1 OLDER=2", "CURRENT");
  params.addParam<MooseEnum>(
      "state",
      stateEnum,
      "This parameter is used to set old state solutions at the start of simulation. If specifying "
      "multiple states at the start of simulation, use one IC object for each state being "
      "specified. The states are CURRENT=0 OLD=1 OLDER=2. States older than 2 are not currently "
      "supported. When the user only specifies current state, the solution is copied to the old "
      "and older states, as expected. This functionality is mainly used for dynamic simulations "
      "with explicit time integration schemes, where old solution states are used in the velocity "
      "and acceleration approximations.");

  return params;
}

InitialConditionInterface::InitialConditionInterface(const InputParameters & parameters)
  : _my_state(parameters.get<MooseEnum>("state"))
{
}

InitialConditionInterface::~InitialConditionInterface() {}

unsigned short
InitialConditionInterface::getState() const
{
  return _my_state;
}
