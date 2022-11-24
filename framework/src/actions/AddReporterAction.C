//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddReporterAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddReporterAction, "add_reporter");

InputParameters
AddReporterAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Reporter object to the simulation.");
  return params;
}

AddReporterAction::AddReporterAction(const InputParameters & params) : MooseObjectAction(params) {}

void
AddReporterAction::act()
{
  _problem->addReporter(_type, _name, _moose_object_pars);
}
