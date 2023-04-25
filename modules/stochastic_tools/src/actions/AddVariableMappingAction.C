//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddVariableMappingAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "VariableMappingBase.h"

registerMooseAction("StochasticToolsApp", AddVariableMappingAction, "add_variable_mapping");

InputParameters
AddVariableMappingAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Adds Mapping objects from a VariableMappings block.");
  return params;
}

AddVariableMappingAction::AddVariableMappingAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddVariableMappingAction::act()
{
  _problem->addObject<VariableMappingBase>(
      _type, _name, _moose_object_pars, /* threaded = */ false);
}
