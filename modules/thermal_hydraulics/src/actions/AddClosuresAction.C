//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddClosuresAction.h"
#include "ClosuresRegistry.h"
#include "ClosuresBase.h"

registerMooseAction("ThermalHydraulicsApp", AddClosuresAction, "THM:add_closures");

InputParameters
AddClosuresAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Adds a Closures object.");
  return params;
}

AddClosuresAction::AddClosuresAction(const InputParameters & params) : MooseObjectAction(params) {}

void
AddClosuresAction::act()
{
  ClosuresRegistry & registry = ClosuresRegistry::findOrCreate(_awh, _action_factory);

  _moose_object_pars.set<FEProblemBase *>("_problem") = _problem.get();
  _moose_object_pars.set<Logger *>("_logger") = &registry.getLogger();

  registry.addClosures(_type, _name, _moose_object_pars);
}
