//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddClosuresAction.h"
#include "THMProblem.h"
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
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
  {
    _moose_object_pars.set<THMProblem *>("_thm_problem") = thm_problem;
    _moose_object_pars.set<Logger *>("_logger") = &(thm_problem->log());

    thm_problem->addClosures(_type, _name, _moose_object_pars);
  }
}
