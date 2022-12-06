//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMPreconditioningIntegrityCheckAction.h"
#include "THMProblem.h"
#include "ThermalHydraulicsApp.h"

registerMooseAction("ThermalHydraulicsApp",
                    THMPreconditioningIntegrityCheckAction,
                    "THM:preconditioning_integrity_check");

InputParameters
THMPreconditioningIntegrityCheckAction::validParams()
{
  InputParameters params = Action::validParams();
  params += THMAppInterface::validParams();

  params.addClassDescription("Triggers the integrity check of preconditioner.");

  return params;
}

THMPreconditioningIntegrityCheckAction::THMPreconditioningIntegrityCheckAction(
    const InputParameters & parameters)
  : Action(parameters), THMAppInterface(parameters)
{
}

void
THMPreconditioningIntegrityCheckAction::act()
{
  auto & thm_app = getTHMApp();
  if (thm_app.getComponents().size() > 0)
    thm_app.getTHMProblem().couplingMatrixIntegrityCheck();
}
