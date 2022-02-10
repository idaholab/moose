//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RiemannArgumentSwitchingInterface.h"
#include "MooseObject.h"

InputParameters
RiemannArgumentSwitchingInterface::validParams()
{
  InputParameters params = emptyInputParameters();

  params.addRequiredParam<bool>("switch_left_and_right", "Switch the left and right arguments?");

  return params;
}

RiemannArgumentSwitchingInterface::RiemannArgumentSwitchingInterface(
    const MooseObject * moose_object)
  : _switch_left_and_right(moose_object->getParam<bool>("switch_left_and_right"))
{
}
