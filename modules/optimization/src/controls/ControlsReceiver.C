//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ControlsReceiver.h"

registerMooseObject("isopodApp", ControlsReceiver);

InputParameters
ControlsReceiver::validParams()
{
  InputParameters params = SamplerReceiver::validParams();
  params.addClassDescription("Control for receiving data from an Optimization Transfer.");
  return params;
}

ControlsReceiver::ControlsReceiver(const InputParameters & parameters) : SamplerReceiver(parameters)
{
}
