//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddMappingAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "MappingBase.h"

registerMooseAction("StochasticToolsApp", AddMappingAction, "add_mapping");

InputParameters
AddMappingAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Adds Mapping objects.");
  return params;
}

AddMappingAction::AddMappingAction(const InputParameters & params) : MooseObjectAction(params) {}

void
AddMappingAction::act()
{
  _problem->addObject<MappingBase>(_type, _name, _moose_object_pars, /* threaded = */ false);
}
