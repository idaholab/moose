//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "AddControlAction.h"
#include "FEProblem.h"
#include "Factory.h"
#include "Control.h"

registerMooseAction("MooseApp", AddControlAction, "add_control");

InputParameters
AddControlAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Control object to the simulation.");
  return params;
}

AddControlAction::AddControlAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddControlAction::act()
{
  _moose_object_pars.addPrivateParam<FEProblemBase *>("_fe_problem_base", _problem.get());
  std::shared_ptr<Control> control = _factory.create<Control>(_type, _name, _moose_object_pars);
  _problem->getControlWarehouse().addObject(control);
}
