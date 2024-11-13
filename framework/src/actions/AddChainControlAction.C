//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddChainControlAction.h"

registerMooseAction("MooseApp", AddChainControlAction, "add_chain_control");

InputParameters
AddChainControlAction::validParams()
{
  InputParameters params = AddControlAction::validParams();
  params.addClassDescription("Adds a ChainControl to the control warehouse.");
  return params;
}

AddChainControlAction::AddChainControlAction(const InputParameters & parameters)
  : AddControlAction(parameters)
{
}
